#pragma once

typedef struct Clock
{
    uimax framecount;
    float32 deltatime;
} Clock;

inline static Clock Clock_build(const uimax p_framecount, const float32 p_deltatime)
{
#if __cplusplus
    return Clock{p_framecount, p_deltatime};
#else
    return (Clock){.framecount = p_framecount, .deltatime = p_deltatime};
#endif
};

inline static Clock Clock_build_default()
{
    return Clock_build(0, 0.0f);
};

inline void Clock_newframe(Clock* thiz)
{
    thiz->framecount += 1;
}

inline void Clock_newupdate(Clock* thiz, float32 p_delta)
{
    thiz->deltatime = p_delta;
}

time_t clock_currenttime_mics();

#ifdef _WIN32

inline time_t clock_currenttime_mics()
{
    FILETIME l_currentTime;
    GetSystemTimeAsFileTime(&l_currentTime);
    return FILETIME_to_mics(&l_currentTime);
};

#elif __linux__

inline time_t clock_currenttime_mics()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return round(spec.tv_nsec / 1000);
};

#endif