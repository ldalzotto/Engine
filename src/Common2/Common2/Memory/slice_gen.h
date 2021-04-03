#pragma once

#define mp_type_element int8
#include "./slice_tt.h"

inline Slice(int8) Slice_c_int8_build_rawstr(const int8* p_str)
{
    return Slice(int8){.slice = Slice__int8_build_rawstr(p_str)};
};
