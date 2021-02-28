#pragma once

/*
	A VaryingSlice is like a VaryingVector instead that containers are Slice (thus, not dynamically resizable).
*/
struct VaryingSlice
{
	Slice<int8> memory;
	Slice<SliceIndex> chunks;

	inline static VaryingSlice build(const Slice<int8>& p_memory, const Slice<SliceIndex>& p_chunks)
	{
#if CONTAINER_BOUND_TEST
		uimax l_max_index = 0;
		for (loop(i, 0, p_chunks.Size))
		{
			const SliceIndex& l_chunk = p_chunks.get(i);
			if ((l_chunk.Begin + l_chunk.Size) >= l_max_index)
			{
				l_max_index = (l_chunk.Begin + l_chunk.Size);
			}
		}

		assert_true(p_memory.Size <= l_max_index);
#endif

		return VaryingSlice{
				p_memory, p_chunks
		};
	};

	inline uimax get_size()
	{
		return this->chunks.Size;
	};

	inline Slice<int8> get_element(const uimax p_index)
	{
		SliceIndex& l_chunk = this->chunks.get(p_index);
		return Slice<int8>::build_memory_offset_elementnb(
				this->memory.Begin,
				l_chunk.Begin,
				l_chunk.Size
		);
	};

	template<class ElementType>
	inline Slice<ElementType> get_element_typed(const uimax p_index)
	{
		return slice_cast<ElementType>(this->get_element(p_index));
	};
};