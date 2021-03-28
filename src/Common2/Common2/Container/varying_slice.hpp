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
#if __DEBUG
        uimax l_max_index = 0;
        for (loop(i, 0, p_chunks.Size))
        {
            const SliceIndex& l_chunk = *Slice_get(&p_chunks, i);
            if ((l_chunk.Begin + l_chunk.Size) >= l_max_index)
            {
                l_max_index = (l_chunk.Begin + l_chunk.Size);
            }
        }

        assert_true(p_memory.Size <= l_max_index);
#endif

        return VaryingSlice{p_memory, p_chunks};
    };

    inline uimax get_size()
    {
        return this->chunks.Size;
    };

    inline Slice<int8> get_element(const uimax p_index)
    {
        SliceIndex& l_chunk = *Slice_get(&this->chunks, p_index);
        return Slice_build_memory_offset_elementnb<int8>(this->memory.Begin, l_chunk.Begin, l_chunk.Size);
    };

    template <class ElementType> inline Slice<ElementType> get_element_typed(const uimax p_index)
    {
        Slice<int8> l_element_slice = this->get_element(p_index);
        return Slice_cast<ElementType>(&l_element_slice);
    };
};