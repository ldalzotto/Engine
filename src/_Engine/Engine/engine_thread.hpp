#pragma once

struct EngineThreadSync
{
    struct Synchronization
    {
        BarrierTwoStep start_of_frame_barrier;
        BarrierTwoStep end_of_frame_barrier;
    } synchronization;
    int8 ask_exit = 0;

    inline void exit()
    {
        this->ask_exit = 1;
    };

    inline void reset()
    {
        this->synchronization.start_of_frame_barrier.state = BarrierTwoStep::State::OPENED;
        this->synchronization.end_of_frame_barrier.state = BarrierTwoStep::State::OPENED;
        this->ask_exit = 0;
    };

    template <class t_Callback> inline void sync_start_of_frame(const t_Callback& p_callback)
    {
        this->synchronization.start_of_frame_barrier.ask_and_wait_for_sync_1();
        {
            p_callback();
        }
        this->synchronization.start_of_frame_barrier.notify_sync_2();
    };

    template <class t_Callback> inline void sync_end_of_frame(const t_Callback& p_callback)
    {
        this->synchronization.end_of_frame_barrier.ask_and_wait_for_sync_1();
        {
            p_callback();
        }
        this->synchronization.end_of_frame_barrier.notify_sync_2();
    };

    template <class t_Callback> inline void sync_wait_for_one_whole_frame_at_end_of_frame(const t_Callback& p_callback)
    {
        this->sync_end_of_frame([]() {
        });
        this->sync_end_of_frame(p_callback);
    };

    inline void on_start_of_frame()
    {
        if (!this->synchronization.start_of_frame_barrier.is_opened())
        {
            this->synchronization.start_of_frame_barrier.notify_sync_1_and_wait_for_sync_2();
        }
    };

    inline void on_end_of_frame()
    {
        if (!this->synchronization.end_of_frame_barrier.is_opened())
        {
            this->synchronization.end_of_frame_barrier.notify_sync_1_and_wait_for_sync_2();
        }
    };
};