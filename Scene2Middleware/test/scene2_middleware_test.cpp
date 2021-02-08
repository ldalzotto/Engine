

#include "Scene2Middleware/scene2_middleware.hpp"


namespace v2
{
	struct ComponentReleaser2
	{
		Collision2& collision;
		SceneMiddleware* scene_middleware;

		inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component)
		{
			on_node_component_removed(this->scene_middleware, this->collision, p_component);
		};

	};

	inline void collision_middleware_component_allocation()
	{
		Scene l_scene = Scene::allocate_default();
		Collision2 l_collision = Collision2::allocate();
		SceneMiddleware l_scene_middleware = SceneMiddleware::allocate_default();
		ComponentReleaser2 component_releaser = ComponentReleaser2{ l_collision, &l_scene_middleware };

		/*
			We allocate the BoxCollider component with the "deferred" path.
			  - The BoxColliderComponentAsset is descontructible even if the CollisionMiddleware step is not called.
			We step the scene_middleware.
			  - The BoxColliderComponentAsset is still descontructible.
			  - But there is no more pending BoxCollider ressource allocation in the collision_middleware.
			The component associated to the BoxCollider component is removed.
			We consume the scene deletion exent.
		*/
		{
			v3f l_half_extend = { 1.0f, 2.0f, 3.0f };
			Token(Node) l_node = l_scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(BoxColliderComponent) l_box_collider_component = l_scene_middleware.collision_middleware.allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{ l_half_extend });

			NodeComponent l_box_collider_node_component = BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component);
			l_scene.add_node_component_by_value(l_node, l_box_collider_node_component);
			l_scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);

			{
				BoxColliderComponentAsset l_box_collider_component_asset = 
					BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_scene_middleware, l_collision, l_box_collider_node_component);
				assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
				assert_true(l_scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 1);
			}
			{
				l_scene_middleware.step(&l_scene, l_collision);
				BoxColliderComponentAsset l_box_collider_component_asset = 
					BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_scene_middleware, l_collision, l_box_collider_node_component);
				assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
				assert_true(l_scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 0);
			}

			l_scene.remove_node_component_typed<BoxColliderComponent>(l_node);
			l_scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);

		}


		{
			v3f l_half_extend = { 1.0f, 2.0f, 3.0f };
			Token(Node) l_node = l_scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(BoxColliderComponent) l_box_collider_component = l_scene_middleware.collision_middleware.allocator.allocate_box_collider_component(l_collision, l_node, BoxColliderComponentAsset{ l_half_extend });

			NodeComponent l_box_collider_node_component = BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component);
			l_scene.add_node_component_by_value(l_node, l_box_collider_node_component);
			l_scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);

			{
				BoxColliderComponentAsset l_box_collider_component_asset = BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_scene_middleware, l_collision, l_box_collider_node_component);
				assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
				assert_true(l_scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 0);
			}

			l_scene.remove_node_component_typed<BoxColliderComponent>(l_node);
			l_scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);
		}

		l_scene.consume_component_events_stateful(component_releaser);
		l_scene_middleware.free(&l_scene, l_collision);
		l_collision.free();
		l_scene.free();
	};

	inline void collision_middleware_queuing_for_calculation()
	{
		Scene l_scene = Scene::allocate_default();
		Collision2 l_collision = Collision2::allocate();
		SceneMiddleware l_scene_middleware = SceneMiddleware::allocate_default();
		ComponentReleaser2 component_releaser = ComponentReleaser2{ l_collision, &l_scene_middleware };

		{
			v3f l_half_extend = { 1.0f, 2.0f, 3.0f };
			Token(Node) l_node = l_scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(BoxColliderComponent) l_box_collider_component = l_scene_middleware.collision_middleware.allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{ l_half_extend });
			l_scene.add_node_component_by_value(l_node, BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component));

			assert_true(!l_scene_middleware.collision_middleware.allocator.box_collider_is_queued_for_detection(l_collision, l_box_collider_component));

			l_scene_middleware.step(&l_scene, l_collision);

			assert_true(l_scene_middleware.collision_middleware.allocator.box_collider_is_queued_for_detection(l_collision, l_box_collider_component));

			l_scene.remove_node_component_typed<BoxColliderComponent>(l_node);
		}

		l_scene.consume_component_events_stateful(component_releaser);
		l_scene_middleware.free(&l_scene, l_collision);
		l_collision.free();
		l_scene.free();
	};
};

int main()
{
	v2::collision_middleware_component_allocation();
	v2::collision_middleware_queuing_for_calculation();

	memleak_ckeck();
}