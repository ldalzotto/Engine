
#include "Engine/engine.hpp"

int main()
{
	Engine l_engine = Engine::allocate(slice_int8_build_rawstr(""));

	struct ExternalEngine
	{
		Token(v2::Node) node_1;
		Token(ColliderDetector) node_1_collider_detector;
		Token(v2::Node) node_2;

		inline void before_collision(Engine& p_engine) {};
		inline void after_collision(Engine& p_engine) {};

		inline void before_update(Engine& p_engine)
		{
			if (p_engine.clock.framecount == 1)
			{
				this->node_1 = p_engine.scene.add_node(transform{ v3f{0.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE }, v2::Scene_const::root_node);
				this->node_2 = p_engine.scene.add_node(transform{ v3f{2.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE }, v2::Scene_const::root_node);

				Token(v2::BoxColliderComponent) l_node_1_box_collider_component = p_engine.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(p_engine.collision, this->node_1, v2::BoxColliderComponentAsset{v3f_const::ONE});
				p_engine.scene.add_node_component_by_value(this->node_1,
					v2::NodeComponent::build(v2::BoxColliderComponent::Type,
						tk_v(l_node_1_box_collider_component)
					)
				);
				this->node_1_collider_detector = p_engine.scene_middleware.collision_middleware.allocator.attach_collider_detector(p_engine.collision, l_node_1_box_collider_component);

				p_engine.scene.add_node_component_by_value(this->node_2,
					v2::NodeComponent::build(v2::BoxColliderComponent::Type,
						tk_v(p_engine.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(p_engine.collision, this->node_2, v2::BoxColliderComponentAsset{v3f_const::ONE}))
					)
				);


			}
			else if (p_engine.clock.framecount == 20 || p_engine.clock.framecount == 40 || p_engine.clock.framecount == 60 || p_engine.clock.framecount == 80 || p_engine.clock.framecount == 100)
			{
				v2::NodeEntry l_node_1_value = p_engine.scene.get_node(this->node_1);
				p_engine.scene.tree.set_localposition(l_node_1_value, p_engine.scene.tree.get_localposition(l_node_1_value) + v3f{ 1.0f,0.0f,0.0f });
				// SceneKernel::set_localposition(testContext.moving_node, l_scenehandle, SceneKernel::get_localposition(testContext.moving_node, l_scenehandle) + Math::vec3f(1.0f, 0.0f, 0.0f));
			}


			printf("Trigger events : ");
			Slice<TriggerEvent> l_0_trigger_events = p_engine.collision.get_collision_events(this->node_1_collider_detector);
			if (l_0_trigger_events.Size > 0)
			{
				printf("collider : %lld", tk_v(this->node_1_collider_detector));
				for (size_t i = 0; i < l_0_trigger_events.Size; i++)
				{
					printf(" to : %lld, ", tk_v(l_0_trigger_events.get(i).other));
					printf("%ld;", l_0_trigger_events.get(i).state);
				}
			};
			printf("\n");



			if (p_engine.clock.framecount == 110)
			{
				p_engine.close();
			}

			/*
			Slice<Trigger::Event> l_1_trigger_events = testContext.collider_detector_1.get_collision_events(l_collider_middleware->collision);
			if (l_1_trigger_events.Size > 0)
			{
				printf("collider : %lld", testContext.collider_detector_1.collider.handle);
				for (size_t i = 0; i < l_1_trigger_events.Size; i++)
				{
					printf(" to : %lld, ", l_1_trigger_events.get(i).other.handle);
					printf("%ld;", l_1_trigger_events.get(i).state);
				}
				printf("\n");
			};
			*/
		};

	} efz;


	l_engine.main_loop(efz);

	l_engine.free();
}