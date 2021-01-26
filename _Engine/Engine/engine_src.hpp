#pragma once



struct Engine
{
	int8 abort_condition;

	Clock clock;
	EngineLoop engine_loop;

	Collision2 collision;

	v2::Scene scene;
	v2::SceneMiddleware scene_middleware;

	static Engine allocate(const Slice<int8>& p_executable_path);
	void free();
	void close();

	template<class ExternalCallbacksFn>
	void main_loop(ExternalCallbacksFn& p_callbacks);
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
	on_node_component_removed(&this->engine.scene_middleware, this->engine.collision, p_component);
};

inline Engine Engine::allocate(const Slice<int8>& p_executable_path)
{
	return Engine
	{
		0,
		Clock::allocate_default(),
		EngineLoop::allocate_default(1000000 / 60),
		Collision2::allocate(),
		v2::Scene::allocate_default(),
		v2::SceneMiddleware::allocate_default()
	};
};

inline void Engine::free()
{
	ComponentReleaser l_component_releaser = ComponentReleaser{ *this };
	this->scene.free_and_consume_component_events_stateful(l_component_releaser);
	this->scene_middleware.free();
	this->collision.free();
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

//TODO -> external hooks
template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::update(const float32 p_delta)
{
	this->engine.clock.newupdate(p_delta);

	this->external_callbacks.before_collision(this->engine);

	this->engine.collision.step();

	this->external_callbacks.after_collision(this->engine);
	this->external_callbacks.before_update(this->engine);

	this->engine.scene_middleware.step(&this->engine.scene, this->engine.collision);

	//TODO -> render
};

template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::render() {};

template<class ExternalCallbacksFn>
inline void Engine::LoopCallbacks<ExternalCallbacksFn>::endofframe()
{
	this->engine.scene.step();
};