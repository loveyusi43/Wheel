#include "thread_cache.h"
#include "central_cache.h"

void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t align_size = SizeClass::RoundUp(size);  // 申请的内存对齐后的大小
	size_t index = SizeClass::Index(size);         // 申请的内存位于那一个哈希桶内

	if (!free_list_[index].Empty())
	{
		return free_list_[index].Pop();
	}
	else
	{
		// 申请的批量内存第一个返回，多余的插入到对应的自由链表中
		return FetchFromCentralCache(index, align_size);  // 当这一级(thread cache)没有内存时向下一级(central cache)申请内存，此时以及在本级处理好了内存对齐规则，下一级不用管内存对齐规则。
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	// 找到对于的自由链表桶，将对象插入到其中
	size_t index = SizeClass::Index(size);
	free_list_[index].Push(ptr);

	// 对象不断返回，当对象的数量大于一定的值(一次批量申请的内存数量)时返回给central cache
	if (free_list_[index].Size() >= free_list_[index].MaxSize())
	{
		ListTooLong(free_list_[index], size);
	}
}

void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end, list.MaxSize());

	CentralCache::GetInstance()->ReleaseListToSpans(start, size);
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// 慢开始反馈调节算法
	// 申请任何对象的内存块时都从1开始逐个递增，上限在[2,512]之间(取决于对象的大小)
	#ifdef __linux__
		const size_t batch_num = std::min(free_list_[index].MaxSize()++, SizeClass::NumMoveSize(size));
	#elif _WIN32
		// 无论size多大, SizeClass::NumMoveSize的返回值都在[2,512]
		const size_t batch_num = min(free_list_[index].MaxSize()++, SizeClass::NumMoveSize(size));
	#endif

	// 批量内存块的起始和终止指针
	void* start = nullptr;
	void* end = nullptr;
	const size_t actual_num = CentralCache::GetInstance()->FetchRangeObj(start, end, batch_num, size);
	// 最少返回一块内存
	assert(actual_num >= 1);
	if (actual_num == 1)
	{
		assert(start == end);
	}
	else
	{
		// 多余的插入到对应的自由链表中
		free_list_[index].PushRange(NextObj(start), end, actual_num - 1);
	}

	return start;
}