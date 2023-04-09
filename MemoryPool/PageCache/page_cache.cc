#include "page_cache.h"

PageCache PageCache::instance_;

Span* PageCache::NewSpan(size_t k)
{
	if (k >= PAGES)
	{
		void* ptr = SystemAlloc(k);
		//Span* span = new Span;
		Span* span = span_pool_.New();
		span->page_id_ = ((PAGE_ID)ptr >> PAGE_SHIFT);
		span->n_ = k;
		id_span_map_[span->page_id_] = span;
		return span;
	}

	assert(k > 0 && k < PAGES);

	if (!span_list_[k].Empty())
	{
		Span* k_span = span_list_[k].PopFront();
		for (PAGE_ID i = 0; i < k_span->n_; ++i)
		{
			id_span_map_[k_span->page_id_ + i] = k_span;
		}
		return k_span;
	}

	// 检查后面的桶中有没有Span
	for (size_t i = k+1; i < PAGES; ++i)
	{
		if (!span_list_[i].Empty())
		{
			// 向后找到一个有Span的桶
			Span* n_span = span_list_[i].PopFront();
			//Span* k_span = new Span;
			Span* k_span = span_pool_.New();

			// 从n_span的头部开始切一个k页的span
			k_span->page_id_ = n_span->page_id_;
			k_span->n_ = k;

			n_span->page_id_ += k;
			n_span->n_ -= k;

			span_list_[n_span->n_].PushFront(n_span);

			id_span_map_[n_span->page_id_] = n_span;
			id_span_map_[n_span->page_id_ + n_span->n_ - 1] = n_span;

			// 建立Span ID 和Span 的映射, 方便central cache 回收小块内存时查找对应的Span
			for (PAGE_ID i = 0; i < k_span->n_; ++i)
			{
				id_span_map_[k_span->page_id_ + i] = k_span;
			}

			return k_span;
		}
	}

	Span* big_span = new Span();
	void* ptr = SystemAlloc(PAGES - 1); // 以页为单位申请内存
	big_span->page_id_ = (PAGE_ID)ptr >> PAGE_SHIFT;
	big_span->n_ = PAGES - 1;
	span_list_[big_span->n_].PushFront(big_span);

	return NewSpan(k);
}

Span* PageCache::MapObjectToSpan(void* obj)
{
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;

	std::unique_lock<std::mutex> lock(page_mtx_);

	auto ret = id_span_map_.find(id);
	if (ret != id_span_map_.end())
	{
		return ret->second;
	}
	else
	{
		assert(false);
		return nullptr;
	}
}


void PageCache::ReleaseSpanToPageCache(Span* span)
{
	if (span->n_ >= PAGES)
	{
		void* ptr = (void*)(span->page_id_ << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		span_pool_.Delete(span);
		return;
	}

	// PageCache 向前合并
	while (1)
	{
		PAGE_ID prev_id = span->page_id_ - 1;
		auto ret = id_span_map_.find(prev_id);

		if (ret == id_span_map_.end())
		{
			break;
		}
		// 前面相邻页在CentralCache中被使用
		Span* prev_span = ret->second;
		if (prev_span->is_use_ == true)
		{
			break;
		}
		if (prev_span->n_ + span->n_ >= PAGES)
		{
			break;
		}

		span->page_id_ = prev_span->page_id_;
		span->n_ += prev_span->n_;
		span_list_[prev_span->n_].Erase(prev_span);
		//delete prev_span;
		span_pool_.Delete(prev_span);
	}

	// 向后合并
	while (1)
	{
		PAGE_ID next_id = span->page_id_ + span->n_;
		auto ret = id_span_map_.find(next_id);
		if (ret == id_span_map_.end())
		{
			break;
		}
		Span* next_span = ret->second;
		if (next_span->is_use_ == true)
		{
			break;
		}
		if (next_span->n_ + span->n_ >= PAGES)
		{
			break;
		}

		span->n_ += next_span->n_;
		span_list_[next_span->n_].Erase(next_span);
		//delete next_span;
		span_pool_.Delete(next_span);
	}

	span_list_[span->n_].PushFront(span);
	span->is_use_ = false;
	id_span_map_[span->page_id_] = span;
	id_span_map_[span->page_id_ + span->n_ - 1] = span;
}