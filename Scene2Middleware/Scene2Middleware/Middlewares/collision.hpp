#pragma once

namespace v2
{ 
	struct BoxColliderComponentAsset
	{
		v3f half_extend;
	};

	struct BoxColliderComponent
	{
		static constexpr component_t Type = HashRaw_constexpr(STR(BoxColliderComponent));
		int8 force_update;

		Token(Node) scene_node;
		Token(BoxCollider) box_collider;
	};

	struct CollisionAllocator
	{
		Collision2* collision;

		PoolIndexed<BoxColliderComponent> box_colliders;
		
		//TODO -> can we generalize the fact that a ressource allocation can be deferred ?
		Vector<Token(BoxColliderComponent)> box_colliders_waiting_for_allocation;
		Vector<BoxColliderComponentAsset> box_colliders_asset_waiting_for_allocation; //linked to box_colliders_waiting_for_allocation

		static CollisionAllocator allocate_default(Collision2* p_collision);
		void free();

		Token(BoxColliderComponent) allocate_box_collider_component(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset);
		Token(BoxColliderComponent) allocate_box_collider_component_deferred(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset);

		BoxColliderComponent& get_or_allocate_box_collider(const Token(BoxColliderComponent) p_box_collider_token);
		BoxColliderComponent& get_box_collider_ressource_unsafe(const Token(BoxColliderComponent) p_box_collider_token); //unsafe

		void free_box_collider_component(const Token(BoxColliderComponent) p_box_collider_component);

		v3f box_collider_get_world_half_extend(const Token(BoxColliderComponent) p_box_collider_component);
		int8 box_collider_is_queued_for_detection(const Token(BoxColliderComponent) p_box_collider_component);

		void allocate_awaiting_entities();
	};

	struct CollisionMiddleware
	{
		Collision2* collision;
		CollisionAllocator allocator;

		static CollisionMiddleware allocate_default(Collision2* p_collision);
		void free();

		void step(Scene* p_scene);
	};


}


namespace v2
{
	inline CollisionAllocator CollisionAllocator::allocate_default(Collision2* p_collision)
	{
		return CollisionAllocator{
			p_collision,
			PoolIndexed<BoxColliderComponent>::allocate_default(),
			Vector<Token(BoxColliderComponent)>::allocate(0),
			Vector<BoxColliderComponentAsset>::allocate(0)
		};
	};

	inline void CollisionAllocator::free()
	{
		this->collision = NULL;
		this->box_colliders.free();
		this->box_colliders_waiting_for_allocation.free();
		this->box_colliders_asset_waiting_for_allocation.free();
	};

	inline Token(BoxColliderComponent) CollisionAllocator::allocate_box_collider_component(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset)
	{
		Token(BoxCollider) l_box_collider = this->collision->allocate_boxcollider(aabb{ v3f_const::ZERO, p_asset.half_extend });
		return this->box_colliders.alloc_element(BoxColliderComponent{ 1, p_scene_node, l_box_collider });
	};

	inline Token(BoxColliderComponent) CollisionAllocator::allocate_box_collider_component_deferred(const Token(Node) p_scene_node, const BoxColliderComponentAsset& p_asset)
	{
		Token(BoxColliderComponent) l_box_collider_token = this->box_colliders.alloc_element(BoxColliderComponent{ 1, p_scene_node, tk_b(BoxCollider,tokent_build_default()) });
		this->box_colliders_waiting_for_allocation.push_back_element(l_box_collider_token);
		this->box_colliders_asset_waiting_for_allocation.push_back_element(p_asset);
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
					l_box_collider_component.box_collider = this->collision->allocate_boxcollider(aabb{ v3f_const::ZERO, this->box_colliders_asset_waiting_for_allocation.get(i).half_extend });
					l_box_collider_component.force_update = 1;
					this->box_colliders_waiting_for_allocation.erase_element_at(i);
					this->box_colliders_asset_waiting_for_allocation.erase_element_at(i);
					return l_box_collider_component;
				}
			}
			vector_foreach_end();
		};

		return this->get_box_collider_ressource_unsafe(p_box_collider_token);
	};

	inline BoxColliderComponent& CollisionAllocator::get_box_collider_ressource_unsafe(const Token(BoxColliderComponent) p_box_collider_token)
	{
		return this->box_colliders.get(p_box_collider_token);
	}; //unsafe

	inline void CollisionAllocator::free_box_collider_component(const Token(BoxColliderComponent) p_box_collider_component)
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
			{
				Token(BoxColliderComponent)& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
				if (tk_eq(l_box_collider_token, p_box_collider_component))
				{
					this->box_colliders_waiting_for_allocation.erase_element_at(i);
					this->box_colliders_asset_waiting_for_allocation.erase_element_at(i);
				}
			}
		};

		Token(BoxCollider) l_box_collider = this->box_colliders.get(p_box_collider_component).box_collider;
		this->collision->free_collider(l_box_collider);
		this->box_colliders.release_element(p_box_collider_component);

	};

	inline v3f CollisionAllocator::box_collider_get_world_half_extend(const Token(BoxColliderComponent) p_box_collider_component)
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
			{
				Token(BoxColliderComponent)& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
				if (tk_eq(l_box_collider_token, p_box_collider_component))
				{
					return this->box_colliders_asset_waiting_for_allocation.get(i).half_extend;
				}
			}
		}

		return this->collision->get_box_collider_copy(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider).local_box.radiuses;
	};

	inline int8 CollisionAllocator::box_collider_is_queued_for_detection(const Token(BoxColliderComponent) p_box_collider_component)
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
			{
				Token(BoxColliderComponent)& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
				if (tk_eq(l_box_collider_token, p_box_collider_component))
				{
					return 0;
				}
			}
		}

		return this->collision->is_collider_queued_for_detection(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider);
	};

	inline void CollisionAllocator::allocate_awaiting_entities()
	{
		if (this->box_colliders_waiting_for_allocation.Size != 0)
		{
			vector_foreach_begin(&this->box_colliders_waiting_for_allocation, i, l_box_collider_token);
			{
				BoxColliderComponent& l_box_collider_component = this->box_colliders.get(l_box_collider_token);
				l_box_collider_component.box_collider = this->collision->allocate_boxcollider(aabb{ v3f_const::ZERO, this->box_colliders_asset_waiting_for_allocation.get(i).half_extend });
				l_box_collider_component.force_update = 1;
			}
			vector_foreach_end();
			this->box_colliders_waiting_for_allocation.clear();
			this->box_colliders_asset_waiting_for_allocation.clear();
		}

	};


	inline CollisionMiddleware CollisionMiddleware::allocate_default(Collision2* p_collision)
	{
		return CollisionMiddleware
		{
			p_collision,
			CollisionAllocator::allocate_default(p_collision)
		};
	};

	inline void CollisionMiddleware::free()
	{
		this->allocator.free();
	};

	inline void CollisionMiddleware::step(Scene* p_scene)
	{
		this->allocator.allocate_awaiting_entities();

		poolindexed_foreach_value_begin(&this->allocator.box_colliders, i, l_box_collider_token, l_box_collider_component);
		{
			NodeEntry l_node = p_scene->get_node(l_box_collider_component.scene_node);
			if (l_box_collider_component.force_update || l_node.Element->state.haschanged_thisframe)
			{
				this->collision->on_collider_moved(l_box_collider_component.box_collider,
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