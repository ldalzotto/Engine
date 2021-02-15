#pragma once

#include <climits>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <stdlib.h>

#define COMMON2_MACROS
#define COMMON2_TYPES
#define COMMON2_LIMITS
#define COMMON2_PLATFORM_INCLUDE
#define COMMON2_CLOCK
#define COMMON2_THREAD
#define COMMON2_ASSERT
#define COMMON2_MEMORY
#define COMMON2_SLICE
#define COMMON2_SPAN
#define COMMON2_TOKEN
#define COMMON2_SORT
#define COMMON2_VECTOR
#define COMMON2_POOL
#define COMMON2_STRING
#define COMMON2_VARYINGVECTOR
#define COMMON2_POOLINDEXED
#define COMMON2_VECTOROFVECTOR
#define COMMON2_POOLOFVECTOR
#define COMMON2_HEAP
#define COMMON2_TREE
#define COMMON2_HEAPMEMORY
#define COMMON2_HASH
#define COMMON2_STRINGFUNCTIONS
#define COMMON2_SERIALIZATION_JSON
#define COMMON2_SERIALIZATION_TYPES
#define COMMON2_CONTAINER_ITERATIONS

#ifdef COMMON2_MACROS

#define COMA ,
#define SEMICOLON ;
#define STR(V1) #V1
#define CONCAT_2(V1, V2) V1 ## V2

#define LINE_RETURN \n
#define MULTILINE(...) #__VA_ARGS__

#define cast(Type, Value) ((Type)(Value))
#define castv(Type, Value) *(Type*)(&Value)

#define loop(Iteratorname, BeginNumber, EndNumber) uimax Iteratorname = BeginNumber; Iteratorname < EndNumber; Iteratorname++
#define loop_int16(Iteratorname, BeginNumber, EndNumber) int16 Iteratorname = BeginNumber; Iteratorname < EndNumber; Iteratorname++

#define loop_reverse(Iteratorname, BeginNumber, EndNumber) uimax Iteratorname = BeginNumber; Iteratorname != EndNumber; --Iteratorname

#define vector_loop(VectorVariable, Iteratorname) uimax Iteratorname = 0; Iteratorname < (VectorVariable)->Size; Iteratorname++
#define vector_loop_reverse(VectorVariable, Iteratorname) uimax Iteratorname = (VectorVariable)->Size - 1; Iteratorname != ((uimax)-1); --Iteratorname

#define pool_loop(PoolVariable, Iteratorname) uimax Iteratorname = 0; Iteratorname < (PoolVariable)->get_size(); Iteratorname++

#define varyingvector_loop(VaryingVectorVariable, Iteratorname) uimax Iteratorname = 0; Iteratorname < (VaryingVectorVariable)->get_size(); Iteratorname++

#endif

#ifdef COMMON2_TYPES

typedef char int8;
typedef unsigned char uint8;

typedef short int16;
typedef unsigned short uint16;

typedef long int32;
typedef unsigned long uint32;

typedef long long int64;
typedef unsigned long long uint64;

typedef float float32;
typedef double float64;
typedef long double float128;

typedef size_t uimax;

const uint8 uint8_max = UCHAR_MAX;

#endif

#ifdef COMMON2_LIMITS

namespace v2
{
	namespace Limits
	{
		// const int8* float_string_format = "%f.5";
		const int8 tol_digit_number = 5;
		const float64 tol_f = ((float64)1 / pow(10, tol_digit_number));
	};

#define float_string_format %.5f
#define float_string_format_str "%.5f"
}

#endif

#ifdef COMMON2_PLATFORM_INCLUDE

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif


#include <Windows.h>
#include <sysinfoapi.h>

inline uint64 FILETIME_to_mics(FILETIME& p_filetime)
{
	ULARGE_INTEGER ul;
	ul.LowPart = p_filetime.dwLowDateTime;
	ul.HighPart = p_filetime.dwHighDateTime;
	return ul.QuadPart / 10;
};

#elif __linux__

#include <time.h>
#include <unistd.h>
#include <pthread.h>

#endif
#endif

#ifdef COMMON2_CLOCK

struct Clock
{
	uimax framecount;
	float deltatime;

	inline static Clock allocate_default()
	{
		return Clock{ 0,0.0f };
	};

	inline void newframe()
	{
		this->framecount += 1;
	}

	inline void newupdate(float p_delta)
	{
		this->deltatime = p_delta;
	}
};

// typedef uint64 time_t;


time_t clock_currenttime_mics();

#ifdef _WIN32

inline time_t clock_currenttime_mics()
{
	FILETIME l_currentTime;
	GetSystemTimeAsFileTime(&l_currentTime);
	return FILETIME_to_mics(l_currentTime);
};

#elif __linux__

inline time_t clock_currenttime_mics()
{
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	return round(spec.tv_nsec / 1000);
};

#endif

#endif

#ifdef COMMON2_THREAD

using thread_t
#ifdef _WIN32
= HANDLE
#elif __linux__
= pthread_t
#endif // _WIN32
;


struct Thread
{
	static thread_t get_current_thread();
	static void wait(const uimax p_time_in_ms);
};

#ifdef _WIN32

inline thread_t Thread::get_current_thread()
{
	return GetCurrentThread();
};

inline void Thread::wait(const uimax p_time_in_ms)
{
	WaitForSingleObject(get_current_thread(), (DWORD)p_time_in_ms);
};

#elif __linux__

inline void Thread::wait(const uimax p_time_in_ms)
{
	usleep(p_time_in_ms * 1000);
};


#endif

#endif

#ifdef COMMON2_ASSERT

inline constexpr void assert_true(int8 p_condition)
{
	if (!p_condition) { abort(); }
};

#endif

#ifdef COMMON2_MEMORY

#if MEM_LEAK_DETECTION

#define MEM_LEAK_MAX_POINTER_COUNTER 10000
#define MEM_LEAK_MAX_BACKTRACE 64

int8* ptr_counter[MEM_LEAK_MAX_POINTER_COUNTER] = {};

typedef LPVOID backtrace_t[MEM_LEAK_MAX_BACKTRACE];

backtrace_t backtraces[MEM_LEAK_MAX_POINTER_COUNTER] = {};

#include "dbghelp.h"

inline void capture_backtrace(const uimax p_ptr_index)
{
	CaptureStackBackTrace(0, MEM_LEAK_MAX_BACKTRACE, backtraces[p_ptr_index], NULL);
};


inline int8* push_ptr_to_tracked(int8* p_ptr)
{
	for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
	{
		if (ptr_counter[i] == NULL)
		{
			ptr_counter[i] = p_ptr;
			capture_backtrace(i);
			return p_ptr;
		}
	}

	abort();
};


inline void remove_ptr_to_tracked(int8* p_ptr)
{
	for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
	{
		if (ptr_counter[i] == p_ptr)
		{
			ptr_counter[i] = NULL;
			return;
		}
	}

	abort();
};

#endif

inline void memleak_ckeck()
{
#if MEM_LEAK_DETECTION
	for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
	{
		if (ptr_counter[i] != NULL)
		{
			abort();
		}
	}
#endif
};


inline int8* heap_malloc(const uimax p_size)
{
#if MEM_LEAK_DETECTION
	return push_ptr_to_tracked((int8*)::malloc(p_size));
#else
	return (int8*)::malloc(p_size);
#endif

};

inline int8* heap_calloc(const uimax p_size)
{
#if MEM_LEAK_DETECTION
	return push_ptr_to_tracked((int8*)::calloc(1, p_size));
#else
	return (int8*)::calloc(1, p_size);
#endif
};


inline int8* heap_realloc(int8* p_memory, const uimax p_new_size)
{
#if MEM_LEAK_DETECTION
	remove_ptr_to_tracked(p_memory);
	return push_ptr_to_tracked((int8*)::realloc(p_memory, p_new_size));
#else
	return (int8*)::realloc(p_memory, p_new_size);
#endif
};

inline void heap_free(int8* p_memory)
{
#if MEM_LEAK_DETECTION
	remove_ptr_to_tracked(p_memory);
#endif
	::free(p_memory);
};


inline int8* memory_cpy(int8* p_dst, const int8* p_src, const uimax p_size)
{
	return (int8*)::memcpy(p_dst, p_src, p_size);
};

inline static int8* memory_cpy_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	if (p_size > p_dst_size)
	{
		abort();
	}
#endif

	return memory_cpy(p_dst, p_src, p_size);
};


inline int8* memory_move(int8* p_dst, const int8* p_src, const uimax p_size)
{
	return (int8*)::memmove(p_dst, p_src, p_size);
};


inline int8* memory_move_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	if (p_size > p_dst_size)
	{
		abort();
	}
#endif

	return memory_move(p_dst, p_src, p_size);
};

inline int8 memory_compare(const int8* p_source, const int8* p_compared, const uimax p_size)
{
	return ::memcmp(p_source, p_compared, p_size) == 0;
};


template<class ElementType>
inline uimax memory_offset_bytes(const uimax p_size)
{
	return sizeof(ElementType) * p_size;
};


inline constexpr uimax strlen_constexpr(const char* start)
{
	const char* end = start;
	while (*end++ != 0);
	return end - start - 1;
};

#endif

#ifdef COMMON2_SLICE


/*
	A Slice is an encapsulated C style array.
*/
template<class ElementType>
struct Slice
{
	uimax Size;
	ElementType* Begin;

	inline static Slice<ElementType> build_default()
	{
		return Slice<ElementType>{0, NULL};
	};

	inline static Slice<ElementType> build(ElementType* p_memory, const uimax p_begin, const uimax p_end)
	{
		return Slice<ElementType>{p_end - p_begin, p_memory + p_begin};
	};

	inline static Slice<ElementType> build_memory_elementnb(ElementType* p_memory, const uimax p_element_nb)
	{
		return Slice<ElementType>{p_element_nb, p_memory};
	};

	inline static Slice<ElementType> build_memory_offset_elementnb(ElementType* p_memory, const uimax p_offset, const uimax p_element_nb)
	{
		return Slice<ElementType>{p_element_nb, p_memory + p_offset};
	};

	inline static Slice<int8> build_asint8(ElementType* p_memory, const uimax p_begin, const uimax p_end)
	{
		return Slice<int8>{sizeof(ElementType)* (p_end - p_begin), cast(int8*, (p_memory + p_begin))};
	};

	inline Slice<int8> build_asint8() const
	{
		return Slice<int8>{sizeof(ElementType)* this->Size, cast(int8*, this->Begin)};
	};

	inline static Slice<int8> build_asint8_memory_elementnb(const ElementType* p_memory, const uimax p_element_nb)
	{
		return Slice<int8>{sizeof(ElementType)* p_element_nb, cast(int8*, p_memory)};
	};

	inline static Slice<int8> build_asint8_memory_singleelement(const ElementType* p_memory)
	{
		return Slice<int8>{sizeof(ElementType), cast(int8*, p_memory)};
	};

	inline ElementType& get(const uimax p_index)
	{
#if CONTAINER_BOUND_TEST
		if (p_index >= this->Size)
		{
			abort();
		}
#endif
		return this->Begin[p_index];
	};

	inline const ElementType& get(const uimax p_index) const
	{
		return ((Slice<ElementType>*)this)->get(p_index);
	};

	inline void slide(const uimax p_offset_index)
	{
#if CONTAINER_BOUND_TEST
		if (p_offset_index > this->Size)
		{
			abort();
		};
#endif

		this->Begin = this->Begin + p_offset_index;
		this->Size -= p_offset_index;
	};

	inline Slice<ElementType> slide_rv(const uimax p_offset_index) const
	{
		Slice<ElementType> l_return = *this;
		l_return.slide(p_offset_index);
		return l_return;
	};

	inline int8 compare(const Slice<ElementType>& p_other) const
	{
		return slice_memcompare_element(*this, p_other);
	};

	inline int8 find(const Slice<ElementType>& p_other, uimax* out_index) const
	{
		return slice_memfind(*this, p_other, out_index);
	};
};

inline Slice<int8> slice_int8_build_rawstr(const int8* p_str)
{
	return Slice<int8>::build_memory_elementnb((int8*)p_str, strlen(p_str));
};


template<class CastedType>
inline Slice<CastedType> slice_cast(const Slice<int8>& p_slice)
{
#if CONTAINER_BOUND_TEST
	if ((p_slice.Size % sizeof(CastedType)) != 0)
	{
		abort();
	}
#endif

	return Slice<CastedType>{ cast(uimax, p_slice.Size / sizeof(CastedType)), cast(CastedType*, p_slice.Begin) };
};


template<class CastedType>
inline CastedType* slice_cast_singleelement(const Slice<int8>& p_slice)
{
#if CONTAINER_BOUND_TEST
	if (p_slice.Size < sizeof(CastedType))
	{
		abort();
	}
#endif
	return cast(CastedType*, p_slice.Begin);
};

template<class CastedType>
inline Slice<CastedType> slice_cast_fixedelementnb(const Slice<int8>& p_slice, const uimax p_element_nb)
{
#if CONTAINER_BOUND_TEST
	if (p_slice.Size < (sizeof(CastedType) * p_element_nb))
	{
		abort();
	}
#endif

	return slice_build_memory_elementnb(cast(CastedType*, p_slice.Begin), p_element_nb);
};

#if TOKEN_TYPE_SAFETY
#define sliceoftoken_cast(CastedType, SourceSlice) Slice<Token(CastedType)>{(SourceSlice).Size, (Token(CastedType)*)(SourceSlice).Begin}
#else
#define sliceoftoken_cast(CastedType, SourceSlice) SourceSlice
#endif





template<class ElementType>
inline int8* slice_memmove(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	return memory_move_safe(cast(int8*, p_target.Begin), p_target.Size * sizeof(ElementType), cast(int8*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
	return memory_move((int8*)p_target.Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

template<class ElementType>
inline int8* slice_memcpy(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	return memory_cpy_safe(cast(int8*, p_target.Begin), p_target.Size * sizeof(ElementType), cast(int8*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
	return memory_cpy((int8*)p_target.Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

template<class ElementType>
inline int8 slice_memcompare_element(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared)
{
	return memory_compare(cast(int8*, p_target.Begin), cast(int8*, p_compared.Begin), p_compared.Size);
};

template<class ElementType>
inline int8 slice_memfind(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared, uimax* out_index)
{
#if CONTAINER_BOUND_TEST
	if (p_compared.Size > p_target.Size)
	{
		abort();
	}
#endif

	Slice<ElementType> l_target_slice = p_target;
	if (slice_memcompare_element(l_target_slice, p_compared))
	{
		*out_index = 0;
		return 1;
	};

	for (uimax i = 1; i < p_target.Size - p_compared.Size + 1; i++)
	{
		l_target_slice.slide(1);
		if (slice_memcompare_element(l_target_slice, p_compared))
		{
			*out_index = i;
			return 1;
		};
	};

	*out_index = -1;
	return 0;
};

/*
	A SliceIndex is just a begin and end uimax
*/
struct SliceIndex
{
	uimax Begin;
	uimax Size;

	inline static SliceIndex build(const uimax p_begin, const uimax p_size)
	{
		return SliceIndex{ p_begin, p_size };
	};

	inline static SliceIndex build_default()
	{
		return build(0, 0);
	};

	inline void slice_two(const uimax p_break_point, SliceIndex* out_left, SliceIndex* out_right) const
	{
		uimax l_source_initial_size = this->Size;
		*out_left = SliceIndex::build(this->Begin, p_break_point - this->Begin);
		*out_right = SliceIndex::build(p_break_point, l_source_initial_size - out_left->Size);
	};
};

template<class ElementType>
struct SliceOffset
{
	uimax Offset;
	ElementType* Memory;

	inline static SliceOffset<ElementType> build(ElementType* p_memory, const uimax p_offset)
	{
		return SliceOffset<ElementType> {p_offset, p_memory	};
	};

	inline static SliceOffset<ElementType> build_from_sliceindex(ElementType* p_memory, const SliceIndex& p_slice_index)
	{
		return SliceOffset<ElementType> {p_slice_index.Begin, p_memory};
	};

};

#endif

#ifdef COMMON2_SPAN


/*
	A Span is a heap allocated chunk of memory.
	Span can allocate memory, be resized and freed.
*/
template<class ElementType>
struct Span
{
	union
	{
		struct
		{
			uimax Capacity;
			ElementType* Memory;
		};
		Slice<ElementType> slice;
	};

	inline static Span<ElementType> build(ElementType* p_memory, const uimax p_capacity)
	{
		return Span<ElementType>{ p_capacity, p_memory };
	};

	inline static Span<ElementType> allocate(const uimax p_capacity)
	{
		return Span<ElementType>{ p_capacity, cast(ElementType*, heap_malloc(p_capacity * sizeof(ElementType))) };
	};

	template<uint8 Capacity_t>
	inline static Span<ElementType> allocate_array(const ElementType p_elements[Capacity_t])
	{
		Span<ElementType> l_span = Span<ElementType>::allocate(Capacity_t);
		slice_memcpy(l_span.slice, Slice<ElementType>::build_memory_elementnb((ElementType*)p_elements, Capacity_t));
		return l_span;
	};

	inline static Span<ElementType> callocate(const uimax p_capacity)
	{
		return Span<ElementType>{ p_capacity, cast(ElementType*, heap_calloc(p_capacity * sizeof(ElementType))) };
	};

	inline ElementType& get(const uimax p_index)
	{
#if CONTAINER_BOUND_TEST
		assert_true(p_index < this->Capacity);
#endif
		return this->Memory[p_index];
	};

	inline int8 resize(const uimax p_new_capacity)
	{
		if (p_new_capacity > this->Capacity)
		{
			ElementType* l_newMemory = (ElementType*)heap_realloc(cast(int8*, this->Memory), p_new_capacity * sizeof(ElementType));
			if (l_newMemory != NULL)
			{
				*this = Span<ElementType>::build(l_newMemory, p_new_capacity);
				return 1;
			}
			return 0;
		}
		return 1;
	};

	inline const ElementType& get(const uimax p_index) const
	{
		return ((Span<ElementType>*)(this))->get(p_index);
	};

	inline Span<ElementType> move_to_value()
	{
		Span<ElementType> l_return = *this;
		*this = Span<ElementType>::build(NULL, 0);
		return l_return;
	};

	inline void resize_until_capacity_met(const uimax p_desired_capacity)
	{
		uimax l_resized_capacity = this->Capacity;

		if (l_resized_capacity >= p_desired_capacity)
		{
			return;
		}

		if (l_resized_capacity == 0)
		{
			l_resized_capacity = 1;
		}

		while (l_resized_capacity < p_desired_capacity)
		{
			l_resized_capacity *= 2;
		}

		this->resize(l_resized_capacity);
	};

	inline void free()
	{
		heap_free(cast(int8*, this->Memory));
		*this = Span<ElementType>::build(NULL, 0);
	};

	inline void bound_inside_check(const Slice<ElementType>& p_tested_slice)
	{
#if CONTAINER_BOUND_TEST
		if ((p_tested_slice.Begin + p_tested_slice.Size) > (this->Memory + this->Capacity))
		{
			abort();
		}
#endif
	};


	inline void bound_check(const uimax p_index)
	{
#if CONTAINER_BOUND_TEST
		if (p_index > this->Capacity)
		{
			abort();
		}
#endif
	};

	inline void move_memory_down(const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
	{
		Slice<ElementType> l_target = Slice<ElementType>::build_memory_offset_elementnb(this->Memory, p_break_index + p_move_delta, p_moved_block_size);
#if CONTAINER_BOUND_TEST
		this->bound_inside_check(l_target);
#endif
		Slice<ElementType> l_source = Slice<ElementType>::build(this->Memory, p_break_index, p_break_index + p_moved_block_size);
		slice_memmove(l_target, l_source);
	};

	inline void move_memory_up(const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
	{
		Slice<ElementType> l_target = Slice<ElementType>::build_memory_offset_elementnb(this->Memory, p_break_index - p_move_delta, p_moved_block_size);
#if CONTAINER_BOUND_TEST
		this->bound_inside_check(l_target);
#endif
		Slice<ElementType> l_source = Slice<ElementType>::build(this->Memory, p_break_index, p_break_index + p_moved_block_size);
		slice_memmove(l_target, l_source);
	};

	inline void copy_memory(const uimax p_copy_index, const Slice<ElementType>& p_elements)
	{
		Slice<ElementType> l_target = Slice<ElementType>::build_memory_elementnb(this->Memory + p_copy_index, p_elements.Size);

#if CONTAINER_BOUND_TEST
		this->bound_inside_check(l_target);
#endif

		slice_memcpy(l_target, p_elements);
	};
};





#endif

#ifdef COMMON2_TOKEN

typedef uimax token_t;

inline token_t tokent_build_default()
{
	return -1;
};

inline void tokent_reset(token_t* p_token)
{
	*p_token = tokent_build_default();
};

inline int8 tokent_is_valid(const token_t* p_token)
{
	return *p_token != (token_t)-1;
};

#if TOKEN_TYPE_SAFETY

template<class ElementType>
struct Token
{
	token_t tok;
};

#define tk_b(ElementType, TokenT) Token<ElementType> { (token_t)(TokenT)}

#define tk_v(TokenVariable) ((TokenVariable).tok)

#define Token(ElementType) Token<ElementType>

#else

#define tk_b(ElementType, TokenT) (token_t)TokenT

#define tk_v(TokenVariable) (TokenVariable)

#define Token(ElementType) uimax

#endif

#define tk_bd(ElementType) tk_b(ElementType, -1)
#define tk_bf(ElementType, SourceToken) tk_b(ElementType, tk_v(SourceToken))
#define tk_eq(Left, Right) (tk_v(Left) == tk_v(Right))
#define tk_neq(Left, Right) (tk_v(Left) != tk_v(Right))

#endif

#ifdef COMMON2_SORT


struct Sort
{
	//TODO ->Implement Quick sort
	template<class ElementType, class CompareFunc>
	inline static void Linear2(Slice<ElementType>& p_slice, const uimax p_start_index)
	{
		for (loop(i, p_start_index, p_slice.Size))
		{
			ElementType& l_left = p_slice.get(i);
			for (loop(j, i, p_slice.Size))
			{
				ElementType& l_right = p_slice.get(j);
				if (CompareFunc::compare(l_left, l_right))
				{
					ElementType l_left_tmp = l_left;
					l_left = l_right;
					l_right = l_left_tmp;
				}
			}
		}
	};
};


#define sort_linear2_begin(ElementType, StructName) \
struct StructName\
{ \
	inline static int8 compare(const ElementType& p_left, const ElementType& p_right)\
	{

#define sort_linear2_end(SliceVariable, ElementType, StructName) \
	};\
};\
auto l_slice_variable_##StructName = (SliceVariable); \
Sort::Linear2<ElementType, StructName>(l_slice_variable_##StructName, 0);



#endif

#ifdef COMMON2_VECTOR


//TODO -> delete when common2 migration is complete
namespace v2
{

	/*
		A Vector is a Span with an imaginary boundary (Size).
		Vector memory is continuous, there is no "hole" between items.
		Vector is reallocated on need.
		Any memory access outside of this imaginary boundary will be in error.
		The Vector expose some safe way to insert/erase data (array or single element).
	*/
	template<class ElementType>
	struct Vector
	{
		uimax Size;
		Span<ElementType> Memory;

		inline static Vector<ElementType> build(ElementType* p_memory, const uimax p_initial_capacity)
		{
			return Vector<ElementType>{ 0, Span<ElementType>::build(p_memory, p_initial_capacity) };
		};

		inline static Vector<ElementType> allocate(const uimax p_initial_capacity)
		{
			return Vector<ElementType>{ 0, Span<ElementType>::allocate(p_initial_capacity) };
		};

		inline static Vector<ElementType> allocate_elements(const Slice<ElementType>& p_initial_elements)
		{
			Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_initial_elements.Size);
			l_vector.push_back_array(p_initial_elements);
			return l_vector;
		};

		inline static Vector<ElementType> allocate_capacity_elements(const uimax p_inital_capacity, const Slice<ElementType>& p_initial_elements)
		{
			Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_inital_capacity);
			l_vector.push_back_array(p_initial_elements);
			return l_vector;
		};

		inline Slice<ElementType> to_slice() const
		{
			return Slice<ElementType>::build_memory_elementnb(this->Memory.Memory, this->Size);
		};

		inline void free()
		{
			this->Memory.free();
			*this = Vector<ElementType>::build(NULL, 0);
		};

		inline ElementType* get_memory()
		{
			return this->Memory.Memory;
		};

		inline uimax get_size()
		{
			return this->Size;
		};

		inline uimax get_capacity()
		{
			return this->Memory.Capacity;
		};

		inline int8 empty()
		{
			return this->Size == 0;
		};

		inline ElementType& get(const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index);
#endif
			return this->Memory.Memory[p_index];
		};

		inline const ElementType& get(const uimax p_index) const
		{
			return ((Vector<ElementType>*)this)->get(p_index);
		};

		inline void clear()
		{
			this->Size = 0;
		};


		inline int8 insert_array_at(const Slice<ElementType>& p_elements, const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

			return this->insert_array_at_unchecked(p_elements, p_index);
		};


		inline int8 insert_element_at(const ElementType& p_element, const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif

			return this->insert_element_at_unchecked(p_element, p_index);
		};

		inline int8 push_back_array(const Slice<ElementType>& p_elements)
		{
			this->Memory.resize_until_capacity_met(this->Size + p_elements.Size);
			this->Memory.copy_memory(this->Size, p_elements);
			this->Size += p_elements.Size;

			return 1;
		};

		inline int8 push_back_array_empty(const uimax p_array_size)
		{
			this->Memory.resize_until_capacity_met(this->Size + p_array_size);
			this->Size += p_array_size;
			return 1;
		};

		inline int8 push_back_element_empty()
		{
			this->Memory.resize_until_capacity_met(this->Size + 1);
			this->Size += 1;
			return 1;
		};

		inline int8 push_back_element(const ElementType& p_element)
		{
			this->Memory.resize_until_capacity_met(this->Size + 1);
			this->Memory.Memory[this->Size] = p_element;
			this->Size += 1;

			return 1;
		};


		inline int8 insert_array_at_always(const Slice<ElementType>& p_elements, const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
#endif
			if (p_index == this->Size)
			{
				return this->push_back_array(p_elements);
			}
			else
			{
				return this->insert_array_at_unchecked(p_elements, p_index);
			}
		};


		inline int8 insert_element_at_always(const ElementType& p_element, const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
#endif

			if (p_index == this->Size)
			{
				return this->push_back_element(p_element);
			}
			else
			{
				return this->insert_element_at_unchecked(p_element, p_index);
			}
		};


		inline int8 erase_array_at(const uimax p_index, const uimax p_element_nb)
		{

#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_check(p_index + p_element_nb);
			this->bound_head_check(p_index); // use vector_pop_back_array //TODO -> create a "always" variant of vector_erase_array_at
#endif

			this->move_memory_up(p_index + p_element_nb, p_element_nb);
			this->Size -= p_element_nb;

			return 1;
		};

		inline int8 erase_element_at(const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index); // use vector_pop_back
#endif

			this->move_memory_up(p_index + 1, 1);
			this->Size -= 1;

			return 1;
		};

		inline int8 pop_back_array(const uimax p_element_nb)
		{
			this->Size -= p_element_nb;
			return 1;
		};

		inline int8 pop_back()
		{
			this->Size -= 1;
			return 1;
		};

		inline int8 erase_element_at_always(const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
#endif

			if (p_index == this->Size)
			{
				return this->pop_back();
			}
			else
			{
				return this->erase_element_at(p_index);
			}
		};

	private:

		inline void bound_check(const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			if (p_index > this->Size)
			{
				abort();
			}
#endif
		};

		inline void bound_head_check(const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			if (p_index == this->Size)
			{
				abort();
			}
#endif
		};


		inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
		{
			this->Memory.move_memory_down(this->Size - p_break_index, p_break_index, p_move_delta);
		};

		inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
		{
			this->Memory.move_memory_up(this->Size - p_break_index, p_break_index, p_move_delta);
		};

		inline int8 insert_element_at_unchecked(const ElementType& p_element, const uimax p_index)
		{
			this->Memory.resize_until_capacity_met(this->Size + 1);
			this->move_memory_down(p_index, 1);
			this->Memory.Memory[p_index] = p_element;
			this->Size += 1;

			return 1;
		};

		inline int8 insert_array_at_unchecked(const Slice<ElementType>& p_elements, const uimax p_index)
		{
			this->Memory.resize_until_capacity_met(this->Size + p_elements.Size);
			this->move_memory_down(p_index, p_elements.Size);
			this->Memory.copy_memory(p_index, p_elements);

			this->Size += p_elements.Size;

			return 1;
		};


#define ShadowVector(ElementType) ElementType##_##ShadowVector
#define ShadowVector_t(ElementType, Prefix) ElementType##_##ShadowVector##_##Prefix

#define sv_get_size() get_size()
#define sv_get(p_index) get(p_index)
#define sv_erase_element_at(p_index) erase_element_at(p_index)
#define sv_push_back_element(p_element) push_back_element(p_element)
#define sv_to_slice() to_slice()

	};

}


#endif

#ifdef COMMON2_POOL


//TODO -> delete when common2 migration is complete
namespace v2
{
	template<class ElementType>
	using PoolMemory_t = Vector<ElementType>;

	template<class ElementType>
	using PoolFreeBlocks_t = Vector<Token(ElementType)>;

	/*
		A Pool is a non continous Vector where elements are acessed via Tokens.
		Generated Tokens are unique from it's source Pool.
		Even if pool memory is reallocate, generated Tokens are still valid.
		/!\ It is very unsafe to store raw pointer of an element. Because is Pool memory is reallocated, then the pointer is no longer valid.
	*/
	template<class ElementType>
	struct Pool
	{
		PoolMemory_t<ElementType> memory;
		PoolFreeBlocks_t<ElementType> free_blocks;

		inline static Pool<ElementType> build(const PoolMemory_t<ElementType> p_memory, const PoolFreeBlocks_t<ElementType> p_free_blocks)
		{
			return Pool<ElementType>{p_memory, p_free_blocks};
		};

		inline static Pool<ElementType> allocate(const uimax p_memory_capacity)
		{
			return Pool<ElementType>{ Vector<ElementType>::allocate(p_memory_capacity), Vector<Token(ElementType)>::build(cast(Token(ElementType)*, NULL), 0) };
		};


		inline void free()
		{
			this->memory.free();
			this->free_blocks.free();
		};

		inline uimax get_size()
		{
			return this->memory.Size;
		};

		inline uimax get_capacity()
		{
			return this->memory.Memory.Capacity;
		};

		inline uimax get_free_size()
		{
			return this->free_blocks.Size;
		};

		inline ElementType* get_memory()
		{
			return this->memory.Memory.Memory;
		};

		inline int8 has_allocated_elements()
		{
			return this->memory.Size != this->free_blocks.Size;
		};

		inline int8 is_element_free(const Token(ElementType) p_token)
		{
			for (vector_loop(&this->free_blocks, i))
			{
				if (tk_v(this->free_blocks.get(i)) == tk_v(p_token))
				{
					return 1;
				};
			};

			return 0;
		};

		inline ElementType& get(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			this->element_free_check(p_token);
#endif

			return this->memory.get(tk_v(p_token));
		};

		inline Token(ElementType) alloc_element_empty()
		{
			if (!this->free_blocks.empty())
			{
				Token(ElementType) l_availble_token = this->free_blocks.get(this->free_blocks.Size - 1);
				this->free_blocks.pop_back();
				return l_availble_token;
			}
			else
			{
				this->memory.push_back_element_empty();
				return Token(ElementType) { this->memory.Size - 1 };
			}
		}

		inline Token(ElementType) alloc_element(const ElementType& p_element)
		{
			if (!this->free_blocks.empty())
			{
				Token(ElementType) l_availble_token = this->free_blocks.get(this->free_blocks.Size - 1);
				this->free_blocks.pop_back();
				this->memory.get(tk_v(l_availble_token)) = p_element;
				return l_availble_token;
			}
			else
			{
				this->memory.push_back_element(p_element);
				return Token(ElementType) { this->memory.Size - 1 };
			}
		};


		inline void release_element(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			this->element_not_free_check(p_token);
#endif

			this->free_blocks.push_back_element(p_token);
		};

	private:

		inline void element_free_check(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_element_free(p_token))
			{
				abort();
			}
#endif
		};

		inline void element_not_free_check(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_element_free(p_token))
			{
				abort();
			}
#endif
		};
	};


}


#endif

#ifdef COMMON2_STRING


namespace v2
{
	/*
		A String is a vector of int8 that always have a NULL int8acter at it's last element.
	*/
	struct String
	{
		Vector<int8> Memory;

		inline static String allocate(const uimax p_initial_capacity)
		{
			int8 l_null_int8 = (int8)NULL;
			return String{ Vector<int8>::allocate_capacity_elements(p_initial_capacity + 1, Slice<int8>::build_asint8_memory_elementnb(&l_null_int8, 1)) };
		};

		inline static String allocate_elements(const Slice<int8>& p_initial_elements)
		{
			String l_string = String{ Vector<int8>::allocate_capacity_elements(p_initial_elements.Size + 1, p_initial_elements) };
			l_string.Memory.push_back_element((int8)NULL);
			return l_string;
		};

		inline void free()
		{
			this->Memory.free();
		};

		inline void append(const Slice<int8>& p_elements)
		{
			this->Memory.insert_array_at(p_elements, this->Memory.Size - 1);
		};

		inline void insert_array_at(const Slice<int8>& p_elements, const uimax p_index)
		{
			// The insert_array_at will fail if p_index == this->get_size();
			this->Memory.insert_array_at(p_elements, p_index);
		};

		inline void erase_array_at(const uimax p_index, const uimax p_size)
		{
#if CONTAINER_BOUND_TEST
			if ((p_index + p_size) == this->get_size() - 1)
			{
				abort();
			}
#endif
			this->Memory.erase_array_at(p_index, p_size);
		};


		inline int8& get(const uimax p_index)
		{
			return this->Memory.get(p_index);
		};

		inline const int8& get(const uimax p_index) const
		{
			return ((String*)this)->Memory.get(p_index);
		};

		inline int8* get_memory()
		{
			return this->Memory.get_memory();
		};

		inline uimax get_size() const
		{
			return this->Memory.Size;
		};

		inline uimax get_int8_nb() const
		{
			return this->Memory.Size - 1;
		};

		inline void clear()
		{
			this->Memory.clear();
			this->Memory.push_back_element((int8)NULL);
		};

		inline Slice<int8> to_slice()
		{
			return Slice<int8>::build_memory_elementnb(this->Memory.Memory.Memory, this->Memory.Size - 1);
		};

		inline void remove_int8s(const int8 p_int8)
		{
			for (vector_loop_reverse(&this->Memory, i))
			{
				if (this->Memory.get(i) == p_int8)
				{
					this->Memory.erase_element_at(i);
				}
			}
		}
	};
}

#endif

#ifdef COMMON2_VARYINGVECTOR


//TODO -> delete when common2 migration is complete
namespace v2
{
	/*
		A VaryingVector is a Vector which elements can have different sizes.
		Elements are accessed vie the Chunks_t lookup table.
		Memory is continuous.
	*/
	struct VaryingVector
	{
		using Memory_t = Vector<int8>;
		using Chunks_t = Vector<SliceIndex>;

		Memory_t memory;
		Chunks_t chunks;

		inline static  VaryingVector build(const Memory_t& p_memory, const Chunks_t& p_chunks)
		{
			return VaryingVector{ p_memory, p_chunks };
		};

		inline static VaryingVector allocate(const uimax p_memory_array_initial_capacity, const uimax p_chunk_array_initial_capacity)
		{
			return build(Memory_t::allocate(p_memory_array_initial_capacity), Chunks_t::allocate(p_chunk_array_initial_capacity));
		};

		inline static VaryingVector allocate_default()
		{
			return allocate(0, 0);
		};


		inline void free()
		{
			this->memory.free();
			this->chunks.free();
		};

		inline uimax get_size()
		{
			return this->chunks.Size;
		};

		inline void push_back(const Slice<int8>& p_bytes)
		{
			SliceIndex l_chunk = SliceIndex::build(this->memory.Size, p_bytes.Size);
			this->memory.push_back_array(p_bytes);
			this->chunks.push_back_element(l_chunk);
		};

		inline void push_back_empty(const uimax p_slice_size)
		{
			SliceIndex l_chunk = SliceIndex::build(this->memory.Size, p_slice_size);
			this->memory.push_back_array_empty(p_slice_size);
			this->chunks.push_back_element(l_chunk);
		};

		template<class ElementType>
		inline void push_back_element(const ElementType& p_element)
		{
			this->push_back(Slice<int8>::build_memory_elementnb(cast(int8*, &p_element), sizeof(ElementType)));
		};

		inline void pop_back()
		{
			this->memory.pop_back_array(
				this->chunks.get(this->chunks.Size - 1).Size
			);
			this->chunks.pop_back();
		};

		inline void insert_at(const Slice<int8>& p_bytes, const uimax p_index)
		{
			SliceIndex& l_break_chunk = this->chunks.get(p_index);
			this->memory.insert_array_at(p_bytes, l_break_chunk.Begin);
			this->chunks.insert_element_at(SliceIndex::build(l_break_chunk.Begin, p_bytes.Size), p_index);

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin += p_bytes.Size;
			}
		};

		inline void erase_element_at(const uimax p_index)
		{

			SliceIndex& l_chunk = this->chunks.get(p_index);
			this->memory.erase_array_at(
				l_chunk.Begin,
				l_chunk.Size
			);

			for (loop(i, p_index, this->chunks.Size))
			{
				this->chunks.get(i).Begin -= l_chunk.Size;
			};

			this->chunks.erase_element_at(p_index);
		};

		//TODO add test :-)
		inline void erase_element_at_always(const uimax p_index)
		{
			if (p_index == this->get_size())
			{
				this->pop_back();
			}
			else
			{
				this->erase_element_at(p_index);
			}
		};

		inline void erase_array_at(const uimax p_index, const uimax p_element_nb)
		{
			SliceIndex l_removed_chunk = SliceIndex::build_default();

			for (loop(i, p_index, p_index + p_element_nb))
			{
				l_removed_chunk.Size += this->chunks.get(i).Size;
			};

			SliceIndex& l_first_chunk = this->chunks.get(p_index);
			l_removed_chunk.Begin = l_first_chunk.Begin;

			this->memory.erase_array_at(
				l_removed_chunk.Begin,
				l_removed_chunk.Size
			);

			for (loop(i, p_index + p_element_nb, this->chunks.Size))
			{
				this->chunks.get(i).Begin -= l_removed_chunk.Size;
			};

			this->chunks.erase_array_at(p_index, p_element_nb);
		};



		inline void element_expand(const uimax p_index, const uimax p_expansion_size)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_expansion_size != 0);
#endif

			uimax l_new_varyingvector_size = this->memory.Size + p_expansion_size;

			this->memory.Memory.resize_until_capacity_met(l_new_varyingvector_size);
			l_updated_chunk.Size += p_expansion_size;

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin += p_expansion_size;
			}
		};

		inline void element_expand_with_value(const uimax p_index, const Slice<int8>& p_pushed_element)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_pushed_element.Size != 0);
#endif

			uimax l_size_delta = p_pushed_element.Size;
			uimax l_new_varyingvector_size = this->memory.Size + l_size_delta;

			this->memory.Memory.resize_until_capacity_met(l_new_varyingvector_size);

			this->memory.insert_array_at_always(p_pushed_element, l_updated_chunk.Begin + l_updated_chunk.Size);
			l_updated_chunk.Size += l_size_delta;

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin += l_size_delta;
			}
		};

		inline void element_shrink(const uimax p_index, const uimax p_size_delta)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_size_delta != 0);
			assert_true(p_size_delta <= l_updated_chunk.Size);
#endif

			this->memory.erase_array_at(l_updated_chunk.Begin + l_updated_chunk.Size - p_size_delta, p_size_delta);
			l_updated_chunk.Size -= p_size_delta;

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin -= p_size_delta;
			}
		};

		inline void element_writeto(const uimax p_index, const uimax p_insertion_offset, const Slice<int8>& p_inserted_element)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);
			Slice<int8> l_updated_chunk_slice = Slice<int8>::build_asint8_memory_elementnb(this->memory.get_memory() + l_updated_chunk.Begin, l_updated_chunk.Size).slide_rv(p_insertion_offset);

			slice_memcpy(l_updated_chunk_slice, p_inserted_element);
		};

		inline void element_movememory(const uimax p_index, const uimax p_insertion_offset, const Slice<int8>& p_inserted_element)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);
			Slice<int8> l_updated_chunk_slice = Slice<int8>::build_asint8_memory_elementnb(this->memory.get_memory() + l_updated_chunk.Begin, l_updated_chunk.Size).slide_rv(p_insertion_offset);

			slice_memmove(l_updated_chunk_slice, p_inserted_element);
		};


		inline Slice<int8> get_element(const uimax p_index)
		{
			SliceIndex& l_chunk = this->chunks.get(p_index);
			return Slice<int8>::build_memory_offset_elementnb(
				this->memory.get_memory(),
				l_chunk.Begin,
				l_chunk.Size
			);
		};

		inline Slice<int8> get_last_element()
		{
			return this->get_element(this->get_size() - 1);
		};

		template<class ElementType>
		inline ElementType* get_element_typed(const uimax p_index)
		{
			return  slice_cast_singleelement<ElementType>(this->get_element(p_index));
		};

	};
}



#endif

#ifdef COMMON2_POOLINDEXED

//TODO delete when common2 migration is complete
namespace v2
{
	template<class ElementType>
	struct PoolIndexed
	{
		Pool<ElementType> Memory;
		Vector<Token(ElementType)> Indices;

		inline static PoolIndexed<ElementType> allocate_default()
		{
			return PoolIndexed<ElementType>
			{
				Pool<ElementType>::allocate(0),
					Vector<Token(ElementType)>::allocate(0)
			};
		};

		inline void free()
		{
			this->Memory.free();
			this->Indices.free();
		};

		inline int8 has_allocated_elements()
		{
			return this->Memory.has_allocated_elements();
		};

		inline Token(ElementType) alloc_element(const ElementType& p_element)
		{
			Token(ElementType) l_token = this->Memory.alloc_element(p_element);
			this->Indices.push_back_element(l_token);
			return l_token;
		};

		inline void release_element(const Token(ElementType) p_element)
		{
			this->Memory.release_element(p_element);
			for (vector_loop(&this->Indices, i))
			{
				if (tk_eq(this->Indices.get(i), p_element))
				{
					this->Indices.erase_element_at(i);
					break;
				}
			};
		};

		inline ElementType& get(const Token(ElementType) p_element)
		{
			return this->Memory.get(p_element);
		};

	};
}

#define poolindexed_foreach_token_2_begin(PoolIndexedVariable, IteratorName, TokenVariableName) \
for (vector_loop((&(PoolIndexedVariable)->Indices), IteratorName)) \
{ \
auto& TokenVariableName = (PoolIndexedVariable)->Indices.get(IteratorName);

#define poolindexed_foreach_token_2_end() \
}


#define poolindexed_foreach_value_begin(PoolIndexedVariable, IteratorName, TokenVariableName, ValueVariableName) \
for (vector_loop(&(PoolIndexedVariable)->Indices, IteratorName)) \
{\
auto& TokenVariableName = (PoolIndexedVariable)->Indices.get(IteratorName); \
auto& ValueVariableName = (PoolIndexedVariable)->Memory.get(TokenVariableName);

#define poolindexed_foreach_value_end() \
}

#endif

#ifdef COMMON2_VECTOROFVECTOR

//TODO delete when common2 migration is complete
namespace v2
{
	/*
		//TODO

		functions missing :
			- insert array version of vectorofvector_element_insert_element_at and vectorofvector_element_erase_element_at
	*/
	/*
		The header of every vector of the VectorOfVector. That indicates the associated memory chunk state.
	*/
	struct VectorOfVector_VectorHeader
	{
		uimax Size;
		uimax Capacity;

		inline static VectorOfVector_VectorHeader build(const uimax p_size, const uimax p_capacity)
		{
			return VectorOfVector_VectorHeader{ p_size, p_capacity };
		};

		inline static VectorOfVector_VectorHeader build_default()
		{
			return build(0, 0);
		};

		inline static Span<int8> allocate(const Slice<int8>& p_vector_slice, const uimax p_vector_size)
		{
			Span<int8> l_allocated_element = Span<int8>::allocate(sizeof(VectorOfVector_VectorHeader) + p_vector_slice.Size);
			VectorOfVector_VectorHeader l_vector_header = build(p_vector_size, p_vector_size);
			l_allocated_element.copy_memory(0, Slice<VectorOfVector_VectorHeader>::build_asint8_memory_elementnb(&l_vector_header, 1));
			l_allocated_element.copy_memory(sizeof(l_vector_header), p_vector_slice);
			return l_allocated_element;
		};

		inline static Span<int8> allocate_0v(const Slice<int8>& p_vector_slice, const uimax p_vector_size)
		{
			return allocate(p_vector_slice, p_vector_size);
		};

		template<class ElementType>
		inline static Span<int8> allocate_vectorelements(const Slice<ElementType>& p_vector_elements)
		{
			return allocate_0v(p_vector_elements.build_asint8(), p_vector_elements.Size);
		};

		inline static uimax get_vector_offset()
		{
			return sizeof(VectorOfVector_VectorHeader);
		};

		inline static uimax get_vector_element_offset(const uimax p_element_index, const uimax p_element_size)
		{
			return get_vector_offset() + (p_element_index * p_element_size);
		};

		template<class ElementType>
		inline Slice<ElementType> get_vector_to_capacity() const
		{
			return Slice<ElementType>::build_memory_elementnb(cast(ElementType*, cast(int8*, this) + sizeof(VectorOfVector_VectorHeader)), this->Capacity);
		};
	};

	/*
		A VectorOfVector is a chain of resizable Vector allocated on the same memory block.
		Every nested vectors can be altered with "vectorofvector_element_*" functions.
	*/
	template<class ElementType>
	struct VectorOfVector
	{
		VaryingVector varying_vector;

		inline static VectorOfVector<ElementType> allocate_default()
		{
			return VectorOfVector<ElementType>{VaryingVector::allocate_default()};
		};

		inline void free()
		{
			this->varying_vector.free();
		};

		inline void push_back()
		{
			VectorOfVector_VectorHeader l_header = VectorOfVector_VectorHeader::build_default();
			Slice<int8> l_header_slice = Slice<VectorOfVector_VectorHeader>::build_asint8_memory_elementnb(&l_header, 1);
			this->varying_vector.push_back(l_header_slice);
		};

		inline void push_back_element(const Slice<ElementType>& p_vector_elements)
		{
			Span<int8> l_pushed_memory = VectorOfVector_VectorHeader::allocate_vectorelements(p_vector_elements);
			this->varying_vector.push_back(l_pushed_memory.slice);
			l_pushed_memory.free();
		};

		//TODO -> add tests :-)
		inline void insert_empty_at(const uimax p_index)
		{
			this->varying_vector.insert_at(Slice<int8>::build(NULL, 0, 1), p_index);
		};

		inline void erase_element_at(const uimax p_index)
		{
			this->varying_vector.erase_element_at(p_index);
		};

		//TODO -> add tests :-)
		inline void erase_element_at_always(const uimax p_index)
		{
			this->varying_vector.erase_element_at_always(p_index);
		};

		inline Slice<ElementType> get(const uimax p_index)
		{
			Slice<int8> l_element = this->varying_vector.get_element(p_index);
			VectorOfVector_VectorHeader* l_header = cast(VectorOfVector_VectorHeader*, l_element.Begin);
			return Slice<ElementType>::build_memory_elementnb(
				cast(ElementType*, l_element.slide_rv(VectorOfVector_VectorHeader::get_vector_offset()).Begin),
				l_header->Size
			);
		};

		inline VectorOfVector_VectorHeader* get_vectorheader(const uimax p_index)
		{
			return cast(VectorOfVector_VectorHeader*, this->varying_vector.get_element(p_index).Begin);
		};

		inline void element_push_back_element(const uimax p_nested_vector_index, const ElementType& p_element)
		{
			VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

			if (l_vector_header->Size + 1 > l_vector_header->Capacity)
			{
				this->varying_vector.element_expand_with_value(
					p_nested_vector_index,
					Slice<ElementType>::build_asint8_memory_elementnb(&p_element, 1)
				);

				// /!\ because we potentially reallocate the p_vector_of_vector, we nee to requery for the VectorOfVector_VectorHeader
				l_vector_header = this->get_vectorheader(p_nested_vector_index);
				l_vector_header->Capacity += 1;
			}
			else
			{
				this->element_write_element(p_nested_vector_index, l_vector_header->Size, p_element);
			}

			l_vector_header->Size += 1;
		};


		inline void element_push_back_array(const uimax p_nested_vector_index, const Slice<ElementType>& p_elements)
		{
			VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

			if (l_vector_header->Size == l_vector_header->Capacity)
			{
				this->varying_vector.element_expand_with_value(
					p_nested_vector_index,
					p_elements.build_asint8()
				);

				l_vector_header = this->get_vectorheader(p_nested_vector_index);
				l_vector_header->Size += p_elements.Size;
				l_vector_header->Capacity += p_elements.Size;
			}
			else if (l_vector_header->Size + p_elements.Size > l_vector_header->Capacity)
			{
				uimax l_write_element_nb = l_vector_header->Capacity - l_vector_header->Size;
				uimax l_expand_element_nb = p_elements.Size - l_write_element_nb;

				this->element_write_array(p_nested_vector_index, l_vector_header->Size, Slice<ElementType>::build_memory_elementnb(p_elements.Begin, l_write_element_nb));

				this->varying_vector.element_expand_with_value(
					p_nested_vector_index,
					Slice<ElementType>::build_asint8_memory_elementnb(p_elements.Begin + l_write_element_nb, l_expand_element_nb)
				);

				l_vector_header = this->get_vectorheader(p_nested_vector_index);
				l_vector_header->Size += p_elements.Size;
				l_vector_header->Capacity += l_expand_element_nb;
			}
			else
			{
				this->element_write_array(p_nested_vector_index, l_vector_header->Size, p_elements);

				l_vector_header->Size += p_elements.Size;
			}


		};

		inline void element_insert_element_at(const uimax p_nested_vector_index, const uimax p_index, const ElementType& p_element)
		{
			VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_index != l_vector_header->Size); //use vectorofvector_element_push_back_element
			assert_true(p_index < l_vector_header->Size);
#endif

			if ((l_vector_header->Size + 1) > l_vector_header->Capacity)
			{
				this->element_movememory_down_and_resize(p_nested_vector_index, *l_vector_header, p_index, 1);
				l_vector_header = this->get_vectorheader(p_nested_vector_index);
			}
			else
			{
				this->element_movememory_down(p_nested_vector_index, *l_vector_header, p_index, 1);
			}

			l_vector_header->Size += 1;

			this->element_write_element(p_nested_vector_index, p_index, p_element);
		}

		inline void element_erase_element_at(const uimax p_nested_vector_index, const uimax p_index)
		{
			VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

#if CONTAINER_BOUND_TEST
			if (p_index == l_vector_header->Size) { abort(); } //use vectorofvector_element_pop_back_element
			if (p_index > l_vector_header->Size) { abort(); }
#endif
			this->element_erase_element_at_unchecked(p_nested_vector_index, p_index, l_vector_header);
		};

		inline void element_pop_back_element(const uimax p_nested_vector_index, const uimax p_index)
		{
			VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);
#if CONTAINER_BOUND_TEST
			if (p_index != (l_vector_header->Size - 1))
			{
				abort(); //use element_erase_element_at
			}
#endif
			this->element_pop_back_element_unchecked(l_vector_header);
		};

		inline void element_erase_element_at_always(const uimax p_nested_vector_index, const uimax p_index)
		{
			VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);
#if CONTAINER_BOUND_TEST
			if (p_index >= l_vector_header->Size) { abort(); }
#endif
			if (p_index < l_vector_header->Size - 1)
			{
				this->element_erase_element_at_unchecked(p_nested_vector_index, p_index, l_vector_header);
			}
			else
			{
				this->element_pop_back_element_unchecked(l_vector_header);
			}

		};

		inline void element_clear(const uimax p_nested_vector_index)
		{
			this->get_vectorheader(p_nested_vector_index)->Size = 0;
		};


		/*
			Element_ShadowVector is a wrapper around a VectorOfVector that acts like a regular Vector.
			It can be used in some templated algorithm that uses Vector.
		*/
		struct Element_ShadowVector
		{
			VectorOfVector<ElementType>* vectorOfVector;
			uimax Index;

			inline static Element_ShadowVector build(VectorOfVector<ElementType>* p_vector_of_vector, const uimax p_index)
			{
				return Element_ShadowVector{
					p_vector_of_vector, p_index
				};
			};

			inline uimax sv_get_size()
			{
				return this->vectorOfVector->get_vectorheader(this->Index)->Size;
			};

			inline ElementType& sv_get(const uimax p_index)
			{
				return this->vectorOfVector->get(this->Index).get(p_index);
			};

			inline void sv_erase_element_at(const uimax p_index)
			{
				this->vectorOfVector->element_erase_element_at(this->Index, p_index);
			};

			inline void sv_push_back_element(const ElementType& p_index)
			{
				this->vectorOfVector->element_push_back_element(this->Index, p_index);
			};

			inline Slice<ElementType> sv_to_slice()
			{
				return this->vectorOfVector->get(this->Index);
			};
		};

		inline Element_ShadowVector element_as_shadow_vector(const uimax p_nested_vector_index)
		{
			return Element_ShadowVector::build(this, p_nested_vector_index);
		};

	private:
		inline void element_movememory_up(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header,
			const uimax p_break_index, const uimax p_move_delta)
		{
			Slice<int8> l_source =
				p_nested_vector_header.get_vector_to_capacity<ElementType>()
				.slide_rv(p_break_index)
				.build_asint8();

			this->varying_vector.element_movememory(
				p_nested_vector_index,
				VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index - p_move_delta, sizeof(ElementType)),
				l_source
			);
		};

		inline void element_movememory_down(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header,
			const uimax p_break_index, const uimax p_move_delta)
		{
			Slice<int8> l_source =
				p_nested_vector_header.get_vector_to_capacity<ElementType>()
				.slide_rv(p_break_index)
				.build_asint8();

			this->varying_vector.element_movememory(
				p_nested_vector_index,
				VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)),
				l_source
			);
		};

		inline void element_movememory_down_and_resize(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header,
			const uimax p_break_index, const uimax p_move_delta)
		{
			Slice<int8> l_source =
				p_nested_vector_header.get_vector_to_capacity<ElementType>()
				.slide_rv(p_break_index)
				.build_asint8();

			this->varying_vector.element_expand(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)) + l_source.Size);

			this->varying_vector.element_movememory(
				p_nested_vector_index,
				VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)),
				l_source
			);
		};

		inline void element_write_element(const uimax p_nested_vector_index, const uimax p_write_start_index, const ElementType& p_element)
		{
			this->varying_vector.element_writeto(
				p_nested_vector_index,
				VectorOfVector_VectorHeader::get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
				Slice<ElementType>::build_asint8_memory_singleelement(&p_element)
			);
		};

		inline void element_write_array(const uimax p_nested_vector_index, const uimax p_write_start_index, const Slice<ElementType>& p_elements)
		{
			this->varying_vector.element_writeto(
				p_nested_vector_index,
				VectorOfVector_VectorHeader::get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
				Slice<ElementType>::build_asint8_memory_elementnb(p_elements.Begin, p_elements.Size)
			);
		};

		inline void element_erase_element_at_unchecked(const uimax p_nested_vector_index, const uimax p_index, VectorOfVector_VectorHeader* p_vector_header)
		{
			this->element_movememory_up(p_nested_vector_index, *p_vector_header, p_index + 1, 1);
			p_vector_header->Size -= 1;
		};

		inline void element_pop_back_element_unchecked(VectorOfVector_VectorHeader* p_vector_header)
		{
			p_vector_header->Size -= 1;
		};




	};

}


#endif

#ifdef COMMON2_POOLOFVECTOR

//TODO delete when common2 migration is complete
namespace v2
{
	template<class ElementType>
	using PoolOfVectorMemory_t = VectorOfVector<ElementType>;

	template<class ElementType>
	using PoolOfVectorToken = Token(Slice<ElementType>);

	template<class ElementType>
	using PoolOfVectorFreeBlocks_t = Vector<PoolOfVectorToken<ElementType>>;

	/*
		A PoolOfVector is a wrapped VectorOfVector with pool allocation logic.
		Any operation on nested vectors must be called with the (poolofvector_element_*) functions
	*/
	template<class ElementType>
	struct PoolOfVector
	{
		PoolOfVectorMemory_t<ElementType> Memory;
		PoolOfVectorFreeBlocks_t<ElementType> FreeBlocks;


		inline static PoolOfVector<ElementType> allocate_default()
		{
			return PoolOfVector<ElementType>
			{
				VectorOfVector<ElementType>::allocate_default(),
					Vector<PoolOfVectorToken<ElementType>>::allocate(0)
			};
		};

		inline void free()
		{
			this->Memory.free();
			this->FreeBlocks.free();
		};


		inline int8 is_token_free(const PoolOfVectorToken<ElementType> p_token)
		{
			for (vector_loop(&this->FreeBlocks, i))
			{
				if (tk_eq(this->FreeBlocks.get(i), p_token))
				{
					return 1;
				}
			}
			return 0;
		};

		inline int8 has_allocated_elements()
		{
			return this->Memory.varying_vector.get_size() != this->FreeBlocks.Size;
		}

		inline PoolOfVectorToken<ElementType> alloc_vector_with_values(const Slice<ElementType>& p_initial_elements)
		{
			if (!this->FreeBlocks.empty())
			{
				PoolOfVectorToken<ElementType> l_token = this->FreeBlocks.get(this->FreeBlocks.Size - 1);
				this->FreeBlocks.pop_back();
				this->Memory.element_push_back_array(tk_v(l_token), p_initial_elements);
				return l_token;
			}
			else
			{
				this->Memory.push_back_element(p_initial_elements);
				return PoolOfVectorToken<ElementType>{ this->Memory.varying_vector.get_size() - 1 };
			}
		};

		inline PoolOfVectorToken<ElementType> alloc_vector()
		{
			if (!this->FreeBlocks.empty())
			{
				PoolOfVectorToken<ElementType> l_token = this->FreeBlocks.get(this->FreeBlocks.Size - 1);
				this->FreeBlocks.pop_back();
				return l_token;
			}
			else
			{
				this->Memory.push_back();
				return PoolOfVectorToken<ElementType>{ this->Memory.varying_vector.get_size() - 1 };
			}
		};

		inline void release_vector(const PoolOfVectorToken<ElementType> p_token)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif

			this->Memory.element_clear(tk_v(p_token));
			this->FreeBlocks.push_back_element(p_token);
		};

		inline Slice<ElementType> get_vector(const PoolOfVectorToken<ElementType> p_token)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif

			return this->Memory.get(tk_v(p_token));
		};

		inline void element_push_back_element(const PoolOfVectorToken<ElementType> p_token, const ElementType& p_element)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif

			this->Memory.element_push_back_element(tk_v(p_token), p_element);
		};

		inline void element_erase_element_at(const PoolOfVectorToken<ElementType> p_token, const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif
			this->Memory.element_erase_element_at(tk_v(p_token), p_index);
		};

		inline void element_erase_element_at_always(const PoolOfVectorToken<ElementType> p_token, const uimax p_index)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif
			this->Memory.element_erase_element_at_always(tk_v(p_token), p_index);
		};

		inline void element_clear(const PoolOfVectorToken<ElementType> p_token)
		{

#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif
			this->Memory.element_clear(tk_v(p_token));
		};


		//TODO -> write test
		struct Element_ShadowVector
		{
			PoolOfVector<ElementType>* pool_of_vector;
			PoolOfVectorToken<ElementType> index;

			inline static Element_ShadowVector build(PoolOfVector<ElementType>* p_pool_of_vector, const PoolOfVectorToken<ElementType> p_index)
			{
				return Element_ShadowVector{ p_pool_of_vector, p_index };
			};

			inline uimax sv_get_size()
			{
				return this->pool_of_vector->get_vector(this->index).Size;
			};

			inline ElementType& sv_get(const uimax p_index)
			{
				return this->pool_of_vector->get_vector(this->index).get(p_index);
			};

			inline void sv_push_back_element(const ElementType& p_element)
			{
				this->pool_of_vector->element_push_back_element(this->index, p_element);
			};
		};


		inline Element_ShadowVector get_element_as_shadow_vector(const PoolOfVectorToken<ElementType> p_index)
		{
			return Element_ShadowVector::build(this, p_index);
		};


	private:
		inline void token_not_free_check(const PoolOfVectorToken<ElementType> p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_token_free(p_token))
			{
				abort();
			}
#endif
		};

	};

}


#endif

#ifdef COMMON2_HEAP



#define ShadowHeap_t(Prefix) ShadowHeap_##Prefix
#define sh_get_size() get_size()
#define sh_resize(p_newsize) resize(p_newsize)
#define sh_get_freechunks() get_freechunks()
#define sh_get_allocated_chunks() get_allocated_chunks()

namespace v2
{


	struct HeapA
	{
		enum class AllocationState
		{
			NOT_ALLOCATED = 1,
			ALLOCATED = 2,
			HEAP_RESIZED = 4,
			ALLOCATED_AND_HEAP_RESIZED = ALLOCATED | HEAP_RESIZED
		};
		using AllocationState_t = uint8;

		struct AllocatedElementReturn
		{
			Token(SliceIndex) token;
			uimax Offset;

			inline static AllocatedElementReturn build(const Token(SliceIndex) p_token, const uimax p_offset)
			{
				return AllocatedElementReturn{ p_token, p_offset };
			};
		};

		template<class ShadowHeap_t(s)>
		static AllocationState allocate_element(ShadowHeap_t(s)& p_heap,
			const uimax p_size, AllocatedElementReturn* out_chunk);

		template<class ShadowHeap_t(s)>
		static AllocationState allocate_element_with_modulo_offset(ShadowHeap_t(s)& p_heap,
			const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);

		template<class ShadowHeap_t(s)>
		static AllocationState allocate_element_norealloc_with_modulo_offset(ShadowHeap_t(s)& p_heap,
			const uimax p_size, const uimax p_alignement_modulo, HeapA::AllocatedElementReturn* out_chunk);

	private:
		template<class ShadowHeap_t(s)>
		static int8 _allocate_element(ShadowHeap_t(s)& p_heap, const uimax p_size, HeapA::AllocatedElementReturn* out_return);

		template<class ShadowHeap_t(s)>
		static int8 _allocate_element_with_modulo_offset(ShadowHeap_t(s)& p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);

		template<class ShadowHeap_t(s)>
		static AllocatedElementReturn _push_chunk(ShadowHeap_t(s)& p_heap, SliceIndex* p_chunk);

		template<class ShadowHeap_t(s)>
		static void _defragment(ShadowHeap_t(s)& p_heap);
	};


	/*
		A Heap is a function object that calculates : "what is the offset of the chunk of data you want to allocate ?".
		The Heap object doesn't actually allocate any memory. It is up to the consumer to choose how memory is allocated.
		The heap has been implemented like that to allow flexibility between allocating memory on the CPU or GPU. (or any other logic)
	*/
	struct Heap
	{
		Pool<SliceIndex> AllocatedChunks;
		Vector<SliceIndex> FreeChunks;
		uimax Size;

		static Heap allocate(const uimax p_heap_size);
		void free();
		void sh_resize(const uimax p_newsize);
		uimax sh_get_size();
		Vector<SliceIndex>& sh_get_freechunks();
		Pool<SliceIndex>& sh_get_allocated_chunks();
		HeapA::AllocationState allocate_element(const uimax p_size, HeapA::AllocatedElementReturn* out_chunk);
		HeapA::AllocationState allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);
		HeapA::AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);
		SliceIndex* get(const Token(SliceIndex) p_chunk);
		void release_element(const Token(SliceIndex) p_chunk);
		HeapA::AllocationState reallocate_element(const Token(SliceIndex) p_chunk, const uimax p_new_size, HeapA::AllocatedElementReturn* out_chunk);
	};


	struct HeapPagedToken
	{
		uimax PageIndex;
		Token(SliceIndex) token;
	};

	/*
		A HeapPaged is a function object that calculates : "what is the offset of the chunk of data you want to allocate ?".
		It has the same behavior of the Heap. But, allocated chunks are never deallocated.
		Instead, when allocation is not possible with the current chunks, it allocates another chunk.
	*/
	struct HeapPaged
	{
		typedef uint8 AllocationState_t;
		enum class AllocationState
		{
			NOT_ALLOCATED = 0,
			ALLOCATED = 1,
			PAGE_CREATED = 2,
			ALLOCATED_AND_PAGE_CREATED = ALLOCATED | PAGE_CREATED
		};

		struct AllocatedElementReturn
		{
			HeapPagedToken token;
			uimax Offset;

			inline static AllocatedElementReturn buid_from_HeapAllocatedElementReturn(const uimax p_page_index,
				const HeapA::AllocatedElementReturn& p_heap_allocated_element_return)
			{
				return AllocatedElementReturn{ HeapPagedToken{p_page_index, p_heap_allocated_element_return.token}, p_heap_allocated_element_return.Offset };
			};
		};

		Pool<SliceIndex> AllocatedChunks;
		VectorOfVector<SliceIndex> FreeChunks;
		uimax PageSize;

		/*
			A ShadowHeap_t that wraps a sigle Heap of the HeapPaged to use it on Heap generic algorithms.
		*/
		struct SingleShadowHeap
		{
			HeapPaged* heapPaged;
			uimax index;
			VectorOfVector<SliceIndex>::Element_ShadowVector tmp_shadow_vec;

			inline static SingleShadowHeap build(HeapPaged* p_heap_paged, const uimax p_index)
			{
				SingleShadowHeap l_shadow_heap;
				l_shadow_heap.heapPaged = p_heap_paged;
				l_shadow_heap.index = p_index;
				return l_shadow_heap;
			};

			inline VectorOfVector<SliceIndex>::Element_ShadowVector& sh_get_freechunks()
			{
				this->tmp_shadow_vec = this->heapPaged->FreeChunks.element_as_shadow_vector(this->index);
				return tmp_shadow_vec;
			};

			inline Pool<SliceIndex>& sh_get_allocated_chunks()
			{
				return this->heapPaged->AllocatedChunks;
			};
		};

		static HeapPaged allocate_default(const uimax p_page_size);
		void free();
		uimax get_page_count();
		AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk);
		void release_element(const HeapPagedToken& p_token);
		SliceIndex* get_sliceindex_only(const HeapPagedToken& p_token);

	private:
		void create_new_page();
	};

}

namespace v2
{


	inline Heap Heap::allocate(const uimax p_heap_size)
	{
		Heap l_heap = Heap{
			Pool<SliceIndex>::allocate(0),
			Vector<SliceIndex>::allocate(1),
			p_heap_size };
		l_heap.FreeChunks.push_back_element(SliceIndex::build(0, p_heap_size));
		return l_heap;
	};

	inline void Heap::free()
	{
		this->AllocatedChunks.free();
		this->FreeChunks.free();
		this->Size = 0;
	};

	inline void Heap::sh_resize(const uimax p_newsize)
	{
		uimax l_old_size = this->Size;
		this->FreeChunks.push_back_element(SliceIndex::build(l_old_size, p_newsize - l_old_size));
		this->Size = p_newsize;
	};


	inline uimax Heap::sh_get_size()
	{
		return this->Size;
	};

	inline Vector<SliceIndex>& Heap::sh_get_freechunks()
	{
		return this->FreeChunks;
	};

	inline	Pool<SliceIndex>& Heap::sh_get_allocated_chunks()
	{
		return this->AllocatedChunks;
	};

	inline HeapA::AllocationState Heap::allocate_element(const uimax p_size, HeapA::AllocatedElementReturn* out_chunk)
	{
		return HeapA::allocate_element(*this, p_size, out_chunk);
	};

	inline HeapA::AllocationState Heap::allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
	{
		return HeapA::allocate_element_with_modulo_offset(*this, p_size, p_modulo_offset, out_chunk);
	};

	inline HeapA::AllocationState Heap::allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
	{
		return HeapA::allocate_element_norealloc_with_modulo_offset(*this, p_size, p_modulo_offset, out_chunk);
	};

	inline SliceIndex* Heap::get(const Token(SliceIndex) p_chunk)
	{
		return &this->AllocatedChunks.get(p_chunk);
	};

	inline void Heap::release_element(const Token(SliceIndex) p_chunk)
	{
		this->FreeChunks.push_back_element(this->AllocatedChunks.get(p_chunk));
		this->AllocatedChunks.release_element(p_chunk);
	};

	inline HeapA::AllocationState Heap::reallocate_element(const Token(SliceIndex) p_chunk, const uimax p_new_size, HeapA::AllocatedElementReturn* out_chunk)
	{
		HeapA::AllocationState l_allocation = this->allocate_element(p_new_size, out_chunk);
		if ((HeapA::AllocationState_t)l_allocation & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED)
		{
			this->release_element(p_chunk);
		}
		return l_allocation;
	};









	inline HeapPaged HeapPaged::allocate_default(const uimax p_page_size)
	{
		return HeapPaged{ Pool<SliceIndex>::allocate(0), VectorOfVector<SliceIndex>::allocate_default(), p_page_size };
	};

	inline void HeapPaged::free()
	{
		this->AllocatedChunks.free();
		this->FreeChunks.free();
	};

	inline uimax HeapPaged::get_page_count()
	{
		return this->FreeChunks.varying_vector.get_size();
	};

	inline HeapPaged::AllocationState HeapPaged::allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk)
	{
		HeapA::AllocatedElementReturn l_heap_allocated_element_return;

		for (loop(i, 0, this->FreeChunks.varying_vector.get_size()))
		{
			SingleShadowHeap l_single_shadow_heap = SingleShadowHeap::build(this, i);
			if ((HeapA::AllocationState_t)HeapA::allocate_element_norealloc_with_modulo_offset(l_single_shadow_heap, p_size, p_modulo_offset, &l_heap_allocated_element_return)
				& (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED)
			{
				*out_chunk = AllocatedElementReturn::buid_from_HeapAllocatedElementReturn(i, l_heap_allocated_element_return);
				return AllocationState::ALLOCATED;
			};
		}

		this->create_new_page();

		SingleShadowHeap l_single_shadow_heap = SingleShadowHeap::build(this, this->FreeChunks.varying_vector.get_size() - 1);
		if ((HeapA::AllocationState_t)HeapA::allocate_element_norealloc_with_modulo_offset(l_single_shadow_heap, p_size, p_modulo_offset, &l_heap_allocated_element_return)
			& (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED)
		{
			*out_chunk = AllocatedElementReturn::buid_from_HeapAllocatedElementReturn(this->FreeChunks.varying_vector.get_size() - 1, l_heap_allocated_element_return);
			return (AllocationState)((AllocationState_t)AllocationState::ALLOCATED | (AllocationState_t)AllocationState::PAGE_CREATED);
		};

		return AllocationState::NOT_ALLOCATED;
	};

	inline void HeapPaged::release_element(const HeapPagedToken& p_token)
	{
		this->FreeChunks.element_push_back_element(p_token.PageIndex, this->AllocatedChunks.get(p_token.token));
		this->AllocatedChunks.release_element(p_token.token);
	};

	inline SliceIndex* HeapPaged::get_sliceindex_only(const HeapPagedToken& p_token)
	{
		return &this->AllocatedChunks.get(p_token.token);
	};


	inline void HeapPaged::create_new_page()
	{
		SliceIndex l_chunk_slice = SliceIndex::build(0, this->PageSize);
		this->FreeChunks.push_back_element(Slice<SliceIndex>::build_memory_elementnb(&l_chunk_slice, 1));
	};










	template<class ShadowHeap_t(s)>
	inline HeapA::AllocationState HeapA::allocate_element(ShadowHeap_t(s)& p_heap, const uimax p_size, HeapA::AllocatedElementReturn* out_chunk)
	{
		if (!_allocate_element(p_heap, p_size, out_chunk))
		{
			_defragment(p_heap);
			if (!_allocate_element(p_heap, p_size, out_chunk))
			{
				uimax l_heap_size = p_heap.sh_get_size();
				p_heap.sh_resize(l_heap_size == 0 ? p_size : ((l_heap_size * 2) + p_size));

#if CONTAINER_MEMORY_TEST
				assert_true(
#endif
					_allocate_element(p_heap, p_size, out_chunk)
#if CONTAINER_MEMORY_TEST
				)
#endif
					;

				return HeapA::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
			}
			return HeapA::AllocationState::ALLOCATED;
		}
		return HeapA::AllocationState::ALLOCATED;
	};

	template<class ShadowHeap_t(s)>
	inline HeapA::AllocationState HeapA::allocate_element_with_modulo_offset(ShadowHeap_t(s)& p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
	{
		if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk))
		{
			_defragment(p_heap);
			if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk))
			{
				uimax l_heap_size = p_heap.sh_get_size();
				p_heap.sh_resize(l_heap_size == 0 ? (p_modulo_offset > p_size ? p_modulo_offset : p_size) : ((l_heap_size * 2) + (p_modulo_offset > p_size ? p_modulo_offset : p_size)));

#if CONTAINER_MEMORY_TEST
				assert_true(
#endif
					_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk)
#if CONTAINER_MEMORY_TEST
				)
#endif
					;

				return HeapA::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
			}
			return HeapA::AllocationState::ALLOCATED;
		}
		return HeapA::AllocationState::ALLOCATED;
	};

	template<class ShadowHeap_t(s)>
	inline HeapA::AllocationState HeapA::allocate_element_norealloc_with_modulo_offset(ShadowHeap_t(s)& p_heap, const uimax p_size, const uimax p_alignement_modulo, HeapA::AllocatedElementReturn* out_chunk)
	{
		if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_alignement_modulo, out_chunk))
		{
			_defragment(p_heap);
			if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_alignement_modulo, out_chunk))
			{
				return AllocationState::NOT_ALLOCATED;
			}
			return AllocationState::ALLOCATED;
		}
		return AllocationState::ALLOCATED;
	};

	template<class ShadowHeap_t(s)>
	inline int8 HeapA::_allocate_element(ShadowHeap_t(s)& p_heap, const uimax p_size, HeapA::AllocatedElementReturn* out_return)
	{
#if CONTAINER_BOUND_TEST
		assert_true(p_size != 0);
#endif

		auto& l_free_chunks = p_heap.sh_get_freechunks();
		for (uimax i = 0; i < l_free_chunks.sv_get_size(); i++)
		{
			SliceIndex& l_free_chunk = l_free_chunks.sv_get(i);
			if (l_free_chunk.Size > p_size)
			{
				SliceIndex l_new_allocated_chunk;
				l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
				*out_return = _push_chunk(p_heap, &l_new_allocated_chunk);
				return 1;
			}
			else if (l_free_chunk.Size == p_size)
			{
				*out_return = _push_chunk(p_heap, &l_free_chunk);
				l_free_chunks.sv_erase_element_at(i);
				return 1;
			}
		}

		return 0;
	};

	template<class ShadowHeap_t(s)>
	inline int8 HeapA::_allocate_element_with_modulo_offset(ShadowHeap_t(s)& p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
	{
#if CONTAINER_BOUND_TEST
		assert_true(p_size != 0);
#endif

		auto& l_free_chunks = p_heap.sh_get_freechunks();
		for (uimax i = 0; i < l_free_chunks.sv_get_size(); i++)
		{
			SliceIndex& l_free_chunk = l_free_chunks.sv_get(i);

			if (l_free_chunk.Size > p_size)
			{
				uimax l_offset_modulo = (l_free_chunk.Begin % p_modulo_offset);
				if (l_offset_modulo == 0)
				{
					// create one free chunk (after)
					SliceIndex l_new_allocated_chunk;
					l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
					*out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);
					return 1;
				}
				else
				{
					uimax l_chunk_offset_delta = p_modulo_offset - l_offset_modulo;
					// Does the offsetted new memory is able to be allocated in the chunk ?
					if (l_free_chunk.Size > (p_size + l_chunk_offset_delta)) //offsetted chunk is in the middle of the free chunk
					{
						//create two free chunk (before and after)

						SliceIndex l_new_allocated_chunk, l_new_free_chunk, l_tmp_chunk;
						l_free_chunk.slice_two(l_free_chunk.Begin + l_chunk_offset_delta, &l_free_chunk, &l_tmp_chunk);
						l_tmp_chunk.slice_two(l_tmp_chunk.Begin + p_size, &l_new_allocated_chunk, &l_new_free_chunk);
						*out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);

						l_free_chunks.sv_push_back_element(l_new_free_chunk);

						return 1;
					}
					else if (l_free_chunk.Size == (p_size + l_chunk_offset_delta)) //offsetted chunk end matches perfectly the end of the free chunk
					{
						SliceIndex l_new_allocated_chunk;
						l_free_chunk.slice_two(l_free_chunk.Begin + l_chunk_offset_delta, &l_free_chunk, &l_new_allocated_chunk);
						*out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);

						return 1;
					}
				}
			}
			else if (l_free_chunk.Size == p_size)
			{
				uimax l_offset_modulo = (l_free_chunk.Size % p_modulo_offset);
				if (l_offset_modulo == 0)
				{
					*out_chunk = _push_chunk(p_heap, &l_free_chunk);
					l_free_chunks.sv_erase_element_at(i);

					return 1;
				}
			}
		}

		return 0;
	};


	template<class ShadowHeap_t(s)>
	inline HeapA::AllocatedElementReturn HeapA::_push_chunk(ShadowHeap_t(s)& p_heap, SliceIndex* p_chunk)
	{
		return HeapA::AllocatedElementReturn::build(
			p_heap.sh_get_allocated_chunks().alloc_element(*p_chunk),
			p_chunk->Begin
		);
	};

	template<class ShadowHeap_t(s)>
	inline void HeapA::_defragment(ShadowHeap_t(s)& p_heap)
	{
		auto& l_free_chunks = p_heap.sh_get_freechunks();
		if (l_free_chunks.sv_get_size() > 0)
		{
			sort_linear2_begin(SliceIndex, defragment_sort);
			return p_left.Begin > p_right.Begin;
			sort_linear2_end(l_free_chunks.sv_to_slice(), SliceIndex, defragment_sort);

			SliceIndex& l_compared_chunk = l_free_chunks.sv_get(0);
			for (loop(i, 1, l_free_chunks.sv_get_size()))
			{
				SliceIndex& l_chunk = l_free_chunks.sv_get(i);
				if ((l_compared_chunk.Begin + l_compared_chunk.Size) == l_chunk.Begin)
				{
					l_compared_chunk.Size += l_chunk.Size;
					l_free_chunks.sv_erase_element_at(i);
					i -= 1;
				}
				else
				{
					l_compared_chunk = l_chunk;
				}
			}
		}
	};

}


#endif

#ifdef COMMON2_TREE

//TODO -> delete when common2 migration is complete
namespace v2
{
	struct NTreeNode
	{
		Token(NTreeNode) index;
		Token(NTreeNode) parent;
		PoolOfVectorToken<Token(NTreeNode)> childs;

		inline static NTreeNode build(const Token(NTreeNode) p_index, const Token(NTreeNode) p_parent, const PoolOfVectorToken<Token(NTreeNode)> p_childs)
		{
			return NTreeNode{ p_index, p_parent, p_childs };
		};

		inline static NTreeNode build_index_childs(const Token(NTreeNode) p_index, const  PoolOfVectorToken<Token(NTreeNode)> p_childs)
		{
			return NTreeNode{ p_index, tk_bd(NTreeNode), p_childs };
		};
	};

	using NTreeChildsToken = PoolOfVectorToken<Token(NTreeNode)>;

	/*
		A NTree is a hierarchy of objects with ( Parent 1 <----> N Child ) relation ship.
	*/
	template<class ElementType>
	struct NTree
	{
		Pool<ElementType> Memory;
		Pool<NTreeNode> Indices;
		PoolOfVector<Token(NTreeNode)> Indices_childs;

		struct Resolve
		{
			ElementType* Element;
			NTreeNode* Node;

			inline static Resolve build(ElementType* p_element, NTreeNode* p_node)
			{
				return Resolve{ p_element, p_node };
			};

			inline int8 has_parent() const
			{
				return tk_v(this->Node->parent) != (token_t)-1;
			};
		};

		inline static NTree<ElementType> allocate_default()
		{
			return NTree<ElementType>
			{
				Pool<ElementType>::allocate(0),
					Pool<NTreeNode>::allocate(0),
					VectorOfVector<Token(NTreeNode)>::allocate_default()
			};
		};

		inline void free()
		{
			this->Memory.free();
			this->Indices.free();
			this->Indices_childs.free();
		};


		inline Resolve get(const Token(ElementType) p_token)
		{
			return Resolve::build(
				&this->Memory.get(p_token),
				&this->Indices.get(tk_bf(NTreeNode, p_token))
			);
		};

		inline Resolve get_from_node(const Token(NTreeNode) p_token)
		{
			return this->get(tk_bf(ElementType, p_token));
		};

		inline ElementType& get_value(const Token(ElementType) p_token)
		{
			return this->Memory.get(p_token);
		};

		inline Slice<Token(NTreeNode)> get_childs(const NTreeChildsToken p_child_token)
		{
			return this->Indices_childs.get_vector(p_child_token);
		};

		inline Slice<Token(NTreeNode)> get_childs_from_node(const Token(NTreeNode) p_node)
		{
			return this->get_childs(this->get_from_node(p_node).Node->childs);
		};

		inline int8 add_child(const Resolve& p_parent, const  Resolve& p_new_child)
		{
			if (!tk_eq(p_parent.Node->index, p_new_child.Node->index))
			{
				this->detach_from_tree(p_new_child);

				p_new_child.Node->parent = p_parent.Node->index;
				this->Indices_childs.element_push_back_element(p_parent.Node->childs, p_new_child.Node->index);

				return 1;
			}
			return 0;
		};


		inline int8 add_child(const Token(ElementType) p_parent, const Token(ElementType) p_new_child)
		{
			Resolve l_new_child_value = this->get(p_new_child);
			return this->add_child(this->get(p_parent), l_new_child_value);
		};


		inline Token(ElementType) push_root_value(const ElementType& p_element)
		{
#if CONTAINER_BOUND_TEST
			assert_true(this->Memory.get_size() == 0);
#endif
			Token(ElementType) l_element;
			Token(NTreeNode) l_node;
			NTreeChildsToken l_childs;
			this->allocate_root_node(p_element, &l_element, &l_node, &l_childs);
			return l_element;
		};

		inline Token(ElementType) push_value(const ElementType& p_element, const Token(ElementType) p_parent)
		{
			Token(ElementType) l_element;
			Token(NTreeNode) l_node;
			NTreeChildsToken l_childs;
			this->allocate_node(tk_bf(NTreeNode, p_parent), p_element, &l_element, &l_node, &l_childs);
			return l_element;
		};

		template<class ForEachFunc>
		inline void traverse3(const Token(NTreeNode) p_current_node, const ForEachFunc& p_foreach_func)
		{
			Resolve l_node = this->get(tk_bf(ElementType, p_current_node));
			p_foreach_func(l_node);
			Slice<Token(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
			for (uimax i = 0; i < l_childs.Size; i++)
			{
				this->traverse3(l_childs.get(i), p_foreach_func);
			};
		};

		template<class ForEachFunc>
		inline void traverse3_excluded(const Token(NTreeNode) p_current_node, const ForEachFunc& p_foreach_func)
		{
			Resolve l_node = this->get(tk_bf(ElementType, p_current_node));
			Slice<Token(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
			for (uimax i = 0; i < l_childs.Size; i++)
			{
				this->traverse3(l_childs.get(i), p_foreach_func);
			};
		};

		inline void get_nodes(const Token(NTreeNode) p_start_node_included, Vector<Resolve>* in_out_nodes)
		{
			this->traverse3(p_start_node_included,
				[in_out_nodes](const Resolve& p_node) {in_out_nodes->push_back_element(p_node); }
			);
		};

		inline void remove_node_recursively(const Token(NTreeNode) p_node)
		{
			Vector<Resolve> l_involved_nodes = Vector<Resolve>::allocate(0);
			this->get_nodes(p_node, &l_involved_nodes);

			Slice<Resolve> l_involved_nodes_slice = l_involved_nodes.to_slice();
			this->remove_nodes_and_detach(l_involved_nodes_slice);

			l_involved_nodes.free();
		};

		inline void remove_nodes(const Slice<Resolve>& p_removed_nodes)
		{
			for (loop(i, 0, p_removed_nodes.Size))
			{
				const Resolve& l_removed_node = p_removed_nodes.get(i);
				this->Memory.release_element(tk_bf(ElementType, l_removed_node.Node->index));
				this->Indices.release_element(l_removed_node.Node->index);
				this->Indices_childs.release_vector(l_removed_node.Node->childs);
			}
		};

		inline void remove_nodes_and_detach(Slice<Resolve>& p_removed_nodes)
		{
			this->detach_from_tree(p_removed_nodes.get(0));
			this->remove_nodes(p_removed_nodes);
		};

		inline void make_node_orphan(Resolve& p_node)
		{
			this->detach_from_tree(p_node);
		};

	private:

		inline void allocate_node(const Token(NTreeNode) p_parent, const ElementType& p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element(NTreeNode::build(tk_bf(NTreeNode, *out_created_element), p_parent, *out_created_childs));

			this->Indices_childs.element_push_back_element(this->get_from_node(p_parent).Node->childs, *out_created_index);
		};

		inline void allocate_root_node(const ElementType& p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element(NTreeNode::build_index_childs(tk_bf(NTreeNode, *out_created_element), *out_created_childs));
		};

		inline void detach_from_tree(const Resolve& p_node)
		{
			if (p_node.has_parent())
			{
				Resolve l_parent = this->get_from_node(p_node.Node->parent);
				Slice<Token(NTreeNode)> l_parent_childs = this->Indices_childs.get_vector(l_parent.Node->childs);
				for (loop(i, 0, l_parent_childs.Size))
				{
					if (tk_eq(l_parent_childs.get(i), p_node.Node->index))
					{
						this->Indices_childs.element_erase_element_at_always(l_parent.Node->childs, i);
						break;
					}
				}
			}
			p_node.Node->parent = tk_bd(NTreeNode);
		};

	};
}


#endif

#ifdef COMMON2_HEAPMEMORY

namespace v2
{
	/*
		A HeapMemory is a Heap where chunks are allocated on the CPU.
	*/
	struct HeapMemory
	{
		Heap _Heap;
		Span<int8> Memory;

		inline static HeapMemory allocate(const uimax p_heap_size)
		{
			return HeapMemory{
				Heap::allocate(p_heap_size),
				Span<int8>::allocate(p_heap_size) };
		};

		inline static HeapMemory allocate_default()
		{
			return HeapMemory::allocate(0);
		};

		inline void free()
		{
			this->_Heap.free();
			this->Memory.free();
		};

		inline Token(SliceIndex) allocate_element(const Slice<int8>* p_element_bytes)
		{
			HeapA::AllocatedElementReturn l_heap_allocated_element;
			this->handle_heap_allocation_state(this->_Heap.allocate_element(p_element_bytes->Size, &l_heap_allocated_element));
			this->Memory.copy_memory(l_heap_allocated_element.Offset, *p_element_bytes);
			return l_heap_allocated_element.token;
		};

		inline Token(SliceIndex) allocate_empty_element(const uimax p_element_size)
		{
			HeapA::AllocatedElementReturn l_heap_allocated_element;
			this->handle_heap_allocation_state(this->_Heap.allocate_element(p_element_size, &l_heap_allocated_element));
			return l_heap_allocated_element.token;
		};

		inline Token(SliceIndex) allocate_empty_element_return_chunk(const uimax p_element_size, Slice<int8>* out_chunk)
		{
			HeapA::AllocatedElementReturn l_heap_allocated_element;
			this->handle_heap_allocation_state(this->_Heap.allocate_element(p_element_size, &l_heap_allocated_element));
			*out_chunk = Slice<int8>::build_memory_elementnb(&this->Memory.Memory[l_heap_allocated_element.Offset], p_element_size);
			return l_heap_allocated_element.token;
		};

		inline Token(SliceIndex) allocate_element(const Slice<int8> p_element_bytes)
		{
			return this->allocate_element(&p_element_bytes);
		};

		template <class ELementType>
		inline Token(SliceIndex) allocate_element_typed(const ELementType* p_element)
		{
			return this->allocate_element(Slice<ELementType>::build_asint8_memory_singleelement(p_element));
		};

		template <class ELementType>
		inline Token(SliceIndex) allocate_element_typed(const ELementType p_element)
		{
			return this->allocate_element_typed(&p_element);
		};

		inline void release_element(const Token(SliceIndex) p_chunk)
		{
			this->_Heap.release_element(p_chunk);
		};

		inline Slice<int8> get(const Token(SliceIndex) p_chunk)
		{
			SliceIndex* l_chunk_slice = this->_Heap.get(p_chunk);
			return Slice<int8>::build_memory_offset_elementnb(this->Memory.Memory, l_chunk_slice->Begin, l_chunk_slice->Size);
		};

		template <class ElementType>
		inline ElementType* get_typed(const Token(SliceIndex) p_chunk)
		{
			return slice_cast_singleelement<ElementType>(this->get(p_chunk));
		};

	private:
		void handle_heap_allocation_state(const HeapA::AllocationState p_allocation_state)
		{
			if ((HeapA::AllocationState_t)p_allocation_state & (HeapA::AllocationState_t)HeapA::AllocationState::HEAP_RESIZED)
			{
#if CONTAINER_MEMORY_TEST
				if (!this->Memory.resize(this->_Heap.Size))
				{
					abort();
				}
#else
				this->Memory.resize(this->_Heap.Size);
#endif
			};

#if CONTAINER_MEMORY_TEST
			if (!((HeapA::AllocationState_t)p_allocation_state & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED))
			{
				abort();
			}
#endif
		};
	};
} // namespace v2
#endif

#ifdef COMMON2_HASH

using hash_t = uimax;

// http://www.cse.yorku.ca/~oz/hash.html
inline constexpr uimax HashFunctionRaw(const int8* p_value, const uimax p_size)
{
	hash_t hash = 5381;

	for (loop(i, 0, p_size))
	{
		int8 c = *(p_value + i);
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

// http://www.cse.yorku.ca/~oz/hash.html
inline uimax HashFunctionRawChecked(const int8* p_value, const uimax p_size)
{
	hash_t hash = 5381;

	for (loop(i, 0, p_size))
	{
		int8 c = *(p_value + i);
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

#if CONTAINER_BOUND_TEST
	// -1 is usually the default value of hash_t, we we prevent in debuf mode any value that can clash
	assert_true(hash != (hash_t)-1);
#endif

	return hash;
}

template<class TYPE>
inline hash_t HashFunction(const TYPE& p_value)
{
	return HashFunctionRaw((int8*)&p_value, sizeof(TYPE));
};

template<class TYPE>
inline hash_t HashCombineFunction(const uimax p_hash, const TYPE& p_value)
{
	return p_hash ^ (HashFunction<TYPE>(p_value) + 0x9e3779b9 + (p_hash << 6) + (p_hash >> 2));
};

template<class TYPE>
inline hash_t HashSlice(const Slice<TYPE>& p_value)
{
	return HashFunctionRaw((int8*)p_value.Begin, p_value.Size * sizeof(TYPE));
};

inline hash_t HashRaw(const int8* p_str)
{
	return HashFunctionRaw((int8*)p_str, strlen(p_str));
};

inline constexpr hash_t HashRaw_constexpr(const int8* p_str)
{
	return HashFunctionRaw((int8*)p_str, strlen_constexpr(p_str));
};
#endif

#ifdef COMMON2_STRINGFUNCTIONS

namespace v2
{
	struct FromString
	{
		inline static float32 afloat32(Slice<int8> p_string)
		{
			if (p_string.Size > 0)
			{
				uimax l_dot_index;
				if (p_string.find(slice_int8_build_rawstr("."), &l_dot_index))
				{
					if (p_string.get(0) == '-')
					{
						float32 l_return = 0.0f;
						for (loop(i, l_dot_index + 1, p_string.Size))
						{
							l_return += (float32)((p_string.get(i) - '0') * (1.0f / (pow(10, (double)i - l_dot_index))));
						}
						for (loop_reverse(i, l_dot_index - 1, (uimax)0))
						{
							l_return += (float32)((p_string.get(i) - '0') * (pow(10, l_dot_index - 1 - (double)i)));
						}
						return -1.0f * l_return;
					}
					else
					{
						float32 l_return = 0.0f;
						for (loop(i, l_dot_index + 1, p_string.Size))
						{
							l_return += (float32)((p_string.get(i) - '0') * (1.0f / (pow(10, (double)i - l_dot_index))));
						}
						for (loop_reverse(i, l_dot_index - 1, (uimax)-1))
						{
							l_return += (float32)((p_string.get(i) - '0') * (pow(10, l_dot_index - 1 - (double)i)));
						}
						return l_return;
					}
				}
				/*
				else
				{
					if (p_string.get(0) == '-')
					{
						float32 l_return = 0.0f;
						for (loop_reverse(i, p_string.Size - 1, (uimax)-1))
						{
							l_return += (float32)((p_string.get(i) - '0') * (pow(10, p_string.Size - 1 - (double)i)));
						}
						return l_return;
					}
					else
					{
						float32 l_return = 0.0f;
						for (loop_reverse(i, p_string.Size - 1, (uimax)0))
						{
							l_return += (float32)((p_string.get(i) - '0') * (pow(10, p_string.Size - 1 - (double)i)));
						}
						return -1.0f * l_return;
					}

				};
				*/
			}

			return 0.0f;
		};
	};

	struct ToString
	{
		static constexpr uimax float32str_size = ((CHAR_BIT * sizeof(Limits::tol_f) / 3) + 3);

		inline static Slice<int8> afloat32(const float32 p_value, const Slice<int8>& out)
		{
#if CONTAINER_BOUND_TEST
			if (out.Size < float32str_size)
			{
				abort();
			};
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable:4996)
#endif
			int32 l_char_nb = sprintf(out.Begin, float_string_format_str, p_value);
#ifdef _MSC_VER
#pragma warning( pop ) 
#endif


			// int32 l_left = (int32)p_value;
			// int32 l_right = (int32)nearbyintf(((p_value - l_left) * pow(10, Limits::tol_digit_number)));
			// 
			// int32 l_char_nb = sprintf(out.Begin, "%i", l_left);
			// out.Begin[l_char_nb] = '.';
			// l_char_nb += 1;
			// l_char_nb += sprintf(out.Begin + l_char_nb, "%i", l_right);

			return Slice<int8>::build_memory_elementnb(out.Begin, l_char_nb);
		};
	};
}
#endif

#ifdef COMMON2_SERIALIZATION_JSON

namespace v2
{

	namespace JSONUtil
	{
		inline Slice<int8> validate_json_type(const Slice<int8>& p_type, const Slice<int8>& p_awaited_type)
		{
#if CONTAINER_BOUND_TEST
			assert_true(p_type.compare(p_awaited_type));
#endif
			return p_type;
		};

		inline void remove_spaces(String& p_source)
		{
			p_source.remove_int8s(' ');
			p_source.remove_int8s('\n');
			p_source.remove_int8s('\r');
			p_source.remove_int8s('\t');
		};
	};


	struct JSONDeserializer
	{
		struct FieldNode
		{
			Slice<int8> value;
			Slice<int8> whole_field;
		};

		Slice<int8> source;

		Slice<int8> parent_cursor;
		Vector<FieldNode> stack_fields;
		uimax current_field;


		inline static JSONDeserializer allocate_default()
		{
			return JSONDeserializer{ Slice<int8>::build_default(), Slice<int8>::build_default(), Vector<FieldNode>::allocate(0), (uimax)-1 };
		};

		inline static JSONDeserializer allocate(const Slice<int8>& p_source, const  Slice<int8>& p_parent_cursor)
		{
			return JSONDeserializer{ p_source, p_parent_cursor, Vector<FieldNode>::allocate(0), (uimax)-1 };
		};

		inline static JSONDeserializer start(String& p_source)
		{
			JSONUtil::remove_spaces(p_source);
			uimax l_start_index;
			p_source.to_slice().find(slice_int8_build_rawstr("{"), &l_start_index);
			l_start_index += 1;
			return allocate(p_source.to_slice(), p_source.to_slice().slide_rv(l_start_index));
		};

		inline JSONDeserializer clone()
		{
			return JSONDeserializer
			{
				this->source,
				this->parent_cursor,
				Vector<FieldNode>::allocate_elements(this->stack_fields.to_slice()),
				this->current_field
			};
		};

		inline void free()
		{
			this->stack_fields.free();
			this->current_field = -1;
		};


		inline int8 next_field(const int8* p_field_name)
		{
			int8 l_field_found = 0;
			Slice<int8> l_next_field_whole_value;
			if (this->find_next_field_whole_value(&l_next_field_whole_value))
			{
				String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
				l_field_name_json.append(slice_int8_build_rawstr("\""));
				l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
				l_field_name_json.append(slice_int8_build_rawstr("\":"));

				if (l_next_field_whole_value.compare(l_field_name_json.to_slice()))
				{
					Slice<int8> l_next_field_value_with_quotes = l_next_field_whole_value.slide_rv(l_field_name_json.get_int8_nb());

					FieldNode l_field_node;
					uimax l_field_value_delta;
					if (l_next_field_value_with_quotes.slide_rv(1).find(slice_int8_build_rawstr("\""), &l_field_value_delta))
					{
						l_field_node.value = l_next_field_value_with_quotes.slide_rv(1);
						l_field_node.value.Size = l_field_value_delta;
						l_field_node.whole_field = l_next_field_whole_value;

						this->stack_fields.push_back_element(l_field_node);
						this->current_field = this->stack_fields.Size - 1;

						l_field_found = 1;
					}
				}

				l_field_name_json.free();
			};


			return l_field_found;
		};


		inline int8 next_object(const int8* p_field_name, JSONDeserializer* out_object_iterator)
		{
			out_object_iterator->free();

			int8 l_field_found = 0;

			Slice<int8> l_compared_slice = this->get_current_slice_cursor();

			String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
			l_field_name_json.append(slice_int8_build_rawstr("\""));
			l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
			l_field_name_json.append(slice_int8_build_rawstr("\":"));

			FieldNode l_field_node;
			if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '{', '}', &l_field_node.whole_field, &l_field_node.value))
			{
				*out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

				this->stack_fields.push_back_element(l_field_node);
				this->current_field = this->stack_fields.Size - 1;
				l_field_found = 1;
			}

			l_field_name_json.free();

			return l_field_found;
		};


		inline int8 next_array(const int8* p_field_name, JSONDeserializer* out_object_iterator)
		{
			out_object_iterator->free();

			int8 l_field_found = 0;

			Slice<int8> l_compared_slice = this->get_current_slice_cursor();

			String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
			l_field_name_json.append(slice_int8_build_rawstr("\""));
			l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
			l_field_name_json.append(slice_int8_build_rawstr("\":"));

			FieldNode l_field_node;
			if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '[', ']', &l_field_node.whole_field, &l_field_node.value))
			{
				// To skip the first "["
				l_field_node.whole_field.Begin += 1;
				l_field_node.value.Begin += 1;

				*out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

				this->stack_fields.push_back_element(l_field_node);
				this->current_field = this->stack_fields.Size - 1;
				l_field_found = 1;
			}

			l_field_name_json.free();

			return l_field_found;
		};

		inline int8 next_array_object(JSONDeserializer* out_object_iterator)
		{

			out_object_iterator->free();

			int8 l_field_found = 0;


			Slice<int8> l_compared_slice = this->get_current_slice_cursor();

			Slice<int8> l_field_name_json_slice = slice_int8_build_rawstr("{");

			FieldNode l_field_node;
			if (find_next_json_field(l_compared_slice, l_field_name_json_slice, '{', '}', &l_field_node.whole_field, &l_field_node.value))
			{
				*out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

				this->stack_fields.push_back_element(l_field_node);
				this->current_field = this->stack_fields.Size - 1;
				l_field_found = 1;
			}
			return l_field_found;
		};

		inline FieldNode& get_currentfield()
		{
			return this->stack_fields.get(this->current_field);
		}

		inline static const Slice<int8> get_json_type(JSONDeserializer& p_deserializer)
		{
			p_deserializer.next_field("type");
			return p_deserializer.get_currentfield().value;
		};

	private:



		inline FieldNode* get_current_field()
		{
			if (this->current_field != (uimax)-1)
			{
				return &this->stack_fields.get(this->current_field);
			}
			return NULL;
		};

		inline int8 find_next_field_whole_value(Slice<int8>* out_field_whole_value)
		{

			*out_field_whole_value = this->get_current_slice_cursor();

			// If there is an unexpected int8acter, before the field name.
			// This can occur if the field is in the middle of a JSON Object.
			// This if statement is mendatory because if the field is at the first position of the JSON 
			// object, then there is no unexpected int8acter. So we handle both cases;
			if (out_field_whole_value->Size > 0 &&
				(out_field_whole_value->get(0) == ',' || out_field_whole_value->get(0) == '{'))
			{
				out_field_whole_value->slide(1);
			}

			// then we get the next field

			uimax l_new_field_index;

			if (out_field_whole_value->find(slice_int8_build_rawstr(","), &l_new_field_index)
				|| out_field_whole_value->find(slice_int8_build_rawstr("}"), &l_new_field_index))
			{
				out_field_whole_value->Size = l_new_field_index;
				return 1;
			}

			out_field_whole_value->Size = 0;
			return 0;
		};

		inline Slice<int8> get_current_slice_cursor()
		{
			Slice<int8> l_compared_slice = this->parent_cursor;
			if (this->current_field != (uimax)-1)
			{
				for (uimax i = 0; i < this->stack_fields.Size; i++)
				{
					l_compared_slice.slide(this->stack_fields.get(i).whole_field.Size);
					l_compared_slice.slide(1); //for getting after ","
				}
			}

			return l_compared_slice;
		};

		inline static int8 find_next_json_field(const Slice<int8>& p_source, const Slice<int8>& p_field_name,
			const int8 value_begin_delimiter, const int8 value_end_delimiter,
			Slice<int8>* out_object_whole_field, Slice<int8>* out_object_value_only)
		{
			if (p_source.compare(p_field_name))
			{
				Slice<int8> l_object_value = p_source.slide_rv(p_field_name.Size);

				uimax l_openedbrace_count = 1;
				uimax l_object_string_iterator = 1;
				while (l_openedbrace_count != 0 && l_object_string_iterator < l_object_value.Size)
				{
					if (l_object_value.get(l_object_string_iterator) == value_begin_delimiter)
					{
						l_openedbrace_count += 1;
					}
					else if (l_object_value.get(l_object_string_iterator) == value_end_delimiter)
					{
						l_openedbrace_count -= 1;
					}

					l_object_string_iterator += 1;
				}

				l_object_value.Size = l_object_string_iterator;

				*out_object_value_only = l_object_value;
				// out_object_value_only->Size = l_object_string_iterator - 1;
				*out_object_whole_field = p_source;
				out_object_whole_field->Size = p_field_name.Size + l_object_value.Size;

				return 1;

			}

			return 0;
		};

	};


}



#define json_deser_iterate_array_start(JsonFieldName, DeserializerVariable) \
{\
JSONDeserializer l_array = JSONDeserializer::allocate_default(), l_object = JSONDeserializer::allocate_default(); \
(DeserializerVariable)->next_array(JsonFieldName, &l_array); \
while(l_array.next_array_object(&l_object)) \
{

#define json_deser_iterate_array_end() \
}\
l_array.free(); l_object.free(); \
}

#define json_deser_iterate_array_object l_object

#define json_deser_object_field(FieldNameValue, DeserializerVariable, OutValueTypedVariable, FromString) \
(DeserializerVariable)->next_field(FieldNameValue); \
OutValueTypedVariable = FromString((DeserializerVariable)->get_currentfield().value);

#define json_deser_object(ObjectFieldNameValue, DeserializerVariable, TmpDeserializerVariable, OutValueTypedVariable, FromSerializer) \
(DeserializerVariable)->next_object(ObjectFieldNameValue, TmpDeserializerVariable); \
OutValueTypedVariable = FromSerializer(TmpDeserializerVariable);




#if CONTAINER_BOUND_TEST

#define json_get_type_checked(DeserializerVariable, AwaitedTypeSlice) \
JSONUtil::validate_json_type(JSONDeserializer::get_json_type(DeserializerVariable), AwaitedTypeSlice);

#else

#define json_get_type_checked(DeserializerVariable, AwaitedTypeSlice) \
JSONDeserializer::get_json_type(DeserializerVariable);

#endif

namespace v2
{
	struct JSONSerializer
	{
		String output;
		uimax current_indentation;

		inline static JSONSerializer allocate_default()
		{
			return JSONSerializer{
				String::allocate(0),
				0
			};
		}

		inline void free()
		{
			this->output.free();
			this->current_indentation = 0;
		}

		inline void start()
		{
			this->output.append(slice_int8_build_rawstr("{\n"));
			this->current_indentation += 1;
		};

		inline void end()
		{
			this->remove_last_coma();
			this->output.append(slice_int8_build_rawstr("}"));
			this->current_indentation -= 1;
		};

		inline void push_field(const Slice<int8>& p_name, const Slice<int8>& p_value)
		{
			this->push_indentation();
			this->output.append(slice_int8_build_rawstr("\""));
			this->output.append(p_name);
			this->output.append(slice_int8_build_rawstr("\": \""));
			this->output.append(p_value);
			this->output.append(slice_int8_build_rawstr("\",\n"));
		};

		inline void start_object(const Slice<int8>& p_name)
		{
			this->push_indentation();
			this->output.append(slice_int8_build_rawstr("\""));
			this->output.append(p_name);
			this->output.append(slice_int8_build_rawstr("\": {\n"));
			this->current_indentation += 1;
		};

		inline void start_object()
		{
			this->push_indentation();
			this->output.append(slice_int8_build_rawstr("{\n"));
			this->current_indentation += 1;
		};

		inline void end_object()
		{
			this->remove_last_coma();
			this->current_indentation -= 1;
			this->push_indentation();
			this->output.append(slice_int8_build_rawstr("},\n"));
		};

		inline void start_array(const Slice<int8>& p_name)
		{
			this->push_indentation();
			this->output.append(slice_int8_build_rawstr("\""));
			this->output.append(p_name);
			this->output.append(slice_int8_build_rawstr("\": [\n"));
			this->current_indentation += 1;
		};

		inline void end_array()
		{
			this->remove_last_coma();
			this->current_indentation -= 1;
			this->push_indentation();
			this->output.append(slice_int8_build_rawstr("],\n"));
		};

	private:
		void push_indentation()
		{
			String l_indentation = String::allocate(this->current_indentation);
			for (size_t i = 0; i < this->current_indentation; i++)
			{
				l_indentation.append(slice_int8_build_rawstr(" "));
			}
			this->output.append(l_indentation.to_slice());
			l_indentation.free();
		};

		void remove_last_coma()
		{
			if (this->output.get(this->output.Memory.Size - 1 - 2) == ',')
			{
				this->output.erase_array_at(this->output.Memory.Size - 1 - 2, 1);
			}
		};
	};
}
#endif

#ifdef COMMON2_SERIALIZATION_TYPES
namespace v2
{
	struct PrimitiveSerializedTypes
	{
		enum class Type
		{
			UNDEFINED = 0,
			FLOAT32 = UNDEFINED + 1,
			FLOAT32_2 = FLOAT32 + 1,
			FLOAT32_3 = FLOAT32_2 + 1,
			FLOAT32_4 = FLOAT32_3 + 1,
		};

		inline static uimax get_size(const Type p_type)
		{
			switch (p_type)
			{
			case Type::FLOAT32:
				return sizeof(float32);
			case Type::FLOAT32_2:
				return sizeof(float32) * 2;
			case Type::FLOAT32_3:
				return sizeof(float32) * 3;
			case Type::FLOAT32_4:
				return sizeof(float32) * 4;
			default:
				abort();
			}
		};
	};

};

#endif

#ifdef COMMON2_CONTAINER_ITERATIONS

#define slice_foreach_begin(SliceVariable, IteratorName, SliceElementVariableName) \
for (loop(IteratorName, 0, (SliceVariable)->Size)) \
{\
auto* SliceElementVariableName = (SliceVariable)->get(IteratorName); \

#define slice_foreach_end() \
}

#define vector_foreach_begin(VectorVariable, IteratorName, VectorElementVariableName) \
for (loop(IteratorName, 0, (VectorVariable)->Size)) \
{\
auto& VectorElementVariableName = (VectorVariable)->get(IteratorName);

#define vector_foreach_end() \
}

#define vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
for (vector_loop_reverse((VectorVariable), IteratorName)) \
{ \
auto& VectorElementVariableName = (VectorVariable)->get(IteratorName);

#define vector_erase_if_2_end(VectorVariable, IteratorName, IfConditionVariableName) \
if ((IfConditionVariableName))\
{\
	(VectorVariable)->erase_element_at(IteratorName);\
};\
}


#define vector_erase_if_2_break_end(VectorVariable, IteratorName, IfConditionVariableName) \
if ((IfConditionVariableName))\
{\
	(VectorVariable)->erase_element_at(IteratorName);\
	break; \
};\
}

#define vector_erase_if_2_single_line(VectorVariable, IteratorName, VectorElementVariableName, IfConditionCode) \
vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
vector_erase_if_2_end(VectorVariable, IteratorName, IfConditionCode)

#define vector_erase_if_2_break_single_line(VectorVariable, IteratorName, VectorElementVariableName, IfConditionCode) \
vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
vector_erase_if_2_break_end(VectorVariable, IteratorName, IfConditionCode)


#endif



#undef COMMON2_MACROS
#undef COMMON2_TYPES
#undef COMMON2_LIMITS
#undef COMMON2_PLATFORM_INCLUDE
#undef COMMON2_CLOCK
#undef COMMON2_THREAD
#undef COMMON2_ASSERT
#undef COMMON2_MEMORY
#undef COMMON2_SLICE
#undef COMMON2_SPAN
#undef COMMON2_TOKEN
#undef COMMON2_SORT
#undef COMMON2_VECTOR
#undef COMMON2_POOL
#undef COMMON2_STRING
#undef COMMON2_VARYINGVECTOR
#undef COMMON2_POOLINDEXED
#undef COMMON2_VECTOROFVECTOR
#undef COMMON2_POOLOFVECTOR
#undef COMMON2_HEAP
#undef COMMON2_TREE
#undef COMMON2_HEAPMEMORY
#undef COMMON2_HASH
#undef COMMON2_STRINGFUNCTIONS
#undef COMMON2_SERIALIZATION_JSON
#undef COMMON2_SERIALIZATION_TYPES
#undef COMMON2_CONTAINER_ITERATIONS