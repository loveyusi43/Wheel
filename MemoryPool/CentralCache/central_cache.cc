#include "central_cache.h"
#include "page_cache.h"

CentralCache CentralCache::instance_;

Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// 先查看当前SpanList(list)有没有空闲的Span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->free_list_ != nullptr)
		{
			return it;
		}
		it = it->next_;
	}

	list.mtx_.unlock();

	// 说明没有空闲的Span,只能找下一级(PageCache)
	PageCache::GetInstance()->page_mtx_.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	span->is_use_ = true;
	span->object_size_ = size;
	PageCache::GetInstance()->page_mtx_.unlock();
	// 计算大块内存的起始地址和字节数大小
	char* start = (char*)(span->page_id_ << PAGE_SHIFT);
	size_t bytes = span->n_ << PAGE_SHIFT;
	char* end = start + bytes;

	// 把大块内存切成自由链表
	span->free_list_ = start;
	start += size;
	void* tail = span->free_list_;

	while (start < end)
	{
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;
	}
	NextObj(tail) = nullptr;

	list.mtx_.lock();
	list.PushFront(span);

	return span;
}

void CentralCache::ReleaseListToSpans(void* start, size_t byte_size)
{
	size_t index = SizeClass::Index(byte_size);
	span_list_[index].mtx_.lock();

	while (start)
	{
		void* next = NextObj(start);

		// Central Cache 下的每个桶是以Span为基本数据类型的带头双向循环链表，要找到一个内存块对的Span常规做法是遍历该链表，但回导致时间复杂度大大增加，为此在PageCache 中设计了MapObjectToSpan,可以不用遍历链表而快速的找到对应的Span
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		NextObj(start) = span->free_list_;
		span->free_list_ = start;
		span->use_count_--;

		// 说明该Span的所有内存都已归还
		if (span->use_count_ == 0)
		{
			span_list_[index].Erase(span); // Erase 的作用并不是真的删除，而是把 span 从链表中解下来(同时也会重新连接链表)
			span->free_list_ = nullptr;
			span->next_ = nullptr;
			span->prev_ = nullptr;
			// span 的页号和页数不能动

			span_list_[index].mtx_.unlock();
			PageCache::GetInstance()->page_mtx_.lock();
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
			PageCache::GetInstance()->page_mtx_.unlock();
			span_list_[index].mtx_.lock();
		}
		start = next;
	}

	span_list_[index].mtx_.unlock();
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batch_num, size_t size)
{
	size_t index = SizeClass::Index(size); // 计算对应的桶号
	span_list_[index].mtx_.lock();

	Span* span = GetOneSpan(span_list_[index], size);
	assert(span);
	assert(span->free_list_);

	start = span->free_list_;
	end = start;
	size_t i = 0;
	size_t actual_num = 1;
	while (NextObj(end) != nullptr && i < batch_num-1)
	{
		end = NextObj(end);
		++i;
		++actual_num;
	}
	span->free_list_ = NextObj(end);
	NextObj(end) = nullptr;
	span->use_count_ += actual_num;

	span_list_[index].mtx_.unlock();
	return actual_num;
}