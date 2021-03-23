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

struct EngineRunnerThread
{
    struct EngineAllocationEvent
    {
        Token(EngineExecutionUnit) token;
        String database_path;
        uint32 width;
        uint32 height;
        EngineExternalStepCallback external_loop_callback;
        EngineExecutionUnit::CleanupCallback cleanup_callback;

        inline void free()
        {
            this->database_path.free();
        };
    };

    Vector<EngineAllocationEvent> allocation_events;
    PoolIndexed<EngineExecutionUnit> engines;

    int8 ask_exit;

    struct Synchronization
    {
        BarrierTwoStep start_of_step_barrier;
        BarrierTwoStep end_of_step_barrier;
    } synchronization;

    int8* thread_input_args;
    Thread::MainInput thread_input;
    thread_t thread;

    inline static EngineRunnerThread allocate()
    {
        EngineRunnerThread l_runner{};
        l_runner.allocation_events = Vector<EngineAllocationEvent>::allocate(0);
        l_runner.engines = PoolIndexed<EngineExecutionUnit>::allocate_default();
        l_runner.ask_exit = 0;
        return l_runner;
    };

    inline void start()
    {
        this->thread_input_args = (int8*)this;
        this->thread_input = Thread::MainInput{EngineRunnerThread::main, Slice<int8*>::build_begin_end(&this->thread_input_args, 0, 1)};
        this->thread = Thread::spawn_thread(this->thread_input);
    };

    inline void free()
    {
        this->ask_exit = 1;
        Thread::wait_for_end_and_terminate(this->thread, -1);
    };

    inline Token(EngineExecutionUnit) allocate_engine_execution_unit(const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height, const EngineExternalStepCallback& p_step_cb,
                                                                     const EngineExecutionUnit::CleanupCallback& p_cleanup_cb)
    {
        Token(EngineExecutionUnit) l_engine;
        this->synchronization.start_of_step_barrier.ask_and_wait_for_sync_1();
        {
            l_engine = this->engines.alloc_element(EngineExecutionUnit{});
            this->allocation_events.push_back_element(EngineAllocationEvent{l_engine, String::allocate_elements(p_asset_database), p_width, p_height, p_step_cb, p_cleanup_cb});
        }
        this->synchronization.start_of_step_barrier.notify_sync_2();

        return l_engine;
    };

    inline EngineExecutionUnit* sync_wait_for_engine_execution_unit_to_be_allocated(const Token(EngineExecutionUnit) p_engine_exectuion_unit)
    {
        while (this->allocation_events.Size > 0)
        {
        }
        EngineExecutionUnit* l_engine_execution_unit;
        this->synchronization.end_of_step_barrier.ask_and_wait_for_sync_1();
        {
            l_engine_execution_unit = &this->engines.get(p_engine_exectuion_unit);
        }
        this->synchronization.end_of_step_barrier.notify_sync_2();
        return l_engine_execution_unit;
    };

    template <class t_Callback> inline void sync_start_of_step(const t_Callback& p_callback)
    {
        this->synchronization.start_of_step_barrier.ask_and_wait_for_sync_1();
        {
            p_callback();
        }
        this->synchronization.start_of_step_barrier.notify_sync_2();
    };

    template <class t_Callback> inline void sync_end_of_step(const t_Callback& p_callback)
    {
        this->synchronization.end_of_step_barrier.ask_and_wait_for_sync_1();
        {
            p_callback();
        }
        this->synchronization.end_of_step_barrier.notify_sync_2();
    };

    template <class t_StartOfStepCallback, class t_EndOfStepCallback>
    inline void sync_step_loop(t_StartOfStepCallback& p_start_of_step_callback, t_EndOfStepCallback& p_end_of_step_callback, int8* p_exit)
    {
        this->synchronization.start_of_step_barrier.ask_for_sync_1();
        while (!(*p_exit))
        {
            this->synchronization.start_of_step_barrier.wait_for_sync_1();
            {
                p_start_of_step_callback();
            }
            this->synchronization.end_of_step_barrier.ask_for_sync_1();
            this->synchronization.start_of_step_barrier.notify_sync_2();
            this->synchronization.end_of_step_barrier.wait_for_sync_1();
            {
                p_end_of_step_callback();
            }
            this->synchronization.end_of_step_barrier.notify_sync_2();
            this->synchronization.start_of_step_barrier.ask_for_sync_1();
        }
        this->synchronization.start_of_step_barrier.wait_for_sync_1();
        this->synchronization.end_of_step_barrier.ask_for_sync_1();
        this->synchronization.start_of_step_barrier.notify_sync_2();
        this->synchronization.end_of_step_barrier.wait_for_sync_1();
        this->synchronization.end_of_step_barrier.notify_sync_2();
    };

    inline void free_engine_execution_unit(const Token(EngineExecutionUnit) p_engine_execution_unit)
    {
        sync_start_of_step([&]() {
            this->engines.get(p_engine_execution_unit).close();
        });
    };

    inline void free_engine_execution_unit_sync(const Token(EngineExecutionUnit) p_engine_execution_unit)
    {
        this->synchronization.start_of_step_barrier.ask_and_wait_for_sync_1();
        this->synchronization.end_of_step_barrier.ask_for_sync_1();
        {
            this->engines.get(p_engine_execution_unit).close();
        }
        this->synchronization.start_of_step_barrier.notify_sync_2();

        this->synchronization.end_of_step_barrier.wait_for_sync_1();
        this->synchronization.end_of_step_barrier.notify_sync_2();
    };

  private:
    inline static int8 main(const Slice<int8*>& p_args)
    {
        EngineRunnerThread* thiz = (EngineRunnerThread*)p_args.get(0);

        while (true)
        {
            if (!thiz->ask_exit)
            {
                thiz->step();
            }
            else
            {
                thiz->exit_step();
                break;
            }
        }

        return 0;
    };

    inline void step()
    {
        if (!this->synchronization.start_of_step_barrier.is_opened())
        {
            this->synchronization.start_of_step_barrier.notify_sync_1_and_wait_for_sync_2();
        }

        for (loop(i, 0, this->allocation_events.Size))
        {
            EngineAllocationEvent& l_event = this->allocation_events.get(i);
            EngineConfiguration l_configuration{};
            l_configuration.asset_database_path = l_event.database_path.to_slice();
            l_configuration.render_size.x = l_event.width;
            l_configuration.render_size.y = l_event.height;

            this->engines.get(l_event.token).allocate(l_configuration, l_event.external_loop_callback, l_event.cleanup_callback);

            l_event.free();
        }

        this->allocation_events.clear();

        this->engines.foreach_reverse([&](Token(EngineExecutionUnit) p_engine_token, EngineExecutionUnit& p_engine) {
            if (p_engine.engine.abort_condition)
            {
                p_engine.free();
                this->engines.release_element(p_engine_token);
            }
            else
            {
                p_engine.single_frame_no_block();
            }
        });

        if (this->engines.Indices.Size > 0)
        {
            time_t l_min_time = this->engines.get_by_index(0).engine.engine_loop.get_remaining_time_for_update();
            for (loop(i, 1, this->engines.Indices.Size))
            {
                time_t l_current_time = this->engines.get_by_index(i).engine.engine_loop.get_remaining_time_for_update();
                if (l_current_time <= l_min_time)
                {
                    l_min_time = l_current_time;
                }
            }
            // printf("%lld \n", l_min_time);
            Thread::wait(Thread::get_current_thread(), (uimax)((float)l_min_time * 0.0009999f));
        }

        if (!this->synchronization.end_of_step_barrier.is_opened())
        {
            this->synchronization.end_of_step_barrier.notify_sync_1_and_wait_for_sync_2();
        }
    };

    inline void exit_step()
    {
        for (loop(i, 0, this->allocation_events.Size))
        {
            EngineAllocationEvent& p_event = this->allocation_events.get(i);
            p_event.free();
        }
        this->allocation_events.free();

        this->engines.foreach ([](Token(EngineExecutionUnit) p_engine_token, EngineExecutionUnit& p_engine) {
            p_engine.free();
        });
        this->engines.free();
    };
};