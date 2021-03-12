#pragma once

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if __PREPROCESS
// clang-format off
@include <Windows.h>
@include <sysinfoapi.h>
// clang-format on
#else
#include <Windows.h>
#include <sysinfoapi.h>
#endif

#if MEM_LEAK_DETECTION
#if __PREPROCESS
// clang-format off
@include "dbghelp.h"
// clang-format on
#else
#include "dbghelp.h"
#endif
#endif

inline uimax dword_lowhigh_to_uimax(const DWORD p_low, const DWORD p_high)
{
    ULARGE_INTEGER ul;
    ul.LowPart = p_low;
    ul.HighPart = p_high;
    return ul.QuadPart;
};

inline uint64 FILETIME_to_mics(const FILETIME* p_filetime)
{
    return dword_lowhigh_to_uimax(p_filetime->dwLowDateTime, p_filetime->dwHighDateTime) / 10;
};

#elif __linux__

#include <time.h>
#include <unistd.h>
#include <pthread.h>

#endif
