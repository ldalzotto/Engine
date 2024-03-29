#pragma once

namespace Limits
{
// const int8* float_string_format = "%f.5";
const int8 tol_digit_number = 5;
const float64 tol_f = ((float64)1 / pow(10, tol_digit_number));
}; // namespace Limits

#define float_string_format % .5f
#define float_string_format_str "%.5f"

#define uimax_string_format %lld
#define uimax_string_format_str "%lld"

#define int32_string_format_str "%d"