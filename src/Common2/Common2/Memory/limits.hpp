#pragma once

const int8 Limits_tol_digit_number = 5;
const float64 Limits_TOL_f = ((float64)1 / pow(10, Limits_tol_digit_number));

#define float_string_format % .5f
#define float_string_format_str "%.5f"

#define uimax_string_format %lld
#define uimax_string_format_str "%lld"