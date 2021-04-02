#pragma once

struct SandboxUtility
{
    inline static void append_frame_number_to_string(String& p_string, const uimax p_frame_number)
    {
        Slice_declare_sized(int8, ToString::uimaxstr_size, l_uimax_str_buffer, l_uimax_str_buffer_slice, );
        p_string.append(ToString::auimax(p_frame_number, l_uimax_str_buffer_slice));
        p_string.append(Slice_int8_build_rawstr(".jpg"));
    };
};