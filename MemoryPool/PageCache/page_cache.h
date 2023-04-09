#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include "common.h"
#include "object_pool.h"

class PageCache
{
public:
	PageCache(const PageCache&) = delete;

	static PageCache* GetInstance()
	{
		return &instance_;
	}

	Span* NewSpan(size_t k); // 获取一个k页的Span

	Span* MapObjectToSpan(void* obj);

	void ReleaseSpanToPageCache(Span* span);

	std::mutex page_mtx_;

private:
	PageCache() = default;

	ObjectPool<Span> span_pool_;

	SpanList span_list_[PAGES];

	static PageCache instance_;

	std::unordered_map<PAGE_ID, Span*> id_span_map_;
};

#endif