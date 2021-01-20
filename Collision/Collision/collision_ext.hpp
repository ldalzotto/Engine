#pragma once



struct CollisionHandle
{
    void* handle;

    inline static CollisionHandle build_default()
    {
        return CollisionHandle{ nullptr };
    };

    void allocate();
    void free();
    void step();
};

struct BoxColliderHandle
{
    uimax handle;

    inline static BoxColliderHandle build_default() { return BoxColliderHandle{ cast(uimax, -1) }; }
    inline void reset() { *this = build_default(); };

    void allocate(CollisionHandle p_collision, const aabb& p_local_aabb);
    void free(CollisionHandle p_collision);
    void on_collider_moved(CollisionHandle p_collision, const transform_pa& p_world_transform);
};

struct ITriggerEvent
{
    BoxColliderHandle other;
    Trigger::State state = Trigger::State::UNDEFINED;
};


struct ColliderDetectorHandle
{
    uimax handle;
    BoxColliderHandle collider;

    inline static ColliderDetectorHandle build_default()
    {
        return ColliderDetectorHandle{
            cast(uimax, -1), BoxColliderHandle::build_default()
        };
    };

    inline void reset()
    {
        *this = build_default();
    }

    void allocate(CollisionHandle p_collision, BoxColliderHandle p_collider);
    void free(CollisionHandle p_collision);

    Slice<ITriggerEvent> get_collision_events(CollisionHandle p_collision);
};



inline void CollisionHandle::allocate()
{
    Collision2::allocate(cast(Collision2**, &this->handle));
};

inline void CollisionHandle::free()
{
    Collision2::free(cast(Collision2**, &this->handle));
};

inline void CollisionHandle::step()
{
    cast(Collision2*, this->handle)->step();
};

using Collision2Ext = Collision2::ExternalInterface;

inline void BoxColliderHandle::allocate(CollisionHandle p_collision, const aabb& p_local_aabb)
{
    this->handle = tk_v(Collision2Ext::allocate_boxcollider(cast(Collision2*, p_collision.handle), p_local_aabb));
};

inline void BoxColliderHandle::free(CollisionHandle p_collision)
{
    Collision2Ext::free_collider(cast(Collision2*, p_collision.handle), Token(BoxCollider) { this->handle });
};

inline void BoxColliderHandle::on_collider_moved(CollisionHandle p_collision, const transform_pa& p_world_transform)
{
    Collision2Ext::on_collider_moved(cast(Collision2*, p_collision.handle), Token(BoxCollider) { this->handle }, p_world_transform);
};


inline void ColliderDetectorHandle::allocate(CollisionHandle p_collision, BoxColliderHandle p_collider)
{
    this->handle = tk_v(Collision2Ext::allocate_colliderdetector(cast(Collision2*, p_collision.handle), tk_b(BoxCollider, p_collider.handle)));
    this->collider = p_collider;
};

inline void ColliderDetectorHandle::free(CollisionHandle p_collision)
{
    Collision2Ext::free_colliderdetector(
        cast(Collision2*, p_collision.handle),
        tk_b(BoxCollider, this->collider.handle),
        tk_b(ColliderDetector, this->handle));
};

inline Slice<ITriggerEvent> ColliderDetectorHandle::get_collision_events(CollisionHandle p_collision)
{
    return slice_cast<ITriggerEvent>(
        cast(Collision2*, p_collision.handle)->collision_heap.get_triggerevents_from_colliderdetector(tk_b(ColliderDetector, this->handle))
        .build_asint8()
        );
};
