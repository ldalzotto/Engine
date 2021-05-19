#pragma once

struct Clock
{
    uimax framecount;
    float32 totaltime;
    float32 deltatime;

    inline static Clock allocate_default()
    {
        return Clock{0, 0.0f, 0.0f};
    };

    inline void newframe()
    {
        this->framecount += 1;
    }

    inline void newupdate(float32 p_delta)
    {
        this->deltatime = p_delta;
        this->totaltime += p_delta;
    }
};

// typedef uint64 time_t;

time_t clock_currenttime_mics();

#ifdef _WIN32

inline time_t clock_currenttime_mics()
{
    FILETIME l_currentTime;
    GetSystemTimeAsFileTime(&l_currentTime);
    return FILETIME_to_mics(l_currentTime);
};

#elif __linux__

inline time_t clock_currenttime_mics()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return round(spec.tv_nsec / 1000);
};

inline timespec miliseconds_to_timespec(const uimax p_miliseconds){
    timespec l_ts;
    l_ts.tv_sec = p_miliseconds / 1000;
    l_ts.tv_nsec = (p_miliseconds % 1000) * 1000000;
    return l_ts;
};

#endif