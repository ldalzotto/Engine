#pragma once

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

	//TODO -> write test :)
	inline static Span<ElementType> allocate_slice(const Slice<ElementType>& p_elements)
	{
		Span<ElementType> l_span = Span<ElementType>::allocate(p_elements.Size);
		slice_memcpy(l_span.slice, Slice<ElementType>::build_memory_elementnb((ElementType*)p_elements.Begin, p_elements.Size));
		return l_span;
	};

	template<uint32 Size_t>
	inline static Span<ElementType> allocate_slicen(const SliceN<ElementType, Size_t>& p_elements)
	{
		Span<ElementType> l_span = Span<ElementType>::allocate(Size_t);
		slice_memcpy(l_span.slice, Slice<ElementType>::build_memory_elementnb((ElementType*)p_elements.Memory, Size_t));
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
		{ l_resized_capacity = 1; }

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

};



