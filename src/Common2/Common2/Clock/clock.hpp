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
