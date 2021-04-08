#pragma once

struct FromString
{
    inline static float32 afloat32(Slice<int8> p_string)
    {
        if (p_string.Size > 0)
        {
            uimax l_dot_index;
            if (Slice_find(p_string, slice_int8_build_rawstr("."), &l_dot_index))
            {
                if (p_string.get(0) == '-')
                {
                    float32 l_return = 0.0f;
                    for (loop(i, l_dot_index + 1, p_string.Size))
                    {
                        l_return += (float32)((p_string.get(i) - '0') * (1.0f / (pow(10, (double)i - l_dot_index))));
                    }
                    for (loop_reverse(i, 1, l_dot_index))
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
                    for (loop_reverse(i, 0, l_dot_index))
                    {
                        l_return += (float32)((p_string.get(i) - '0') * (pow(10, l_dot_index - 1 - (double)i)));
                    }
                    return l_return;
                }
            }
        }

        return 0.0f;
    };

    inline static uimax auimax(const Slice<int8>& p_string)
    {
        uimax l_value = 0;
        for (loop(i, 0, p_string.Size))
        {
            l_value += (uimax)((p_string.get(i) - '0') * (uimax)pow(10, ((double)p_string.Size - 1) - i));
        }
        return l_value;
    };

    inline static uint8 auint8(const Slice<int8>& p_string)
    {
        uint8 l_value = 0;
        for (loop(i, 0, p_string.Size))
        {
            l_value += (uint8)((p_string.get(i) - '0') * (uint8)pow(10, ((double)p_string.Size - 1) - i));
        }
        return l_value;
    };
};

struct ToString
{
    static constexpr uimax float32str_size = ((CHAR_BIT * sizeof(Limits::tol_f) / 3) + 3);
    static constexpr uimax uimaxstr_size = ((CHAR_BIT * sizeof(uimax) / 3) + 3);

    inline static Slice<int8> afloat32(const float32 p_value, const Slice<int8>& out)
    {
#if __DEBUG
        if (out.Size < float32str_size)
        {
            abort();
        };
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
        int32 l_char_nb = sprintf(out.Begin, float_string_format_str, p_value);
#ifdef _MSC_VER
#pragma warning(pop)
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

    inline static Slice<int8> auimax(const uimax p_value, const Slice<int8>& out)
    {
#if __DEBUG
        if (out.Size < uimaxstr_size)
        {
            abort();
        };
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
        int32 l_char_nb = sprintf(out.Begin, uimax_string_format_str, p_value);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

        return Slice<int8>::build_memory_elementnb(out.Begin, l_char_nb);
    };

    inline static void auimax_append(const uimax p_value, String& p_str)
    {
        SliceN<int8, uimaxstr_size> l_uimax_str_buffer_arr = {};
        p_str.append(auimax(p_value, slice_from_slicen(&l_uimax_str_buffer_arr)));
    };
};