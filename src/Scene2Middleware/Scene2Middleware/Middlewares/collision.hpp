#pragma once

struct BoxColliderComponentAsset
{
    v3f half_extend;
};

struct BoxColliderComponent
{
    static constexpr component_t Type = HashRaw_constexpr(STR(BoxColliderComponent));
    int8 force_update;

    Token<Node> scene_node;
    Token<BoxCollider> box_collider;
};

struct CollisionAllocator
{
    PoolIndexed<BoxColliderComponent> box_colliders;

    // TODO -> can we generalize the fact that a ressource allocation can be deferred ? YES ! we want to refactor this file so that it uses the RessourceUtility methods. Like the render.
    Vector<Token<BoxColliderComponent>> box_colliders_waiting_for_allocation;
    Vector<BoxColliderComponentAsset> box_colliders_asset_waiting_for_allocation; // linked to box_colliders_waiting_for_allocation

    static CollisionAllocator allocate_default();

    void free();

    Token<BoxColliderComponent> allocate_box_collider_component(Collision2& p_collision, const Token<Node> p_scene_node, const BoxColliderComponentAsset& p_asset);

    Token<BoxColliderComponent> allocate_box_collider_component_deferred(const Token<Node> p_scene_node, const BoxColliderComponentAsset& p_asset);

    // TODO -> this method must be removed because BoxCollidation add occur before the allocation step
    BoxColliderComponent& get_or_allocate_box_collider(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_token);

    BoxColliderComponent& get_box_collider_ressource_unsafe(const Token<BoxColliderComponent> p_box_collider_token); // unsafe

    void free_box_collider_component(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_component);

    // TODO -> this function must be removed. It is up to the consumer to ensure that the requested ressource is allocated
    v3f box_collider_get_world_half_extend(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_component);

    // TODO -> this function must be removed. It is up to the consumer to ensure that the requested ressource is allocated
    int8 box_collider_is_queued_for_detection(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_component);

    Token<ColliderDetector> attach_collider_detector(Collision2& p_collition, const Token<BoxColliderComponent> p_box_collider_component);

    void allocate_awaiting_entities(Collision2& p_collision);
};

struct CollisionMiddleware
{
    CollisionAllocator allocator;

    static CollisionMiddleware allocate_default();

    void free(Collision2& p_collision, Scene* p_scene);

    void step(Collision2& p_collision, Scene* p_scene);
};

inline CollisionAllocator CollisionAllocator::allocate_default()
{
    return CollisionAllocator{PoolIndexed<BoxColliderComponent>::allocate_default(), Vector<Token<BoxColliderComponent>>::allocate(0), Vector<BoxColliderComponentAsset>::allocate(0)};
};

inline void CollisionAllocator::free()
{
    this->box_colliders.free();
    this->box_colliders_waiting_for_allocation.free();
    this->box_colliders_asset_waiting_for_allocation.free();
};

inline Token<BoxColliderComponent> CollisionAllocator::allocate_box_collider_component(Collision2& p_collision, const Token<Node> p_scene_node, const BoxColliderComponentAsset& p_asset)
{
    Token<BoxCollider> l_box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO.vec3, p_asset.half_extend});
    return this->box_colliders.alloc_element(BoxColliderComponent{1, p_scene_node, l_box_collider});
};

inline Token<BoxColliderComponent> CollisionAllocator::allocate_box_collider_component_deferred(const Token<Node> p_scene_node, const BoxColliderComponentAsset& p_asset)
{
    Token<BoxColliderComponent> l_box_collider_token = this->box_colliders.alloc_element(BoxColliderComponent{1, p_scene_node, token_build<BoxCollider>(tokent_build_default())});
    this->box_colliders_waiting_for_allocation.push_back_element(l_box_collider_token);
    this->box_colliders_asset_waiting_for_allocation.push_back_element(p_asset);
    return l_box_collider_token;
};

inline BoxColliderComponent& CollisionAllocator::get_or_allocate_box_collider(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_token)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            Token<BoxColliderComponent> l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(p_box_collider_token, l_box_collider_token))
            {
                BoxColliderComponent& l_box_collider_component = this->box_colliders.get(l_box_collider_token);
                l_box_collider_component.box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO.vec3, this->box_colliders_asset_waiting_for_allocation.get(i).half_extend});
                l_box_collider_component.force_update = 1;
                this->box_colliders_waiting_for_allocation.erase_element_at(i);
                this->box_colliders_asset_waiting_for_allocation.erase_element_at(i);
                return l_box_collider_component;
            }
        }
    };

    return this->get_box_collider_ressource_unsafe(p_box_collider_token);
};

inline BoxColliderComponent& CollisionAllocator::get_box_collider_ressource_unsafe(const Token<BoxColliderComponent> p_box_collider_token)
{
    return this->box_colliders.get(p_box_collider_token);
}; // unsafe

inline void CollisionAllocator::free_box_collider_component(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            Token<BoxColliderComponent>& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(l_box_collider_token, p_box_collider_component))
            {
                this->box_colliders_waiting_for_allocation.erase_element_at(i);
                this->box_colliders_asset_waiting_for_allocation.erase_element_at(i);
            }
        }
    };

    Token<BoxCollider> l_box_collider = this->box_colliders.get(p_box_collider_component).box_collider;
    p_collision.free_collider(l_box_collider);
    this->box_colliders.release_element(p_box_collider_component);
};

inline v3f CollisionAllocator::box_collider_get_world_half_extend(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            Token<BoxColliderComponent>& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(l_box_collider_token, p_box_collider_component))
            {
                return this->box_colliders_asset_waiting_for_allocation.get(i).half_extend;
            }
        }
    }

    return p_collision.get_box_collider_copy(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider).local_box.radiuses;
};

inline int8 CollisionAllocator::box_collider_is_queued_for_detection(Collision2& p_collision, const Token<BoxColliderComponent> p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            Token<BoxColliderComponent>& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(l_box_collider_token, p_box_collider_component))
            {
                return 0;
            }
        }
    }

    return p_collision.is_collider_queued_for_detection(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider);
};

inline Token<ColliderDetector> CollisionAllocator::attach_collider_detector(Collision2& p_collition, const Token<BoxColliderComponent> p_box_collider_component)
{
    return p_collition.allocate_colliderdetector(this->get_box_collider_ressource_unsafe(p_box_collider_component).box_collider);
};

inline void CollisionAllocator::allocate_awaiting_entities(Collision2& p_collision)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            BoxColliderComponent& l_box_collider_component = this->box_colliders.get(this->box_colliders_waiting_for_allocation.get(i));
            l_box_collider_component.box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO.vec3, this->box_colliders_asset_waiting_for_allocation.get(i).half_extend});
            l_box_collider_component.force_update = 1;
        }

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

    this->allocator.box_colliders.foreach ([&](const Token<BoxColliderComponent> l_box_collider_token, BoxColliderComponent& l_box_collider_component) {
        NodeEntry l_node = p_scene->get_node(l_box_collider_component.scene_node);
        if (l_box_collider_component.force_update || l_node.Element->state.haschanged_thisframe)
        {
            p_collision.on_collider_moved(l_box_collider_component.box_collider, transform_pa{p_scene->tree.get_worldposition(l_node), p_scene->tree.get_worldrotation(l_node).to_axis()});
            l_box_collider_component.force_update = false;
        }
    });
};