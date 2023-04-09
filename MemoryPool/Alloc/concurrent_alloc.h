#ifndef CONCURRENT_ALLOC_H
#define CONCURRENT_ALLOC_H

#include "common.h"
#include "thread_cache.h"
#include "page_cache.h"
#include "object_pool.h"

void* ConcurrentAlloc(size_t size);

void ConcurrentFree(void* ptr);

#endif