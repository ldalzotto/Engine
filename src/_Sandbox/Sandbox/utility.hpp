#pragma once

struct SandboxUtility
{
    inline static void append_frame_number_to_string(String& p_string, const uimax p_frame_number)
    {
        Slice<int8> l_uimax_str_buffer = SliceN<int8, ToString::uimaxstr_size>{}.to_slice();
        p_string.append(ToString::auimax(p_frame_number, l_uimax_str_buffer));
        p_string.append(slice_int8_build_rawstr(".jpg"));
    };
};