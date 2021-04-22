#pragma once

/*
    A String is a vector of int8 that always have a NULL int8acter at it's last element.
*/
struct String
{
    Vector<int8> Memory;

    inline static String allocate(const uimax p_initial_capacity)
    {
        int8 l_null_int8 = (int8)NULL;
        return String{Vector<int8>::allocate_capacity_elements(p_initial_capacity + 1, Slice<int8>::build_asint8_memory_elementnb(&l_null_int8, 1))};
    };

    inline static String allocate_elements(const Slice<int8>& p_initial_elements)
    {
        String l_string = String{Vector<int8>::allocate_capacity_elements(p_initial_elements.Size + 1, p_initial_elements)};
        l_string.Memory.push_back_element((int8)NULL);
        return l_string;
    };

    inline static String allocate_elements_2(const Slice<int8>& p_initial_elements_0, const Slice<int8>& p_initial_elements_1)
    {
        String l_string = String{Vector<int8>::allocate_capacity_elements_2(p_initial_elements_0.Size + p_initial_elements_1.Size + 1, p_initial_elements_0, p_initial_elements_1)};
        l_string.Memory.push_back_element((int8)NULL);
        return l_string;
    };

    inline static String allocate_elements_3(const Slice<int8>& p_initial_elements_0, const Slice<int8>& p_initial_elements_1, const Slice<int8>& p_initial_elements_2)
    {
        String l_string = String{Vector<int8>::allocate_capacity_elements_3(p_initial_elements_0.Size + p_initial_elements_1.Size + +p_initial_elements_2.Size + 1, p_initial_elements_0,
                                                                            p_initial_elements_1, p_initial_elements_2)};
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
#if __DEBUG
        assert_true((p_index + p_size) <= this->get_length());
#endif
        this->Memory.erase_array_at(p_index, p_size);
    };

    inline void pop_back_array(const uimax p_size)
    {
#if __DEBUG
        assert_true(this->get_length() >= p_size);
#endif

        this->Memory.erase_array_at(this->get_length() - p_size, p_size);
    };

    inline void erase_array_at_always(const uimax p_index, const uimax p_size)
    {
        this->Memory.erase_array_at_always(p_index, p_size);
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

    inline uimax get_length() const
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

    inline Slice<int8> to_slice() const
    {
        return ((String*)this)->to_slice();
    };

    inline Slice<int8> to_slice_with_null_termination()
    {
        return Slice<int8>::build_memory_elementnb(this->Memory.Memory.Memory, this->Memory.Size);
    };
};