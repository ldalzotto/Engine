#pragma once


struct EngineConfiguration
{
	Slice<int8> asset_database_path;
};

struct Engine
{
	int8 abort_condition;

	Clock clock;
	EngineLoop engine_loop;

	Collision2 collision;
	v2::GPUContext gpu_context;
	v2::D3Renderer renderer;

	v2::Scene scene;
	v2::SceneMiddleware scene_middleware;

	AssetDatabase asset_database;

	static Engine allocate(const EngineConfiguration& p_configuration);

	void free();

	void close();

	template<class ExternalCallbacksFn>
	void main_loop(ExternalCallbacksFn& p_callbacks);


	template<class ExternalCallbacksFn>
	void single_frame_forced_delta(const float32 p_delta, ExternalCallbacksFn& p_callbacks);

private:

	struct ComponentReleaser
	{
		Engine& engine;

		void on_component_removed(v2::Scene* p_scene, const v2::NodeEntry& p_node, const v2::NodeComponent& p_component);
	};

	template<class ExternalCallbacksFn>
	struct LoopCallbacks
	{
		Engine& engine;
		ExternalCallbacksFn& external_callbacks;

		void newframe();

		void update(const float32 p_delta);

		void render();

		void endofframe();
	};

	template<class ExternalCallbacksFn>
	void single_frame(ExternalCallbacksFn& p_callbacks);

};


inline void Engine::ComponentReleaser::on_component_removed(v2::Scene* p_scene, const v2::NodeEntry& p_node, const v2::NodeComponent& p_component)
{
	on_node_component_removed(&this->engine.scene_middleware, this->engine.collision, this->engine.renderer, this->engine.gpu_context, p_component);
};

inline Engine Engine::allocate(const EngineConfiguration& p_configuration)
{
	v2::GPUContext l_gpu_context = v2::GPUContext::allocate();
	v2::D3Renderer l_renderer = v2::D3Renderer::allocate(l_gpu_context, v2::ColorStep::AllocateInfo{ v3ui{ 8, 8, 1 }, 0 });
	return Engine
			{
					0,
					Clock::allocate_default(),
					EngineLoop::allocate_default(1000000 / 60),
					Collision2::allocate(),
					l_gpu_context,
					l_renderer,
					v2::Scene::allocate_default(),
					v2::SceneMiddleware::allocate_default(),
					AssetDatabase::allocate(p_configuration.asset_database_path)
			};
};

inline void Engine::free()
{
	ComponentReleaser l_component_releaser = ComponentReleaser{ *this };
	this->scene.consume_component_events_stateful(l_component_releaser);
	this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_context, this->asset_database);
	this->asset_database.free();
	this->collision.free();
	this->renderer.free(this->gpu_context);
	this->gpu_context.free();
	this->scene.free();
};

inline void Engine::close()
{
	this->abort_condition = 1;
};

template<class ExternalCallbacksFn>
inline void Engine::main_loop(ExternalCallbacksFn& p_callbacks)
{
	while (!this->abort_condition)
	{
		this->single_frame(p_callbacks);
	};
};

template<class ExternalCallbacksFn>
inline void Engine::single_frame_forced_delta(const float32 p_delta, ExternalCallbacksFn& p_callbacks)
{
	LoopCallbacks<ExternalCallbacksFn> engine_callbacks = LoopCallbacks<ExternalCallbacksFn>{ *this, p_callbacks };
	this->engine_loop.update_forced_delta(p_delta, engine_callbacks);
};

template<class ExternalCallbacksFn>
inline void Engine::single_frame(ExternalCallbacksFn& p_callbacks)
{
	LoopCallbacks<ExternalCallbacksFn> engine_callbacks = LoopCallbacks<ExternalCallbacksFn>{ *this, p_callbacks };
	this->engine_loop.update(engine_callbacks);
};

template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::newframe()
{
	this->engine.clock.newframe();
	//TODO -> input
};

template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::update(const float32 p_delta)
{
	this->engine.clock.newupdate(p_delta);

	this->external_callbacks.before_collision(this->engine);

	this->engine.collision.step();

	this->external_callbacks.after_collision(this->engine);
	this->external_callbacks.before_update(this->engine);

	this->engine.scene_middleware.step(&this->engine.scene, this->engine.collision, this->engine.renderer, this->engine.gpu_context, this->engine.asset_database);
};

template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::render()
{
	this->engine.renderer.buffer_step(this->engine.gpu_context);
	this->engine.gpu_context.buffer_step_and_submit();
	v2::GraphicsBinder l_graphics_binder = this->engine.gpu_context.creates_graphics_binder();
	this->engine.renderer.graphics_step(l_graphics_binder);
	this->engine.gpu_context.submit_graphics_binder(l_graphics_binder);
	this->engine.gpu_context.wait_for_completion();
};

template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::endofframe()
{
	ComponentReleaser l_component_releaser = ComponentReleaser{ this->engine };
	this->engine.scene.consume_component_events_stateful(l_component_releaser);
	this->engine.scene.step();
};