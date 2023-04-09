#ifndef CENTRAL_CACHE_H
#define CENTRAL_CACHE_H

#include "common.h"

class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &instance_;
	}

	// 返回一个非空的Span
	Span* GetOneSpan(SpanList& list, size_t size);

	// 当ThreadCache调用时返回批量的内存块
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);

	void ReleaseListToSpans(void* start, size_t byte_size);

private:
	CentralCache() = default;

	CentralCache(const CentralCache&) = delete;

	// 哈希桶的映射规则与ThreadCache相同，所以桶的数量也是一样的
	SpanList span_list_[NFREE_LIST];

	static CentralCache instance_;
};

#endif