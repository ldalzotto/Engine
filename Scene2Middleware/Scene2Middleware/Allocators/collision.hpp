#pragma once

namespace v2
{
	struct BoxColliderComponentAsset
	{
		v3f radiuses;
	};

	struct BoxColliderComponent
	{
		static component_t Type;
		int8 force_update;

		Token(Node) scene_node;
		BoxColliderHandle box_collider;
	};

	component_t BoxColliderComponent::Type = 0;

	struct CollisionAllocator
	{
		CollisionHandle collision;

		PoolIndexed<BoxColliderComponent> box_colliders;
		Pool<BoxColliderComponentAsset> box_colliders_asset; // TODO
		Vector<Token(BoxColliderComponent)> box_colliders_waiting_for_allocation;

		static CollisionAllocator allocate_default(const CollisionHandle p_collision);
		void free();

		Token(BoxColliderComponent) allocate_box_collider_component(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset);
		Token(BoxColliderComponent) allocate_box_collider_component_deferred(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset);

		BoxColliderComponent& get_or_allocate_box_collider(const Token(BoxColliderComponent) p_box_collider_token);
		BoxColliderComponent& get_box_collider(const Token(BoxColliderComponent) p_box_collider_token); //unsafe

		void free_box_collider_component(const Token(BoxColliderComponent) p_box_collider_component);

		void step(Scene* p_scene);
	};
}


namespace v2
{
	inline CollisionAllocator CollisionAllocator::allocate_default(const CollisionHandle p_collision)
	{
		return CollisionAllocator{
			p_collision, PoolIndexed<BoxColliderComponent>::allocate_default(), 
			Pool<BoxColliderComponentAsset>::allocate(0),
			Vector<Token(BoxColliderComponent)>::allocate(0)
		};
	};

	inline void CollisionAllocator::free()
	{
		this->collision = CollisionHandle::build_default();
		this->box_colliders_waiting_for_allocation.free();
		this->box_colliders.free();
	};

	inline Token(BoxColliderComponent) CollisionAllocator::allocate_box_collider_component(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset)
	{
		BoxColliderHandle l_box_collider;
		l_box_collider.allocate(this->collision, aabb{v3f_const::ZERO, p_asset.radiuses });
		this->box_colliders_asset.alloc_element(p_asset);
		return this->box_colliders.alloc_element(BoxColliderComponent{ 1, p_scene_node, l_box_collider });
	};

	inline Token(BoxColliderComponent) CollisionAllocator::allocate_box_collider_component_deferred(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset)
	{
		this->box_colliders_asset.alloc_element(p_asset);
		Token(BoxColliderComponent) l_box_collider_token = this->box_colliders.alloc_element(BoxColliderComponent{ 1, p_scene_node, BoxColliderHandle::build_default() });
		this->box_colliders_waiting_for_allocation.push_back_element(l_box_collider_token);
		return l_box_collider_token;
	};


	inline BoxColliderComponent& CollisionAllocator::get_or_allocate_box_collider(const Token(BoxColliderComponent) p_box_collider_token)
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			vector_foreach_begin(&this->box_colliders_waiting_for_allocation, i, l_box_collider_token);
			{
				if (tk_eq(p_box_collider_token, l_box_collider_token))
				{
					BoxColliderComponent& l_box_collider_component = this->box_colliders.get(l_box_collider_token);
					l_box_collider_component.box_collider.allocate(this->collision, aabb{v3f_const::ZERO, this->box_colliders_asset.get(tk_bf(BoxColliderComponentAsset, p_box_collider_token)).radiuses});
					l_box_collider_component.force_update = 1;
					return l_box_collider_component;
				}
			}
			vector_foreach_end();
		};

		return this->get_box_collider(p_box_collider_token);
	};

	inline BoxColliderComponent& CollisionAllocator::get_box_collider(const Token(BoxColliderComponent) p_box_collider_token)
	{
		return this->box_colliders.get(p_box_collider_token);
	}; //unsafe

	inline void CollisionAllocator::free_box_collider_component(const Token(BoxColliderComponent) p_box_collider_component)
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			vector_erase_if_2_break_single_line(&this->box_colliders_waiting_for_allocation, i,
				l_box_collider_token, tk_eq(l_box_collider_token, p_box_collider_component));
		};

		BoxColliderHandle l_box_collider = this->box_colliders.get(p_box_collider_component).box_collider;
		l_box_collider.free(this->collision);
		this->box_colliders.release_element(p_box_collider_component);
		this->box_colliders_asset.release_element(tk_bf(BoxColliderComponentAsset, p_box_collider_component));
	};

	inline 	void CollisionAllocator::step(Scene* p_scene)
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			vector_foreach_begin(&this->box_colliders_waiting_for_allocation, i, l_box_collider_token);
			{
				BoxColliderComponent& l_box_collider_component = this->box_colliders.get(l_box_collider_token);
				l_box_collider_component.box_collider.allocate(this->collision, aabb{v3f_const::ZERO, this->box_colliders_asset.get(tk_bf(BoxColliderComponentAsset, l_box_collider_token)).radiuses});
				l_box_collider_component.force_update = 1;
			}
			vector_foreach_end();
			this->box_colliders_waiting_for_allocation.clear();
		}

		poolindexed_foreach_value_begin(&this->box_colliders, i, l_box_collider_token, l_box_collider_component);
		{
			NodeEntry l_node = p_scene->get_node(l_box_collider_component.scene_node);
			if (l_box_collider_component.force_update || l_node.Element->state.haschanged_thisframe)
			{
				l_box_collider_component.box_collider.on_collider_moved(this->collision,
					transform_pa{
					p_scene->tree.get_worldposition(l_node),
						p_scene->tree.get_worldrotation(l_node).to_axis()
				});
				l_box_collider_component.force_update = false;
			}
		}
		poolindexed_foreach_token_2_end();
	};

}