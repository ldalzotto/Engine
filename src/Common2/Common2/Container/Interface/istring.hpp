#pragma once

template <class _String> struct iString
{
    _String& string;

    inline void append(const Slice<int8>& p_elements)
    {
        this->string.append(p_elements);
    };

    inline void insert_array_at(const Slice<int8>& p_elements, const uimax p_index)
    {
        this->string.insert_array_at(p_elements, p_index);
    };

    inline void erase_array_at(const uimax p_index, const uimax p_size)
    {
        this->string.erase_array_at(p_index, p_size);
    };

    inline void pop_back_array(const uimax p_size)
    {
        this->string.pop_back_array(p_size);
    };

    inline void erase_array_at_always(const uimax p_index, const uimax p_size)
    {
        this->string.erase_array_at_always(p_index, p_size);
    };

    inline int8& get(const uimax p_index)
    {
        return this->string.get(p_index);
    };

    inline uimax get_size() const
    {
        return this->string.get_size();
    };

    inline uimax get_length() const
    {
        return this->string.get_length();
    };

    inline void clear()
    {
        this->string.clear();
    };

    inline Slice<int8> to_slice()
    {
        return this->string.to_slice();
    };
};