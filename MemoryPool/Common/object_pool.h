#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include "common.h"

template<class T>
class ObjectPool
{
public:
	T* New()
	{
		// 优先考虑自由链表是否有还回来的内存块；
		T* obj = nullptr;
		if (free_list_)
		{
			void* next = *(void**)free_list_; // 如果p是一个指针(指向一块大小未知的内存块),则*(void**)p可以得到该内存块的前4个或8个字节
			obj = (T*)free_list_;
			free_list_ = next;
			new(obj)T;  // 定位new，初始化一块已有的内存块
			return obj;
		}

		// 当剩余的内存不够一个对象大小时开辟新的大块内存
		if (remain_bytes_ < sizeof(T))
		{
			remain_bytes_ = 128 * 1024;
			memory_ = (char*)SystemAlloc(remain_bytes_ >> 13);
			if (nullptr == memory_)
			{
				throw std::bad_alloc();
			}
		}
		obj = (T*)memory_;
		const size_t object_size_ = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
		memory_ += object_size_;
		remain_bytes_ -= object_size_;
		new(obj)T;
		return obj;
	}

	void Delete(T* obj)
	{
		obj->~T();
		*(void**)obj = free_list_;  // 指针的大小会根据所处的平台不同而改变
		free_list_ = (void*)obj;
	}

private:
	char* memory_ = nullptr;       // 指向大块内存的指针
	void* free_list_ = nullptr;    // 管理回收内存块的自由链表的头指针
	size_t remain_bytes_ = 0;     // 大块内存剩余的字节数
};

#endif