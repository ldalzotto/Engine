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
