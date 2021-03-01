#pragma once

#include "Engine/engine.hpp"

struct SandboxEngineRunner
{
    Engine engine;
    float32 simulated_delta_time;

    inline static SandboxEngineRunner allocate(const EngineConfiguration& p_engine_configuration, const float32 p_simulated_delta_time)
    {
        return SandboxEngineRunner{Engine::allocate(p_engine_configuration), p_simulated_delta_time};
    };

    template <class SandboxEngineCallbacks> inline void main_loop(SandboxEngineCallbacks& p_callbacks)
    {
        while (!this->engine.abort_condition)
        {
            this->engine.single_frame_forced_delta(this->simulated_delta_time, p_callbacks);
        }

        this->engine.free();
    };
};