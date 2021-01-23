

#include "Scene2Middleware/scene2_middleware.hpp"

using namespace v2;

int main()
{
	Scene l_scene = Scene::allocate_default();
	Collision2* l_collision;
	Collision2::allocate(&l_collision);
	CollisionAllocator l_collision_allocator = CollisionAllocator::allocate_default(CollisionHandle{ l_collision });


	struct ComponentReleaser2
	{
		CollisionAllocator* collision_allocator;

		inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component)
		{
			if (p_component.type == BoxColliderComponent::Type)
			{
				this->collision_allocator->free_box_collider_component(tk_b(BoxColliderComponent, p_component.type));
			}
		};

	} component_releaser;
	component_releaser = ComponentReleaser2{ &l_collision_allocator };

	Token(Node) l_node = l_scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
	Token(BoxColliderComponent) l_box_collider_component = l_collision_allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{ v3f{} });
	l_scene.add_node_component_by_value(l_node, NodeComponent::build(BoxColliderComponent::Type, tk_v(l_box_collider_component)));

	l_scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);

	l_collision_allocator.get_or_allocate_box_collider(l_box_collider_component);

	l_scene.remove_node_component_typed<BoxColliderComponent>(l_node);
	l_scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);

	l_collision_allocator.step(&l_scene);

	l_scene.free_and_consume_component_events_stateful(component_releaser);

	l_collision_allocator.free();
	Collision2::free(&l_collision);
}