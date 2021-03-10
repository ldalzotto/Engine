#pragma once

/*
    Collision Shape used for intersection calculation
*/
struct BoxCollider
{
    int8 enabled;
    transform_pa transform;
    aabb local_box;

    static BoxCollider build_from_local_aabb(const int8 p_enabled, const aabb& p_local_box);
    ;
};

struct Trigger
{
    enum class State
    {
        UNDEFINED = 0,
        TRIGGER_ENTER = 1,
        TRIGGER_STAY = 2,
        TRIGGER_EXIT = 3,
        NONE = 4
    };
};

/*
    State of the trigger intersection between owner BoxCollider and the other.
*/
struct TriggerEvent
{
    TokenT(BoxCollider) other;
    Trigger::State state;

    static TriggerEvent build_default();
    static TriggerEvent build(const TokenT(BoxCollider) p_other, const Trigger::State p_state);
};

/*
    A ColliderDetector indicates that a collision shape is emitting TriggerEvent collision events.
*/
struct ColliderDetector
{
    PoolOfVectorToken<TriggerEvent> collision_events;

    static ColliderDetector build(const PoolOfVectorToken<TriggerEvent> p_collidsion_events);
};

struct CollisionHeap2
{
    PoolIndexed<BoxCollider> box_colliders;
    Pool<TokenT(ColliderDetector)> box_colliders_to_collider_detector;
    PoolIndexed<ColliderDetector> collider_detectors;
    PoolOfVector<TriggerEvent> collider_detectors_events_2;

    static CollisionHeap2 allocate_default();
    void free();

    TokenT(ColliderDetector) allocate_colliderdetector(const TokenT(BoxCollider) p_box_collider);
    void free_colliderdetector(const TokenT(BoxCollider) p_box_collider, const TokenT(ColliderDetector) p_collider_detector);

    TokenT(BoxCollider) allocate_boxcollider(const BoxCollider& p_box_collider);
    void push_boxcollider_transform(TokenT(BoxCollider) p_boxcollider, const transform_pa& p_world_transform);
    void free_boxcollider(const TokenT(BoxCollider) p_box_collider);

    TokenT(ColliderDetector) & get_colliderdetector_from_boxcollider(const TokenT(BoxCollider) p_box_collider);

    Slice<TriggerEvent> get_triggerevents_from_boxcollider(const TokenT(BoxCollider) p_box_collider);
    Slice<TriggerEvent> get_triggerevents_from_colliderdetector(const TokenT(ColliderDetector) p_collider_detector);

    int8 does_boxcollider_have_colliderdetector(const TokenT(BoxCollider) p_box_collider);
};

struct CollisionDetectionStep
{
    /*
        An IntersectionEvent is an internal structure of the CollisionDetectionStep.
        It is the output generated when handling processed Colliders (that can come from either a deletion or an entry in in_colliders_processed).
        An intersectin event is unidirectional. Meaning that intersection is true only from the point of view of the ColliderDetector.
        It can either be an "enter" or "exit" collision.
    */
    struct IntersectionEvent
    {
        TokenT(ColliderDetector) detector;
        TokenT(BoxCollider) other;

        inline static IntersectionEvent build(const TokenT(ColliderDetector) p_detector, const TokenT(BoxCollider) p_other);
        inline int8 equals_intersectionevent(const IntersectionEvent& p_other);
    };

    struct CollisionDetectorDeletionEvent
    {
        TokenT(BoxCollider) collider;
        TokenT(ColliderDetector) detector;

        inline static CollisionDetectorDeletionEvent build(const TokenT(BoxCollider) p_box_collider, const TokenT(ColliderDetector) p_collider_detector);
    };

    Vector<TokenT(BoxCollider)> in_colliders_disabled;
    Vector<TokenT(BoxCollider)> in_colliders_processed;

    Vector<TokenT(BoxCollider)> deleted_colliders_from_last_step;
    Vector<CollisionDetectorDeletionEvent> deleted_collider_detectors_from_last_step;

    Vector<IntersectionEvent> currentstep_enter_intersection_events;
    Vector<IntersectionEvent> currentstep_exit_intersection_events;

    Vector<IntersectionEvent> is_waitingfor_trigger_stay_detector;
    Vector<IntersectionEvent> is_waitingfor_trigger_none_detector;

    Vector<IntersectionEvent> is_waitingfor_trigger_stay_nextframe_detector;
    Vector<IntersectionEvent> is_waitingfor_trigger_none_nextframe_detector;

    inline static CollisionDetectionStep allocate();
    inline void free(CollisionHeap2& p_collision_heap);

    /* A frame of the Collision engine. */
    inline void step(CollisionHeap2& p_collision_heap);

    inline void push_collider_for_process(const TokenT(BoxCollider) p_moved_collider);
    inline void push_collider_for_deletion(const TokenT(BoxCollider) p_collider);
    inline void push_collider_detector_for_deletion(const TokenT(BoxCollider) p_collider, const TokenT(ColliderDetector) p_detector);

  private:
    inline void step_freeingresource_only(CollisionHeap2& p_collision_heap);

    inline void swap_detector_events();

    // When an intersection from the source collider to target occurs
    // If there is already a TriggerEvent event between them, we set to TRIGGER_STAY else, we initialize to TRIGGER_ENTER
    inline void enter_collision(CollisionHeap2& p_collision_heap, const IntersectionEvent& p_intersection_event);

    // We get all ColliderDetector associated to the p_source_collider and check if they have an active state with the involved collider
    // if that's the case, then we invalidate the collision
    inline void exit_collision(CollisionHeap2& p_collision_heap, const IntersectionEvent& p_intersection_event);

    inline void remove_references_to_colliderdetector(CollisionHeap2& p_collision_heap, const TokenT(ColliderDetector) p_collider_detector);

    // /!\ Do not take care of the associated ColliderDetectors.
    inline void remove_references_to_boxcollider(const TokenT(BoxCollider) p_box_collider);

    // Norify all ColliderDetectors with an exit_collision event.
    // TODO -> In the future, we want to partition the space to not notify the entire world
    inline void generate_exit_collision_for_collider(CollisionHeap2& p_collision_heap, const TokenT(BoxCollider) p_box_collider);

    inline void process_deleted_collider_detectors(CollisionHeap2& p_collision_heap);

    /*
        When a collider is deleted :
            -> All associated ColliderDetectors and their references are freed.
            -> All pending processing data that refers to the deleted collider are deleted
            -> All ColliderDetectors are notified that the collider is no more intersecting
    */
    inline void process_deleted_colliders(CollisionHeap2& p_collision_heap);
    inline void process_input_colliders(CollisionHeap2& p_collision_heap);

    inline void remove_intersectionevents_duplicate(Vector<IntersectionEvent>* in_out_intersection_events);

    /*
        Previous step may push the same intersection events. Because a collider detector and it's related box collider may have moved, so they both generate the same intersection event.
    */
    inline void remove_current_step_event_duplicates();
    inline void udpate_triggerstate_from_intersectionevents(CollisionHeap2& p_collision_heap);
    inline void clear_current_step_events();
    inline void set_triggerstate_matchingWith_boxcollider(CollisionHeap2& p_collision_heap, const TokenT(ColliderDetector) p_collision_detector, const TokenT(BoxCollider) p_matched_boxcollider,
                                                          const Trigger::State p_trigger_state);
    inline void udpate_triggerstate_from_lastframe_intersectionevents(CollisionHeap2& p_collision_heap);
    inline void free_deleted_colliders(CollisionHeap2& p_collision_heap);
};