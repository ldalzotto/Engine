#pragma once

time_t clock_native_currenttime_mics()
{
    FILETIME l_currentTime;
    GetSystemTimeAsFileTime(&l_currentTime);
    return FILETIME_to_mics(l_currentTime);
};
