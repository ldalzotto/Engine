
namespace Path
{
inline int8 move_up(String* p_string)
{
    Slice<int8> l_string_slice = p_string->to_slice();
    uimax l_last_slash_index;
    if (Slice_last_index_of_not_endofslice(l_string_slice, slice_int8_build_rawstr("/"), &l_last_slash_index))
    {
        if (l_last_slash_index < (l_string_slice.Size - 1))
        {
            p_string->pop_back_array(l_string_slice.Size - (l_last_slash_index + 1));
            return 1;
        }
    }
    return 0;
};
}; // namespace Path