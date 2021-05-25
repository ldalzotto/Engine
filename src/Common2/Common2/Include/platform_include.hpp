#pragma once

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <sysinfoapi.h>

inline uimax dword_lowhigh_to_uimax(const DWORD p_low, const DWORD p_high)
{
    ULARGE_INTEGER ul;
    ul.LowPart = p_low;
    ul.HighPart = p_high;
    return ul.QuadPart;
};

inline uint64 FILETIME_to_mics(FILETIME& p_filetime)
{
    return dword_lowhigh_to_uimax(p_filetime.dwLowDateTime, p_filetime.dwHighDateTime) / 10;
};

#elif __linux__

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xmd.h>

template <class ParameterType> inline ParameterType xlib_status_handle(const ParameterType p_function_return)
{
#if __DEBUG
    if (p_function_return == 0)
    {
        abort();
    }
#endif
    return p_function_return;
};

template <class ParameterType> inline ParameterType xlib_error_handle(const ParameterType p_function_return)
{
#if __DEBUG
    if (p_function_return != 0)
    {
        abort();
    }
#endif
    return p_function_return;
};

#endif
