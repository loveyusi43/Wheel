#include "concurrent_alloc.h"

void* ConcurrentAlloc(size_t size)
{
	if (size > MAX_BYTES) // 当申请的内存大于MAX_BYTES时，直接向page_cache申请内存
	{
		size_t align_size = SizeClass::RoundUp(size);
		size_t k_page = align_size >> PAGE_SHIFT;

		PageCache::GetInstance()->page_mtx_.lock();
		Span* span = PageCache::GetInstance()->NewSpan(k_page);
		span->object_size_ = size;
		PageCache::GetInstance()->page_mtx_.unlock();

		void* ptr = (void*)(span->page_id_ << PAGE_SHIFT);
		return ptr;
	}
	else
	{
		if (p_tls_thread_cache == nullptr)
		{
			//p_tls_thread_cache = new ThreadCache();
			static ObjectPool<ThreadCache> tc_pool;
			p_tls_thread_cache = tc_pool.New();
		}
		return p_tls_thread_cache->Allocate(size);
	}
}

void ConcurrentFree(void* ptr)
{
	Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
	size_t size = span->object_size_;
	if (size > MAX_BYTES)
	{
		PageCache::GetInstance()->page_mtx_.lock();
		PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		PageCache::GetInstance()->page_mtx_.unlock();
	}
	else
	{
		assert(p_tls_thread_cache);
		p_tls_thread_cache->Deallocate(ptr, size);
	}
}