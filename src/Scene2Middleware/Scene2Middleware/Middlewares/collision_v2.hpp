#pragma once

struct ColliderDetectorInstance
{
    int8 is_allocated;
    Token(ColliderDetector) collider_detector;

    struct AllocationEvent
    {
        // Token(BoxColliderComponent_TMP)
        Token(ColliderDetectorInstance) collider_detector;
    };

    struct FreeEvent
    {
        Token(ColliderDetectorInstance) collider_detector;
    };
};

struct BoxColliderComponent_TMP
{
    static constexpr component_t Type = HashRaw_constexpr(STR(BoxColliderComponent));
    int8 force_update;
    int8 is_allocated;
    Token(Node) scene_node;
    Token(BoxCollider) box_collider;

    struct Dependencies
    {
        Token(ColliderDetectorInstance) detector;
    };

    Dependencies dependencies;

    inline static BoxColliderComponent_TMP build(const Token(Node) p_scene_node)
    {
        return BoxColliderComponent_TMP{0, 0, p_scene_node, tk_bd(BoxCollider), Dependencies{tk_bd(ColliderDetectorInstance)}};
    };

    struct Asset
    {
        v3f half_extend;
    };

    struct AllocationEvent
    {
        Asset asset;
        Token(BoxColliderComponent_TMP) allocated_ressource;

        inline static AllocationEvent build(const Asset& p_asset, const Token(BoxColliderComponent_TMP) p_box_collider)
        {
            return AllocationEvent{p_asset, p_box_collider};
        };
    };

    struct FreeEvent
    {
        Token(BoxColliderComponent_TMP) released_ressource;
    };
};

struct CollisionMiddleware_TMP
{
    PoolIndexed<BoxColliderComponent_TMP> box_collider_components;
    Pool<ColliderDetectorInstance> collider_detectors;

    Vector<BoxColliderComponent_TMP::AllocationEvent> boxcollider_allocation_events;
    Vector<BoxColliderComponent_TMP::FreeEvent> boxcollider_free_events;

    Vector<ColliderDetectorInstance::AllocationEvent> collider_detector_allocation_events;
    Vector<ColliderDetectorInstance::FreeEvent> collider_detector_free_events;

    inline static CollisionMiddleware_TMP allocate()
    {
        return CollisionMiddleware_TMP{PoolIndexed<BoxColliderComponent_TMP>::allocate_default(),      Pool<ColliderDetectorInstance>::allocate(0),
                                       Vector<BoxColliderComponent_TMP::AllocationEvent>::allocate(0), Vector<BoxColliderComponent_TMP::FreeEvent>::allocate(0),
                                       Vector<ColliderDetectorInstance::AllocationEvent>::allocate(0), Vector<ColliderDetectorInstance::FreeEvent>::allocate(0)};
    };

    inline void free(Collision2& p_collision)
    {

#if __DEBUG
        assert_true(this->boxcollider_allocation_events.Size == 0);
        assert_true(this->collider_detector_allocation_events.Size == 0);
#endif

        this->deallocation_step(p_collision);

        // TODO -> this->step
    };

    inline void allocation_step(Collision2& p_collision)
    {
        for (loop(i, 0, this->boxcollider_allocation_events.Size))
        {
            auto& l_event = this->boxcollider_allocation_events.get(i);
            BoxColliderComponent_TMP& l_component = this->box_collider_components.get(l_event.allocated_ressource);
            l_component.box_collider = p_collision.allocate_boxcollider(aabb{v3f_const::ZERO, l_event.asset.half_extend});
            l_component.is_allocated = 1;
            l_component.force_update = 1;
        }
        this->boxcollider_allocation_events.clear();

        for (loop(i, 0, this->collider_detector_allocation_events.Size))
        {
            ColliderDetectorInstance::AllocationEvent& l_event = this->collider_detector_allocation_events.get(i);
            ColliderDetectorInstance& l_collider_detector_ressource = this->collider_detectors.get(l_event.collider_detector);
            Token(BoxCollider) l_box_collider = this->box_collider_components.get(l_collider_detector_ressource.dependencies.box_collider).box_collider;
            l_collider_detector_ressource.collider_detector = p_collision.allocate_colliderdetector(l_box_collider);
            l_collider_detector_ressource.is_allocated = 1;
        }
        this->collider_detector_allocation_events.clear();
    };

    inline void deallocation_step(Collision2& p_collision)
    {
        for (loop(i, 0, this->collider_detector_free_events.Size))
        {
            auto& l_event = this->collider_detector_free_events.get(i);
            ColliderDetectorInstance& l_collider_detector = this->collider_detectors.get(l_event.collider_detector);
            BoxColliderComponent_TMP& l_box_collider = this->box_collider_components.get(l_collider_detector.dependencies.box_collider);
            p_collision.free_colliderdetector(l_box_collider.box_collider, l_collider_detector.collider_detector);
        }
        this->collider_detector_free_events.clear();

        for (loop(i, 0, this->boxcollider_free_events.Size))
        {
            auto& l_event = this->boxcollider_free_events.get(i);
            BoxColliderComponent_TMP& l_component = this->box_collider_components.get(l_event.released_ressource);
            p_collision.free_box_collider(l_component.box_collider);
        }
        this->boxcollider_free_events.clear();
    };
};

struct CollisionMiddleware_AllocationComposition
{
    inline static Token(BoxColliderComponent_TMP)
        allocate_boxcollider_inline(CollisionMiddleware_TMP& p_collision_middleware, const BoxColliderComponent_TMP::Asset& p_input, const Token(Node) & p_scene_node)
    {
        Token(BoxColliderComponent_TMP) l_box_collider = p_collision_middleware.box_collider_components.alloc_element(BoxColliderComponent_TMP::build(p_scene_node));
        p_collision_middleware.boxcollider_allocation_events.push_back_element(BoxColliderComponent_TMP::AllocationEvent::build(p_input, l_box_collider));
        return l_box_collider;
    };

    inline static void free_boxcollider(CollisionMiddleware_TMP& p_collision_middleware, const Token(BoxColliderComponent_TMP) p_box_collider)
    {
            for(loop(i, 0, p_collision_middleware.boxcollider_allocation_events.Size))
            {
                if(tk_eq(p_collision_middleware.boxcollider_allocation_events.get(i).allocated_ressource, p_box_collider))
                {
                    p_collision_middleware.boxcollider_allocation_events.erase_element_at_always(i);
                    return;
                }
            }

            p_collision_middleware.boxcollider_free_events.push_back_element(BoxColliderComponent_TMP::FreeEvent{p_box_collider});
    };
};