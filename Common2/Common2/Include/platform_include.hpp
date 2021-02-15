#pragma once

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif


#include <Windows.h>
#include <sysinfoapi.h>

inline uint64 FILETIME_to_mics(FILETIME& p_filetime)
{
	ULARGE_INTEGER ul;
	ul.LowPart = p_filetime.dwLowDateTime;
	ul.HighPart = p_filetime.dwHighDateTime;
	return ul.QuadPart / 10;
};

#elif __linux__

#include <time.h>
#include <unistd.h>
#include <pthread.h>

#endif