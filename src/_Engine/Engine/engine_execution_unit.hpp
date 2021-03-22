#pragma once

/*
    A general purpose Engine execution functional object that runs an Engine instance.
    This structure allows synchronisation hooks.
*/
struct EngineExecutionUnit
{
    struct CleanupCallback
    {
        void* closure;
        typedef void (*cb_t)(Engine&, void*);
        cb_t cb;

        inline void apply(Engine& p_engine)
        {
            this->cb(p_engine, this->closure);
        };
    };

    Engine engine;
    EngineExternalStepCallback step_callback;

    struct Synchronization
    {
        Barrier engine_spawned;
        BarrierTwoStep end_of_frame;
    } synchronization;

    EngineExternalStepCallback external_loop_callback;
    CleanupCallback cleanup_callback;

    inline void allocate(const EngineConfiguration& p_engine_configuration, const EngineExternalStepCallback& p_step_callback, const CleanupCallback& p_cleanup_callback)
    {
        this->synchronization.engine_spawned.close();
        this->engine = SpawnEngine(p_engine_configuration);
        this->synchronization.engine_spawned.open();
        this->external_loop_callback = p_step_callback;
        this->cleanup_callback = p_cleanup_callback;
    };

    inline void close()
    {
        this->engine.close();
    };

    inline void free()
    {
        this->cleanup_callback.apply(this->engine);
        DestroyEngine(this->engine);
    };

    inline void single_frame_no_block()
    {
        struct single_frame_no_block_cb {
            EngineExecutionUnit* thiz;
            inline void step(const EngineExternalStep p_step, Engine& p_engine) const
            {
                thiz->external_loop_callback.step(p_step, p_engine);
                EngineExecutionUnit::engine_sync(p_step, p_engine, thiz);
            };
        };
        EngineRunner::single_frame_no_block(this->engine, single_frame_no_block_cb{this});
    };

    inline void sync_wait_for_engine_to_spawn()
    {
        this->synchronization.engine_spawned.wait_for_open();

        while (FrameCount(this->engine) <= 1)
        {
            this->sync_at_end_of_frame([]() {
            });
        }
    };

    // /!\ WARNING - this doesn't guarantee that a full frame have passed by.
    template <class t_EndOfFrameFunc> inline void sync_at_end_of_frame(const t_EndOfFrameFunc& p_callback)
    {
        this->synchronization.end_of_frame.ask_and_wait_for_sync_1();
        p_callback();
        this->synchronization.end_of_frame.notify_sync_2();
    };

    template <class t_EndOfFrameFunc> inline void sync_wait_for_one_whole_frame_at_end_of_frame(const t_EndOfFrameFunc& p_callback)
    {
        this->sync_at_end_of_frame([]() {
        });
        this->sync_at_end_of_frame(p_callback);
    };

  private:
    inline static void engine_sync(const EngineExternalStep p_step, Engine& p_engine, EngineExecutionUnit* thiz)
    {
        if (p_step == EngineExternalStep::END_OF_FRAME)
        {
            if (!thiz->synchronization.end_of_frame.is_opened())
            {
                thiz->synchronization.end_of_frame.notify_sync_1_and_wait_for_sync_2();
            }
        }
    };
};
