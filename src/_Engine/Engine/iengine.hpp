#pragma once

template <class _Engine> struct iEngine
{
    _Engine& engine;

    inline void free()
    {
        this->engine.free();
    };

    inline EngineModuleCore& get_core()
    {
        return this->engine.core;
    };

    inline Token<EWindow> get_window()
    {
        return this->engine.window;
    };

    inline void frame_before()
    {
        this->engine.frame_before();
    };

    inline void frame_after()
    {
        this->engine.frame_after();
    };

    template <class LoopFunc> inline void main_loop_forced_delta_typed(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        int8 l_running = 1;
        while (l_running)
        {
            switch (this->get_core().main_loop_forced_delta_v2(p_delta))
            {
            case EngineLoopState::FRAME:
                this->frame_before();
                p_loop_func(this->get_core().clock.deltatime);
                this->frame_after();
                break;
            case EngineLoopState::ABORTED:
                l_running = 0;
                break;
            default:
                break;
            }
        }
    };

    template <class LoopFunc> inline void main_loop_blocking_typed(const LoopFunc& p_loop_func)
    {
        int8 l_running = 1;
        while (l_running)
        {
            switch (this->get_core().main_loop_non_blocking_v2())
            {
            case EngineLoopState::FRAME:
            {
                this->frame_before();
                p_loop_func(this->get_core().clock.deltatime);
                this->frame_after();
            }
            break;
            case EngineLoopState::IDLE:
                this->get_core().engine_loop.block_until_next_update();
                break;
            case EngineLoopState::ABORTED:
                l_running = 0;
                break;
            }
        }
    };

    template <class LoopFunc> inline void single_frame_forced_delta_typed(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->get_core().single_frame_forced_delta_v2(p_delta);
        this->frame_before();
        p_loop_func(p_delta);
        this->frame_after();
    };
};