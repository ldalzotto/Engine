#pragma once

namespace v2
{
struct BoxColliderComponentAsset
{
    v3f half_extend;
};

struct BoxColliderComponent
{
    static const component_t Type = 17968809677980084943; // HashRaw("BoxColliderComponent")
    int8 force_update;

    TokenT(Node) scene_node;
    TokenT(BoxCollider) box_collider;
};

struct CollisionAllocator
{
    PoolIndexed<BoxColliderComponent> box_colliders;

    // TODO -> can we generalize the fact that a ressource allocation can be deferred ? YES ! we want to refactor this file so that it uses the RessourceUtility methods. Like the render.
    Vector<TokenT(BoxColliderComponent)> box_colliders_waiting_for_allocation;
    Vector<BoxColliderComponentAsset> box_colliders_asset_waiting_for_allocation; // linked to box_colliders_waiting_for_allocation

    static CollisionAllocator allocate_default();

    void free();

    TokenT(BoxColliderComponent) allocate_box_collider_component(Collision2& p_collision, const TokenT(Node) p_scene_node, const BoxColliderComponentAsset& p_asset);

    TokenT(BoxColliderComponent) allocate_box_collider_component_deferred(const TokenT(Node) p_scene_node, const BoxColliderComponentAsset& p_asset);

    BoxColliderComponent& get_or_allocate_box_collider(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_token);

    BoxColliderComponent& get_box_collider_ressource_unsafe(const TokenT(BoxColliderComponent) p_box_collider_token); // unsafe

    void free_box_collider_component(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_component);

    v3f box_collider_get_world_half_extend(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_component);

    int8 box_collider_is_queued_for_detection(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_component);

    TokenT(ColliderDetector) attach_collider_detector(Collision2& p_collition, const TokenT(BoxColliderComponent) p_box_collider_component);

    void allocate_awaiting_entities(Collision2& p_collision);
};

struct CollisionMiddleware
{
    CollisionAllocator allocator;

    static CollisionMiddleware allocate_default();

    void free(Collision2& p_collision, Scene* p_scene);

    void step(Collision2& p_collision, Scene* p_scene);
};

} // namespace v2

namespace v2
{
inline CollisionAllocator CollisionAllocator::allocate_default()
{
    return CollisionAllocator{PoolIndexed<BoxColliderComponent>::allocate_default(), Vector<TokenT(BoxColliderComponent)>::allocate(0), Vector<BoxColliderComponentAsset>::allocate(0)};
};

inline void CollisionAllocator::free()
{
    this->box_colliders.free();
    this->box_colliders_waiting_for_allocation.free();
    this->box_colliders_asset_waiting_for_allocation.free();
};

inline TokenT(BoxColliderComponent) CollisionAllocator::allocate_box_collider_component(Collision2& p_collision, const TokenT(Node) p_scene_node, const BoxColliderComponentAsset& p_asset)
{
    TokenT(BoxCollider) l_box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO, p_asset.half_extend});
    return this->box_colliders.alloc_element(BoxColliderComponent{1, p_scene_node, l_box_collider});
};

inline TokenT(BoxColliderComponent) CollisionAllocator::allocate_box_collider_component_deferred(const TokenT(Node) p_scene_node, const BoxColliderComponentAsset& p_asset)
{
    TokenT(BoxColliderComponent) l_box_collider_token = this->box_colliders.alloc_element(BoxColliderComponent{1, p_scene_node, tk_bT(BoxCollider, tokent_build_default())});
    this->box_colliders_waiting_for_allocation.push_back_element(l_box_collider_token);
    this->box_colliders_asset_waiting_for_allocation.push_back_element(p_asset);
    return l_box_collider_token;
};

inline BoxColliderComponent& CollisionAllocator::get_or_allocate_box_collider(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_token)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        vector_foreach_begin(&this->box_colliders_waiting_for_allocation, i, l_box_collider_token);
        {
            if (tk_eq(p_box_collider_token, l_box_collider_token))
            {
                BoxColliderComponent& l_box_collider_component = this->box_colliders.get(l_box_collider_token);
                l_box_collider_component.box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO, this->box_colliders_asset_waiting_for_allocation.get(i).half_extend});
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

inline BoxColliderComponent& CollisionAllocator::get_box_collider_ressource_unsafe(const TokenT(BoxColliderComponent) p_box_collider_token)
{
    return this->box_colliders.get(p_box_collider_token);
}; // unsafe

inline void CollisionAllocator::free_box_collider_component(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            TokenT(BoxColliderComponent)& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (tk_eq(l_box_collider_token, p_box_collider_component))
            {
                this->box_colliders_waiting_for_allocation.erase_element_at(i);
                this->box_colliders_asset_waiting_for_allocation.erase_element_at(i);
            }
        }
    };

    TokenT(BoxCollider) l_box_collider = this->box_colliders.get(p_box_collider_component).box_collider;
    p_collision.free_collider(l_box_collider);
    this->box_colliders.release_element(p_box_collider_component);
};

inline v3f CollisionAllocator::box_collider_get_world_half_extend(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            TokenT(BoxColliderComponent)& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (tk_eq(l_box_collider_token, p_box_collider_component))
            {
                return this->box_colliders_asset_waiting_for_allocation.get(i).half_extend;
            }
        }
    }

    return p_collision.get_box_collider_copy(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider).local_box.radiuses;
};

inline int8 CollisionAllocator::box_collider_is_queued_for_detection(Collision2& p_collision, const TokenT(BoxColliderComponent) p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            TokenT(BoxColliderComponent)& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (tk_eq(l_box_collider_token, p_box_collider_component))
            {
                return 0;
            }
        }
    }

    return p_collision.is_collider_queued_for_detection(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider);
};

inline TokenT(ColliderDetector) CollisionAllocator::attach_collider_detector(Collision2& p_collition, const TokenT(BoxColliderComponent) p_box_collider_component)
{
    return p_collition.allocate_colliderdetector(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider);
};

inline void CollisionAllocator::allocate_awaiting_entities(Collision2& p_collision)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        vector_foreach_begin(&this->box_colliders_waiting_for_allocation, i, l_box_collider_token);
        {
            BoxColliderComponent& l_box_collider_component = this->box_colliders.get(l_box_collider_token);
            l_box_collider_component.box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO, this->box_colliders_asset_waiting_for_allocation.get(i).half_extend});
            l_box_collider_component.force_update = 1;
        }
        vector_foreach_end();
        this->box_colliders_waiting_for_allocation.clear();
        this->box_colliders_asset_waiting_for_allocation.clear();
    }
};

inline CollisionMiddleware CollisionMiddleware::allocate_default()
{
    return CollisionMiddleware{CollisionAllocator::allocate_default()};
};

inline void CollisionMiddleware::free(Collision2& p_collision, Scene* p_scene)
{
    this->step(p_collision, p_scene);

    this->allocator.free();
};

inline void CollisionMiddleware::step(Collision2& p_collision, Scene* p_scene)
{
    this->allocator.allocate_awaiting_entities(p_collision);

    poolindexed_foreach_value_begin(&this->allocator.box_colliders, i, l_box_collider_token, l_box_collider_component);
    {
        NodeEntry l_node = p_scene->get_node(l_box_collider_component.scene_node);
        if (l_box_collider_component.force_update || l_node.Element->state.haschanged_thisframe)
        {
            p_collision.on_collider_moved(l_box_collider_component.box_collider, transform_pa{p_scene->tree.get_worldposition(l_node), p_scene->tree.get_worldrotation(l_node).to_axis()});
            l_box_collider_component.force_update = false;
        }
    }
    poolindexed_foreach_token_2_end();
};
} // namespace v2