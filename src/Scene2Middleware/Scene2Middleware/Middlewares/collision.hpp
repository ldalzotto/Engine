#pragma once

struct BoxColliderComponentAsset
{
    v3f half_extend;
};

struct BoxColliderComponent
{
    static constexpr component_t Type = HashFunctions::hash_compile<strlen_compile::get_size(STR(BoxColliderComponent))>(STR(BoxColliderComponent));
    int8 force_update;

    Node_Token scene_node;
    BoxCollider_Token box_collider;
};

// TODO -> define token in the structure with better name
using t_BoxColliderComponents = PoolIndexed<BoxColliderComponent>;
using BoxColliderComponent_Token = t_BoxColliderComponents::sToken;
using BoxColliderComponent_TokenValue = t_BoxColliderComponents::sTokenValue;

struct CollisionAllocator
{
    PoolIndexed<BoxColliderComponent> box_colliders;

    // TODO -> can we generalize the fact that a resource allocation can be deferred ? YES ! we want to refactor this file so that it uses the ResourceUtility methods. Like the render.
    Vector<BoxColliderComponent_Token> box_colliders_waiting_for_allocation;
    Vector<BoxColliderComponentAsset> box_colliders_asset_waiting_for_allocation; // linked to box_colliders_waiting_for_allocation

    static CollisionAllocator allocate_default();

    void free();

    BoxColliderComponent_Token allocate_box_collider_component(Collision2& p_collision, const Node_Token p_scene_node, const BoxColliderComponentAsset& p_asset);

    BoxColliderComponent_Token allocate_box_collider_component_deferred(const Node_Token p_scene_node, const BoxColliderComponentAsset& p_asset);

    // TODO -> this method must be removed because BoxCollidation add occur before the allocation step
    BoxColliderComponent& get_or_allocate_box_collider(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_token);

    BoxColliderComponent& get_box_collider_resource_unsafe(const BoxColliderComponent_Token p_box_collider_token); // unsafe

    void free_box_collider_component(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_component);

    // TODO -> this function must be removed. It is up to the consumer to ensure that the requested resource is allocated
    v3f box_collider_get_world_half_extend(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_component);

    // TODO -> this function must be removed. It is up to the consumer to ensure that the requested resource is allocated
    int8 box_collider_is_queued_for_detection(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_component);

    ColliderDetector_Token attach_collider_detector(Collision2& p_collition, const BoxColliderComponent_Token p_box_collider_component);

    void allocate_awaiting_entities(Collision2& p_collision);
};

struct CollisionMiddleware
{
    CollisionAllocator allocator;

    static CollisionMiddleware allocate_default();

    void free(Collision2& p_collision);

    void step(Collision2& p_collision, Scene* p_scene);
};

inline CollisionAllocator CollisionAllocator::allocate_default()
{
    return CollisionAllocator{PoolIndexed<BoxColliderComponent>::allocate_default(), Vector<BoxColliderComponent_Token>::allocate(0), Vector<BoxColliderComponentAsset>::allocate(0)};
};

inline void CollisionAllocator::free()
{
    this->box_colliders.free();
    this->box_colliders_waiting_for_allocation.free();
    this->box_colliders_asset_waiting_for_allocation.free();
};

inline BoxColliderComponent_Token CollisionAllocator::allocate_box_collider_component(Collision2& p_collision, const Node_Token p_scene_node, const BoxColliderComponentAsset& p_asset)
{
    BoxCollider_Token l_box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO.vec3, p_asset.half_extend});
    return this->box_colliders.alloc_element(BoxColliderComponent{1, p_scene_node, l_box_collider});
};

inline BoxColliderComponent_Token CollisionAllocator::allocate_box_collider_component_deferred(const Node_Token p_scene_node, const BoxColliderComponentAsset& p_asset)
{
    BoxColliderComponent_Token l_box_collider_token = this->box_colliders.alloc_element(BoxColliderComponent{1, p_scene_node, token_build<BoxCollider_TokenValue>(tokent_build_default())});
    this->box_colliders_waiting_for_allocation.push_back_element(l_box_collider_token);
    this->box_colliders_asset_waiting_for_allocation.push_back_element(p_asset);
    return l_box_collider_token;
};

inline BoxColliderComponent& CollisionAllocator::get_or_allocate_box_collider(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_token)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            BoxColliderComponent_Token l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
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

    return this->get_box_collider_resource_unsafe(p_box_collider_token);
};

inline BoxColliderComponent& CollisionAllocator::get_box_collider_resource_unsafe(const BoxColliderComponent_Token p_box_collider_token)
{
    return this->box_colliders.get(p_box_collider_token);
}; // unsafe

inline void CollisionAllocator::free_box_collider_component(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            BoxColliderComponent_Token& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(l_box_collider_token, p_box_collider_component))
            {
                this->box_colliders_waiting_for_allocation.erase_element_at(i);
                this->box_colliders_asset_waiting_for_allocation.erase_element_at(i);
            }
        }
    };

    BoxCollider_Token l_box_collider = this->box_colliders.get(p_box_collider_component).box_collider;
    p_collision.free_collider(l_box_collider);
    this->box_colliders.release_element(p_box_collider_component);
};

inline v3f CollisionAllocator::box_collider_get_world_half_extend(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            BoxColliderComponent_Token& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(l_box_collider_token, p_box_collider_component))
            {
                return this->box_colliders_asset_waiting_for_allocation.get(i).half_extend;
            }
        }
    }

    return p_collision.get_box_collider_copy(this->get_box_collider_resource_unsafe(p_box_collider_component).box_collider).local_box.radiuses;
};

inline int8 CollisionAllocator::box_collider_is_queued_for_detection(Collision2& p_collision, const BoxColliderComponent_Token p_box_collider_component)
{
    if (this->box_colliders_waiting_for_allocation.Size != 0)
    {
        for (loop(i, 0, this->box_colliders_waiting_for_allocation.Size))
        {
            BoxColliderComponent_Token& l_box_collider_token = this->box_colliders_waiting_for_allocation.get(i);
            if (token_equals(l_box_collider_token, p_box_collider_component))
            {
                return 0;
            }
        }
    }

    return p_collision.is_collider_queued_for_detection(this->get_box_collider_resource_unsafe(p_box_collider_component).box_collider);
};

inline ColliderDetector_Token CollisionAllocator::attach_collider_detector(Collision2& p_collition, const BoxColliderComponent_Token p_box_collider_component)
{
    return p_collition.allocate_colliderdetector(this->get_box_collider_resource_unsafe(p_box_collider_component).box_collider);
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

inline void CollisionMiddleware::free(Collision2& p_collision)
{
    this->allocator.free();
};

inline void CollisionMiddleware::step(Collision2& p_collision, Scene* p_scene)
{
    this->allocator.allocate_awaiting_entities(p_collision);

    this->allocator.box_colliders.foreach ([&](const BoxColliderComponent_Token l_box_collider_token, BoxColliderComponent& l_box_collider_component) {
        NodeEntry l_node = p_scene->get_node(l_box_collider_component.scene_node);
        if (l_box_collider_component.force_update || l_node.Element->state.haschanged_thisframe)
        {
            p_collision.on_collider_moved(l_box_collider_component.box_collider, transform_pa{p_scene->tree.get_worldposition(l_node), p_scene->tree.get_worldrotation(l_node).to_axis()});
            l_box_collider_component.force_update = false;
        }
    });
};