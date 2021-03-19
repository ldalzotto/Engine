#pragma once

/*
    A general purpose thread that runs an Engine instance.
    This structure allows synchronisation hooks
*/
// TODO -> adding test :)
// TODO -> This is not the right way to handle Engine execution on a thread. With the current implementation, one thread is limited to a single Engine.
//         It is a wiser choice to implement an "EngineExecutionUnit" that executes the Engine step by step (Like the Sandbox execution). It is up to the caller to decide on which thread the execution
//         is done.
// TODO -> Also, isn't it weird to evaluate thread synchronization inside the main loop ? (yes) Shouldn't we have an "EngineRunner" that handle all synchronisation requests ?
struct EngineThreadV2
{
    Engine engine;

    struct Synchronization
    {
        Barrier engine_spawned;
        BarrierTwoStep end_of_frame;
    } synchronization;

    struct Input
    {
        String database_path;
        uint32 width;
        uint32 height;
        int8* external_loop_callback;
        int8* thread_input_closure;
        Thread::MainInput thread_input;
    } input;

    thread_t thread;

    inline static EngineThreadV2 allocate()
    {
        EngineThreadV2 l_return{};
        l_return.input.database_path = String::allocate(0);
        return l_return;
    };

    template <class t_ExternalLoopFunc> inline void start(const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height, const t_ExternalLoopFunc& p_external_loop_callback)
    {
        this->synchronization.engine_spawned.close();
        this->input.database_path.append(p_asset_database);
        this->input.width = p_width;
        this->input.height = p_height;
        this->input.external_loop_callback = (int8*)&p_external_loop_callback;
        this->input.thread_input_closure = (int8*)this;
        this->input.thread_input = Thread::MainInput{EngineThreadV2::main<t_ExternalLoopFunc>, Slice<int8*>::build_begin_end(&this->input.thread_input_closure, 0, 1)};
        this->thread = Thread::spawn_thread(this->input.thread_input);
    };

    inline void free()
    {
        this->kill();
        this->input.database_path.free();
    }

    inline int8 is_running()
    {
        return this->thread != NULL;
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

    inline void kill()
    {
        this->engine.close();
        if (this->is_running())
        {
            Thread::wait_for_end_and_terminate(this->thread, -1);
            this->thread = NULL;
        }
    };

    template <class t_ExternalLopp> inline static int8 main(const Slice<int8*>& p_args)
    {
        EngineThreadV2* thiz = (EngineThreadV2*)p_args.get(0);

        EngineConfiguration l_engine_config{};
        l_engine_config.asset_database_path = thiz->input.database_path.to_slice();
        l_engine_config.render_size = v2ui{thiz->input.width, thiz->input.height};

        thiz->engine = SpawnEngine(l_engine_config);

        thiz->synchronization.engine_spawned.open();

        struct s_engine_loop
        {
            EngineThreadV2* thread;
            t_ExternalLopp& l_external_loop;
            inline void step(const EngineExternalStep p_step, Engine& p_engine) const
            {
                this->l_external_loop.step(p_step, p_engine);
                engine_sync(p_step, p_engine, thread);
            };
        };

        t_ExternalLopp& l_external_loop = *(t_ExternalLopp*&)thiz->input.external_loop_callback;
        thiz->engine.main_loop(s_engine_loop{thiz, l_external_loop});

        l_external_loop.cleanup_ressources(thiz->engine);
        DestroyEngine(thiz->engine);

        return 0;
    };

  private:
    inline static void engine_sync(const EngineExternalStep p_step, Engine& p_engine, EngineThreadV2* thiz)
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