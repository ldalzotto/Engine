#pragma once

/*
    A VaryingVector is a Vector which elements can have different sizes.
    Elements are accessed vie the Chunks_t lookup table.
    Memory is continuous.
*/
struct VaryingVector
{
    Vector<int8> memory;
    Vector<SliceIndex> chunks;

    inline static VaryingVector build(const Vector<int8>& p_memory, const Vector<SliceIndex>& p_chunks)
    {
        return VaryingVector{p_memory, p_chunks};
    };

    inline static VaryingVector allocate(const uimax p_memory_array_initial_capacity, const uimax p_chunk_array_initial_capacity)
    {
        return build(Vector<int8>::allocate(p_memory_array_initial_capacity), Vector<SliceIndex>::allocate(p_chunk_array_initial_capacity));
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

    inline void push_back_2(const Slice<int8>& p_bytes_1, const Slice<int8>& p_bytes_2)
    {
        SliceIndex l_chunk = SliceIndex::build(this->memory.Size, p_bytes_1.Size + p_bytes_2.Size);
        this->memory.push_back_array_2(p_bytes_1, p_bytes_2);
        this->chunks.push_back_element(l_chunk);
    };

    inline void push_back_empty(const uimax p_slice_size)
    {
        SliceIndex l_chunk = SliceIndex::build(this->memory.Size, p_slice_size);
        this->memory.push_back_array_empty(p_slice_size);
        this->chunks.push_back_element(l_chunk);
    };

    template <class ElementType> inline void push_back_element(const ElementType& p_element)
    {
        this->push_back(Slice_build_memory_elementnb<int8>((int8*)&p_element, sizeof(ElementType)));
    };

    inline void pop_back()
    {
        this->memory.pop_back_array(this->chunks.get(this->chunks.Size - 1).Size);
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
        this->memory.erase_array_at(l_chunk.Begin, l_chunk.Size);

        for (loop(i, p_index, this->chunks.Size))
        {
            this->chunks.get(i).Begin -= l_chunk.Size;
        };

        this->chunks.erase_element_at(p_index);
    };

    inline void erase_element_at_always(const uimax p_index)
    {
        if (p_index == this->get_size() - 1)
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

        this->memory.erase_array_at(l_removed_chunk.Begin, l_removed_chunk.Size);

        for (loop(i, p_index + p_element_nb, this->chunks.Size))
        {
            this->chunks.get(i).Begin -= l_removed_chunk.Size;
        };

        this->chunks.erase_array_at(p_index, p_element_nb);
    };

    inline void element_expand(const uimax p_index, const uimax p_expansion_size)
    {
        SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if __DEBUG
        assert_true(p_expansion_size != 0);
#endif

        uimax l_new_varyingvector_size = this->memory.Size + p_expansion_size;

        Span_resize_until_capacity_met(&this->memory.Memory, l_new_varyingvector_size);
        l_updated_chunk.Size += p_expansion_size;

        for (loop(i, p_index + 1, this->chunks.Size))
        {
            this->chunks.get(i).Begin += p_expansion_size;
        }
    };

    inline void element_expand_with_value(const uimax p_index, const Slice<int8>& p_pushed_element)
    {
        SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if __DEBUG
        assert_true(p_pushed_element.Size != 0);
#endif

        uimax l_size_delta = p_pushed_element.Size;
        uimax l_new_varyingvector_size = this->memory.Size + l_size_delta;

        Span_resize_until_capacity_met(&this->memory.Memory, l_new_varyingvector_size);

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

#if __DEBUG
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
        Slice<int8> l_updated_chunk_slice = Slice_build_asint8_memory_elementnb<int8>(this->memory.get_memory() + l_updated_chunk.Begin, l_updated_chunk.Size);
        Slice<int8> l_updated_chunk_slice_offsetted = Slice_slide_rv(&l_updated_chunk_slice, p_insertion_offset);

        Slice_copy_memory(&l_updated_chunk_slice_offsetted, &p_inserted_element);
    };

    inline void element_movememory(const uimax p_index, const uimax p_insertion_offset, const Slice<int8>& p_inserted_element)
    {
        SliceIndex& l_updated_chunk = this->chunks.get(p_index);
        Slice<int8> l_updated_chunk_slice = Slice_build_asint8_memory_elementnb<int8>(this->memory.get_memory() + l_updated_chunk.Begin, l_updated_chunk.Size);
        Slice<int8> l_updated_chunk_slice_offsetted = Slice_slide_rv(&l_updated_chunk_slice, p_insertion_offset);
        Slice_move_memory(&l_updated_chunk_slice_offsetted, &p_inserted_element);
    };

    inline Slice<int8> get_element(const uimax p_index)
    {
        SliceIndex& l_chunk = this->chunks.get(p_index);
        return Slice_build_memory_offset_elementnb<int8>(this->memory.get_memory(), l_chunk.Begin, l_chunk.Size);
    };

    inline Slice<int8> get_last_element()
    {
        return this->get_element(this->get_size() - 1);
    };

    template <class ElementType> inline Slice<ElementType> get_element_typed(const uimax p_index)
    {
        Slice<int8> l_element_slice = this->get_element(p_index);
        return Slice_cast<ElementType>(&l_element_slice);
    };

    inline VaryingSlice to_varying_slice()
    {
        return VaryingSlice{this->memory.to_slice(), this->chunks.to_slice()};
    };
};