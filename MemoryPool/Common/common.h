#ifndef COMMON_H
#define COMMON_H

#include <mutex>
#include <algorithm>
#include <unordered_map>

static const size_t MAX_BYTES = 256 * 1024; // thread_cache能申请的最大内存
static const size_t NFREE_LIST = 208; // 自由链表的总数
static const size_t PAGES = 129;
static const size_t PAGE_SHIFT = 13;

#include <malloc.h>
#include <new>

#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <unistd.h>
#endif


inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif __linux__
	void *ptr = sbrk(kpage << 13);
#endif

	if (nullptr == ptr)
		throw std::bad_alloc();
	return ptr;
}

inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// 
#endif
}

#ifdef _WIN64
	typedef unsigned long long PAGE_ID; // 2^64/2^13 = 2^51
#elif _WIN32
	typedef size_t PAGE_ID;  // 2^32/2^13 = 2^19
#elif __linux__
	typedef unsigned long long PAGE_ID;
#endif

#include <cassert>

static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

/*********************************************************************************************************/
// 管理切分好的小对象的内存
class FreeList
{
public:
	void Push(void* obj)
	{
		// 头插
		assert(obj);
		NextObj(obj) = free_list_;
		free_list_ = obj;
		++size_;
	}

	void PushRange(void* start, void* end, size_t n)
	{
		NextObj(end) = free_list_;
		free_list_ = start;
		size_ += n;
	}

	void PopRange(void*& start, void*& end, size_t n)
	{
		assert(n >= size_);
		start = free_list_; // 遗漏导致出错
		end = start;
		for (size_t i = 0; i < n-1; ++i)
		{
			end = NextObj(end);
		}
		free_list_ = NextObj(end);
		NextObj(end) = nullptr;
		size_ -= n;
	}

	void* Pop()
	{
		assert(free_list_);
		void* obj = free_list_;
		free_list_ = NextObj(free_list_);
		--size_;
		return obj;
	}

	bool Empty() const
	{
		return free_list_ == nullptr;
	}

	size_t Size() const
	{
		return size_;
	}

	size_t& MaxSize()
	{
		return max_size_;
	}

private:
	void* free_list_ = nullptr;
	size_t max_size_ = 1; // 向CentralCache申请内存块的起始个数
	size_t size_ = 0;
};

/***************************************************************************************************/

// 计算对象大小的对齐映射规则
class SizeClass
{
public:
/// <summary>
///	整体控制在最多10%的内碎片浪费
/// [1,128]                   8byte对齐              free_list_[0,16)
///	[128+1,1024]              16byte对齐             free_list_[16,72)
///	[1024+1,8*1024]           128byte对齐            free_list_[72,128)
///	[8*1024+1,64*1024]        1024byte对齐           free_list_[128,184)
///	[64*1024+1,256*1024]      8*1024byte对齐         free_list_[184,208)
/// </summary>

	// 不同的区间有不同的映射规则（对齐数不同）
	static inline size_t _RoundUp(const size_t size, const size_t align_num)
	{
		size_t align_size = 0;
		if (size%align_num != 0)  // 5%8 = 5 != 0
		{
			align_size = (size / align_num + 1) * align_num; // = (5/8+1)*8 = 1*8 = 8
		}
		else
		{
			align_size = size;
		}
		return align_size;
	}

	// 返回将size按照上述对齐规则对齐后的大小
	static inline size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8*1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64*1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256*1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			return _RoundUp(size, 1 << PAGE_SHIFT);
		}
	}

	static inline size_t _Index(size_t bytes, size_t align_num)
	{
		if (bytes % align_num == 0)
		{
			return bytes / align_num - 1;
		}
		else
		{
			return bytes / align_num;
		}
	}

	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);
		static int group_array[4] = { 16,56,56,56 };

		if (bytes <= 128)
		{
			return _Index(bytes, 8);
		}
		else if (bytes <= 1024)
		{
			return _Index(bytes - 128, 16) + group_array[0];
		}
		else if (bytes <= 8*1024)
		{
			return _Index(bytes - 1024, 128) + group_array[0] + group_array[1];
		}
		else if (bytes <= 64*1024)
		{
			return _Index(bytes - 8 * 1024, 1024) + group_array[0] + group_array[1] + group_array[2];
		}
		else if (bytes <= 256*1024)
		{
			return _Index(bytes - 64 * 1024, 8 * 1024) + group_array[0] + group_array[1] + group_array[2]+ group_array[3];
		}
		else
		{
			assert(false);
			return -1;
		}
	}

	static size_t NumMoveSize(size_t size)
	{
		assert(size > 0);

		size_t num = MAX_BYTES / size;
		// 根据申请内存对象的不同返回不同的内存数量，但返回的内存块不会过大或过小，将其控制在[2,512]范围内，小对象多返回一点内存块，大对象少返回一点内存块
		if (num < 2)
			num = 2;
		if (num > 512)
			num = 512;
		return num;
	}

	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size); // 当申请size大小的内存块时一次最多给num个
		size_t npage = num * size;  // 总字节数
		npage >>= PAGE_SHIFT;		// 一页为2^13字节，最后返回总共需要几页
		if (0 == npage)
		{
			npage = 1;
		}
		return npage;
	}
};

/*************************************************************************************************/

// 管理以页为单位的大块内存
// **在CentralCache中**每个Span内部把自己切为多个小块内存，当thread_cache申请内存时返回多块内存
// thread_cache是无锁的，所以在thread_cache中申请内存是最优的
struct Span
{
	// page_id_是每一页内存的编号(8k为一页)，在32位下最大编号为：2^32 / 2^13 = 2^19,而在64位平台下最大编号为：2^64 / 2^13 = 2^51,超出了一个整形的表示范围
	PAGE_ID page_id_ = 0; // 大块内存起始页号,一个Span有多个页，page_id_ 为起始页的页号，通过page_id_ 可以推算出Span 这块内存的起始地址
	size_t n_ = 0; // 页的数量

	Span* next_ = nullptr;
	Span* prev_ = nullptr;

	// use_count_ == 0时说明该Span的小块内存已经全部还回来了，即该Span一块"完整"的内存
	size_t use_count_ = 0; // 小块内存使用的数量(分配出去的小块内存的数量)
	void* free_list_ = nullptr; // 切好的小块内存的自由链表

	bool is_use_ = false;

	size_t object_size_ = 0; // 切好的内存块的大小
};

/************************************************************************************************/

// 带头双向循环链表
class SpanList
{
public:
	SpanList()
	{
		head_ = new Span();
		head_->next_ = head_;
		head_->prev_ = head_;
	}

	Span* Begin() const
	{
		return head_->next_;
	}

	Span* End() const
	{
		return head_;
	}

	bool Empty()
	{
		return head_->next_ == head_;
	}

	Span* PopFront()
	{
		Span* front = head_->next_;
		Erase(front);
		return front;
	}

	void PushFront(Span* span)
	{
		Insert(Begin(), span);
	}

	void Insert(Span* pos, Span* new_span)
	{
		assert(pos);
		assert(new_span);

		Span* prev = pos->prev_;
		prev->next_ = new_span;
		new_span->prev_ = prev;
		new_span->next_ = pos;
		pos->prev_ = new_span;
	}

	// 将Span从SpanList中弹出，还给上一层的PageCache
	void Erase(Span* pos) const
	{
		assert(pos && pos != head_);

		Span* prev = pos->prev_;
		Span* next = pos->next_;

		prev->next_ = next;
		next->prev_ = prev;
	}

private:
	Span* head_;

public:
	std::mutex mtx_;  // 每个桶一个锁，而不是一把锁将central cache锁死
};

#endif