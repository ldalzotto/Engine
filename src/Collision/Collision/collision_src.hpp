

inline BoxCollider BoxCollider::build_from_local_aabb(const int8 p_enabled, const aabb& p_local_box)
{
    BoxCollider l_box_collider;
    l_box_collider.enabled = p_enabled;
    l_box_collider.local_box = p_local_box;
    return l_box_collider;
};

inline TriggerEvent TriggerEvent::build_default()
{
    return TriggerEvent{token_build_default<BoxCollider>(), Trigger::State::UNDEFINED};
};

inline TriggerEvent TriggerEvent::build(const Token<BoxCollider> p_other, const Trigger::State p_state)
{
    return TriggerEvent{p_other, p_state};
};

inline ColliderDetector ColliderDetector::build(const PoolOfVectorToken<TriggerEvent> p_collidsion_events)
{
    return ColliderDetector{p_collidsion_events};
};

inline CollisionHeap2 CollisionHeap2::allocate_default()
{
    return CollisionHeap2{PoolIndexed<BoxCollider>::allocate_default(), Pool<Token<ColliderDetector>>::allocate(0), PoolIndexed<ColliderDetector>::allocate_default(),
                          PoolOfVector<TriggerEvent>::allocate_default()};
};

inline void CollisionHeap2::free()
{
#if __DEBUG
    assert_true(!this->box_colliders.has_allocated_elements());
    assert_true(!this->box_colliders_to_collider_detector.has_allocated_elements());
    assert_true(!this->collider_detectors.has_allocated_elements());
    assert_true(!this->collider_detectors_events_2.has_allocated_elements());
#endif

    this->box_colliders.free();
    this->box_colliders_to_collider_detector.free();
    this->collider_detectors.free();
    this->collider_detectors_events_2.free();
};

inline Token<ColliderDetector> CollisionHeap2::allocate_colliderdetector(const Token<BoxCollider> p_box_collider)
{

#if __DEBUG
    // Cannot attach multiple collider detector to a collider for now
    assert_true(!this->does_boxcollider_have_colliderdetector(p_box_collider));
#endif

    PoolOfVectorToken<TriggerEvent> l_trigger_events = this->collider_detectors_events_2.alloc_vector();
    Token<ColliderDetector> l_collider_detector_token = this->collider_detectors.alloc_element(ColliderDetector::build(l_trigger_events));
    this->get_colliderdetector_from_boxcollider(p_box_collider) = l_collider_detector_token;
    return l_collider_detector_token;
};

inline void CollisionHeap2::free_colliderdetector(const Token<BoxCollider> p_box_collider, const Token<ColliderDetector> p_collider_detector){

    {ColliderDetector& l_collider_detector = this->collider_detectors.get(p_collider_detector);
this->collider_detectors_events_2.release_vector(l_collider_detector.collision_events);
this->collider_detectors.release_element(p_collider_detector);
}

{
    this->get_colliderdetector_from_boxcollider(p_box_collider) = token_build_default<ColliderDetector>();
    // this->box_colliders_to_collider_detector.release_element(token_cast_p(Token<ColliderDetector>, p_box_collider));
}
}
;

inline Token<BoxCollider> CollisionHeap2::allocate_boxcollider(const BoxCollider& p_box_collider)
{
    Token<BoxCollider> l_box_collider_index = this->box_colliders.alloc_element(p_box_collider);
    this->box_colliders_to_collider_detector.alloc_element(token_build_default<ColliderDetector>());
    return l_box_collider_index;
};

inline void CollisionHeap2::push_boxcollider_transform(Token<BoxCollider> p_boxcollider, const transform_pa& p_world_transform)
{
    BoxCollider& l_boxcollider = this->box_colliders.get(p_boxcollider);
    l_boxcollider.transform = p_world_transform;
};

inline void CollisionHeap2::free_boxcollider(const Token<BoxCollider> p_box_collider)
{
    Token<ColliderDetector>& l_collider_detector = this->get_colliderdetector_from_boxcollider(p_box_collider);
    if (!token_equals(l_collider_detector, token_build<ColliderDetector>(-1)))
    {
        this->free_colliderdetector(p_box_collider, l_collider_detector);
    }
    this->box_colliders.release_element(p_box_collider);
    this->box_colliders_to_collider_detector.release_element(token_build_from<Token<ColliderDetector>>( p_box_collider));
};

inline Token<ColliderDetector> & CollisionHeap2::get_colliderdetector_from_boxcollider(const Token<BoxCollider> p_box_collider)
{
    return this->box_colliders_to_collider_detector.get(token_build_from<Token<ColliderDetector>>( p_box_collider));
};

inline Slice<TriggerEvent> CollisionHeap2::get_triggerevents_from_boxcollider(const Token<BoxCollider> p_box_collider)
{
    if (this->does_boxcollider_have_colliderdetector(p_box_collider))
    {
        Token<ColliderDetector>& l_collider_detextor = this->get_colliderdetector_from_boxcollider(p_box_collider);
        return this->collider_detectors_events_2.get_vector(this->collider_detectors.get(l_collider_detextor).collision_events);
    }
    return Slice<TriggerEvent>::build_default();
};

inline Slice<TriggerEvent> CollisionHeap2::get_triggerevents_from_colliderdetector(const Token<ColliderDetector> p_collider_detector)
{
    return this->collider_detectors_events_2.get_vector(this->collider_detectors.get(p_collider_detector).collision_events);
};

inline int8 CollisionHeap2::does_boxcollider_have_colliderdetector(const Token<BoxCollider> p_box_collider)
{
    if (!this->box_colliders_to_collider_detector.is_element_free(token_build_from<Token<ColliderDetector>>( p_box_collider)))
    {
        if (!token_equals(this->get_colliderdetector_from_boxcollider(p_box_collider), token_build<ColliderDetector>(-1)))
        {
            return 1;
        }
    };
    return 0;
};

inline CollisionDetectionStep::IntersectionEvent CollisionDetectionStep::IntersectionEvent::build(const Token<ColliderDetector> p_detector, const Token<BoxCollider> p_other)
{
    return IntersectionEvent{p_detector, p_other};
};

inline int8 CollisionDetectionStep::IntersectionEvent::equals_intersectionevent(const IntersectionEvent& p_other)
{
    return token_equals(this->detector, p_other.detector) && token_equals(this->other, p_other.other);
};

inline CollisionDetectionStep::CollisionDetectorDeletionEvent CollisionDetectionStep::CollisionDetectorDeletionEvent::build(const Token<BoxCollider> p_box_collider,
                                                                                                                            const Token<ColliderDetector> p_collider_detector)
{
    return CollisionDetectorDeletionEvent{p_box_collider, p_collider_detector};
};

inline CollisionDetectionStep CollisionDetectionStep::allocate()
{
    return CollisionDetectionStep{
        Vector<Token<BoxCollider>>::allocate(0), Vector<Token<BoxCollider>>::allocate(0), Vector<Token<BoxCollider>>::allocate(0), Vector<CollisionDetectorDeletionEvent>::allocate(0),
        Vector<IntersectionEvent>::allocate(0),  Vector<IntersectionEvent>::allocate(0),  Vector<IntersectionEvent>::allocate(0),  Vector<IntersectionEvent>::allocate(0),
        Vector<IntersectionEvent>::allocate(0),  Vector<IntersectionEvent>::allocate(0),
    };
};

inline void CollisionDetectionStep::free(CollisionHeap2& p_collision_heap)
{
    // To free pending resources
    this->step_freeingresource_only(p_collision_heap);

#if __DEBUG
    assert_true(this->in_colliders_disabled.get_size() == 0);
    assert_true(this->in_colliders_processed.get_size() == 0);
    assert_true(this->deleted_colliders_from_last_step.get_size() == 0);
    assert_true(this->deleted_collider_detectors_from_last_step.get_size() == 0);
    assert_true(this->currentstep_enter_intersection_events.get_size() == 0);
    assert_true(this->currentstep_exit_intersection_events.get_size() == 0);
    assert_true(this->is_waitingfor_trigger_stay_detector.get_size() == 0);
    assert_true(this->is_waitingfor_trigger_stay_nextframe_detector.get_size() == 0);
    assert_true(this->is_waitingfor_trigger_none_detector.get_size() == 0);
    assert_true(this->is_waitingfor_trigger_none_nextframe_detector.get_size() == 0);
#endif

    this->in_colliders_disabled.free();
    this->in_colliders_processed.free();
    this->deleted_colliders_from_last_step.free();
    this->deleted_collider_detectors_from_last_step.free();
    this->currentstep_enter_intersection_events.free();
    this->currentstep_exit_intersection_events.free();
    this->is_waitingfor_trigger_stay_detector.free();
    this->is_waitingfor_trigger_stay_nextframe_detector.free();
    this->is_waitingfor_trigger_none_detector.free();
    this->is_waitingfor_trigger_none_nextframe_detector.free();
};

/*
A *frame* of the collision engine.
    1* Swap "per frame" buffers.

    2* Execute ColliderDetectors destroy event.
       By :
         1/ Cleaning all references of the deleted Detectors from all CollisionDetectionStep calculation data.
            To avoid manipulating Detectors that have already been deleted.
         2/ Freeing detectors from the heap.

    3* Execute Colliders destroy event.
       By :
         1/ Cleaning all references of the attached Detectors from all CollisionDetectionStep calculation data.
         2/ Cleaning all references of the deleted Colliders from all CollisionDetectionStep calculation data.
         3/ Freeing Collides from the heap.

    4* Process moved colliders.
       By :
         1/ Calculating intersections for every possible combinations with the moved Colliders that have a ColliderDetector.
         2/ Pushing enter and exit IntersectionEvents for further processing.
       The Colliders processing may induce duplicate intersections. eg: two Colliders with ColliderDetectors that have moved and intersect will generate 4 identical IntersectionEvents.
       IntersectionEvents generation is always done from the point of view of a single ColliderDetector. This is to handle the case where a ColliderDetector that is not moving (thus, not processed)
       stille generates events when another collider intersect with it.

    5* Removing IntersectionEvents duplicates.

    6* Update the TriggerEvent state based on last frame IntersectionEvents.
        When TriggerEvent states are updated in 7*, they may also generated deferred IntersectionEvents that will be processed the next frame.
        For example, when a TriggerState value is set to TRIGGER_ENTER, then if nothing happens, on the next frame, the TriggerState will automaticaly be set to
        TRIGGER_STAY.

    7* Update the TriggerEvent state based on generated IntersectionEvents.

    8* 9* Clear resrouces.
*/
inline void CollisionDetectionStep::step(CollisionHeap2& p_collision_heap)
{
    this->swap_detector_events(); // 1*

    this->process_deleted_collider_detectors(p_collision_heap); // 2*
    this->process_deleted_colliders(p_collision_heap);          // 3*

    this->process_input_colliders(p_collision_heap); // 4*
    this->remove_current_step_event_duplicates();    // 5*

    this->udpate_triggerstate_from_lastframe_intersectionevents(p_collision_heap); // 6*
    this->udpate_triggerstate_from_intersectionevents(p_collision_heap);           // 7*

    this->clear_current_step_events(); // 8*

    this->free_deleted_colliders(p_collision_heap); // 9*
};

inline void CollisionDetectionStep::push_collider_for_process(const Token<BoxCollider> p_moved_collider)
{
    this->in_colliders_processed.push_back_element(p_moved_collider);
};

inline void CollisionDetectionStep::push_collider_for_deletion(const Token<BoxCollider> p_collider)
{
    this->deleted_colliders_from_last_step.push_back_element(p_collider);
};

inline void CollisionDetectionStep::push_collider_detector_for_deletion(const Token<BoxCollider> p_collider, const Token<ColliderDetector> p_detector)
{
    this->deleted_collider_detectors_from_last_step.push_back_element(CollisionDetectorDeletionEvent::build(p_collider, p_detector));
};

inline void CollisionDetectionStep::step_freeingresource_only(CollisionHeap2& p_collision_heap)
{
    this->process_deleted_collider_detectors(p_collision_heap);
    this->free_deleted_colliders(p_collision_heap);
};

inline void CollisionDetectionStep::swap_detector_events(){{Vector<IntersectionEvent> l_tmp = this->is_waitingfor_trigger_stay_detector;
this->is_waitingfor_trigger_stay_detector = this->is_waitingfor_trigger_stay_nextframe_detector;
this->is_waitingfor_trigger_stay_nextframe_detector = l_tmp;
}
{
    Vector<IntersectionEvent> l_tmp = this->is_waitingfor_trigger_none_detector;
    this->is_waitingfor_trigger_none_detector = this->is_waitingfor_trigger_none_nextframe_detector;
    this->is_waitingfor_trigger_none_nextframe_detector = l_tmp;
}
}
;

// When an intersection from the source collider to target occurs
// If there is already a TriggerEvent event between them, we set to TRIGGER_STAY else, we initialize to TRIGGER_ENTER
inline void CollisionDetectionStep::enter_collision(CollisionHeap2& p_collision_heap, const IntersectionEvent& p_intersection_event)
{
    PoolOfVectorToken<TriggerEvent> l_collider_triggerevents_nestedvector = p_collision_heap.collider_detectors.get(p_intersection_event.detector).collision_events;
    Slice<TriggerEvent> l_collider_triggerevents = p_collision_heap.collider_detectors_events_2.get_vector(l_collider_triggerevents_nestedvector);
    bool l_trigger_event_found = false;
    for (loop(i, 0, l_collider_triggerevents.Size))
    {
        TriggerEvent& l_collider_trigger_event = l_collider_triggerevents.get(i);
        if (token_equals(l_collider_trigger_event.other, p_intersection_event.other))
        {
            l_collider_trigger_event.state = Trigger::State::TRIGGER_STAY;
            l_trigger_event_found = true;
            break;
        }
    }

    if (!l_trigger_event_found)
    {
        p_collision_heap.collider_detectors_events_2.element_push_back_element(l_collider_triggerevents_nestedvector, TriggerEvent::build(p_intersection_event.other, Trigger::State::TRIGGER_ENTER));

        this->is_waitingfor_trigger_stay_nextframe_detector.push_back_element(p_intersection_event);
    }
};

// We get all ColliderDetector associated to the p_source_collider and check if they have an active state with the involved collider
// if that's the case, then we invalidate the collision
inline void CollisionDetectionStep::exit_collision(CollisionHeap2& p_collision_heap, const IntersectionEvent& p_intersection_event)
{
    Slice<TriggerEvent> l_collider_triggerevents = p_collision_heap.collider_detectors_events_2.get_vector(p_collision_heap.collider_detectors.get(p_intersection_event.detector).collision_events);

    for (loop(i, 0, l_collider_triggerevents.Size))
    {
        TriggerEvent& l_trigger_event = l_collider_triggerevents.get(i);
        if (token_equals(l_trigger_event.other, p_intersection_event.other))
        {
            if (l_trigger_event.state != Trigger::State::NONE)
            {
                l_trigger_event.state = Trigger::State::TRIGGER_EXIT;
                // on next step, collision event will be deleted
                this->is_waitingfor_trigger_none_nextframe_detector.push_back_element(p_intersection_event);
            }
        }
    }
};

inline void CollisionDetectionStep::remove_references_to_colliderdetector(CollisionHeap2& p_collision_heap, const Token<ColliderDetector> p_collider_detector)
{
    this->is_waitingfor_trigger_stay_detector.erase_if([&](const CollisionDetectionStep::IntersectionEvent& l_intsersrection_event){
        return token_equals(l_intsersrection_event.detector, p_collider_detector);
    });

    this->is_waitingfor_trigger_none_detector.erase_if([&](const CollisionDetectionStep::IntersectionEvent& l_intsersrection_event){
      return token_equals(l_intsersrection_event.detector, p_collider_detector);
    });

    this->in_colliders_processed.erase_if([&](const Token<BoxCollider> l_disabled_collider){
        Token<ColliderDetector>& l_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(l_disabled_collider);
      if (!token_equals(l_collider_detector, token_build<ColliderDetector>(-1)) && token_equals(l_collider_detector, p_collider_detector))
      {
          return true;
      }
      return false;
    });
};

// /!\ Do not take care of the associated ColliderDetectors.
inline void CollisionDetectionStep::remove_references_to_boxcollider(const Token<BoxCollider> p_box_collider)
{
    this->in_colliders_processed.erase_if([&](const Token<BoxCollider> l_collider) {
        return token_equals(l_collider, p_box_collider);
    });

    this->is_waitingfor_trigger_stay_detector.erase_if([&](const CollisionDetectionStep::IntersectionEvent& l_intsersrection_event) {
        return token_equals(l_intsersrection_event.other, p_box_collider);
    });

    this->is_waitingfor_trigger_none_detector.erase_if([&](const CollisionDetectionStep::IntersectionEvent& l_intsersrection_event) {
        return token_equals(l_intsersrection_event.other, p_box_collider);
    });
};

// Norify all ColliderDetectors with an exit_collision event.
// TODO -> In the future, we want to partition the space to not notify the entire world
inline void CollisionDetectionStep::generate_exit_collision_for_collider(CollisionHeap2& p_collision_heap, const Token<BoxCollider> p_box_collider)
{
    p_collision_heap.box_colliders.foreach ([&](const Token<BoxCollider> p_compared_boxcollider_token, const BoxCollider& p_compared_boxcollider) {
        if (p_compared_boxcollider.enabled)
        {
            Token<ColliderDetector>& l_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(p_compared_boxcollider_token);
            if (!token_equals(l_collider_detector, token_build<ColliderDetector>(-1)))
            {
                this->currentstep_exit_intersection_events.push_back_element(IntersectionEvent::build(l_collider_detector, p_box_collider));
            }
        }
    });
};

inline void CollisionDetectionStep::process_deleted_collider_detectors(CollisionHeap2& p_collision_heap)
{
    for (vector_loop(&this->deleted_collider_detectors_from_last_step, i))
    {
        CollisionDetectorDeletionEvent& l_deletion_event = this->deleted_collider_detectors_from_last_step.get(i);
        this->remove_references_to_colliderdetector(p_collision_heap, l_deletion_event.detector);
        p_collision_heap.free_colliderdetector(l_deletion_event.collider, l_deletion_event.detector);
    }
    this->deleted_collider_detectors_from_last_step.clear();
};

inline void CollisionDetectionStep::process_deleted_colliders(CollisionHeap2& p_collision_heap)
{
    // dereferencing ColliderDetectors and Colliders.
    for (vector_loop(&this->deleted_colliders_from_last_step, i))
    {
        Token<BoxCollider>& l_deleted_collider = this->deleted_colliders_from_last_step.get(i);
        Token<ColliderDetector>& l_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(l_deleted_collider);
        if (!token_equals(l_collider_detector, token_build<ColliderDetector>(-1)))
        {
            this->remove_references_to_colliderdetector(p_collision_heap, l_collider_detector);
            p_collision_heap.free_colliderdetector(l_deleted_collider, l_collider_detector);
        }

        p_collision_heap.box_colliders.get(l_deleted_collider).enabled = false;
        this->remove_references_to_boxcollider(l_deleted_collider);
    }

    // Once the collision step is cleaned up, we can generate exit_collision event
    // to notify all other ColliderDetectors that a collider has gone
    // Some exit events will be false positive (event sended but there was no collision at the first plane),
    // but that's not a problem as it will be ignored by the udpate_triggerstate_from_intersectionevents step.
    for (vector_loop(&this->deleted_colliders_from_last_step, i))
    {
        Token<BoxCollider>& l_disabled_box_collider_token = this->deleted_colliders_from_last_step.get(i);
        this->generate_exit_collision_for_collider(p_collision_heap, l_disabled_box_collider_token);
    }
};

inline void CollisionDetectionStep::process_input_colliders(CollisionHeap2& p_collision_heap)
{
    for (loop(i, 0, this->in_colliders_processed.Size))
    {
        Token<BoxCollider>& l_left_collider_token = this->in_colliders_processed.get(i);
        Token<ColliderDetector>& l_left_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(l_left_collider_token);

        // If the processed collider have a collider detector, we calculate intersection with other BoxColliders
        // then push collision_event according to the intersection result
        if (!token_equals(l_left_collider_detector, token_build<ColliderDetector>(-1)))
        {
            BoxCollider& l_left_collider = p_collision_heap.box_colliders.get(l_left_collider_token);
            if (l_left_collider.enabled)
            {
                obb l_left_projected = l_left_collider.local_box.add_position_rotation(l_left_collider.transform);

                // TODO -> In the future, we want to avoid to query the world
                p_collision_heap.box_colliders.foreach ([&](const Token<BoxCollider> l_right_collider_token, BoxCollider& l_right_collider) {
                    // Avoid self test
                    if (!token_equals(l_left_collider_token, l_right_collider_token))
                    {
                        if (l_right_collider.enabled)
                        {
                            obb l_right_projected = l_left_collider.local_box.add_position_rotation(l_right_collider.transform);

                            if (l_left_projected.overlap2(l_right_projected))
                            {
                                this->currentstep_enter_intersection_events.push_back_element(IntersectionEvent::build(l_left_collider_detector, l_right_collider_token));

                                Token<ColliderDetector>& l_right_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(l_right_collider_token);
                                if (!token_equals(l_right_collider_detector, token_build<ColliderDetector>(-1)))
                                {
                                    this->currentstep_enter_intersection_events.push_back_element(IntersectionEvent::build(l_right_collider_detector, l_left_collider_token));
                                }
                            }
                            else
                            {
                                this->currentstep_exit_intersection_events.push_back_element(IntersectionEvent::build(l_left_collider_detector, l_right_collider_token));

                                Token<ColliderDetector>& l_right_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(l_right_collider_token);
                                if (!token_equals(l_right_collider_detector, token_build<ColliderDetector>(-1)))
                                {
                                    this->currentstep_exit_intersection_events.push_back_element(IntersectionEvent::build(l_right_collider_detector, l_left_collider_token));
                                }
                            }
                        }
                    }
                });
            }
        }
        // If the processed collider doesn't have a collider detector, we get all other collider detectors and test collision
        // then push collision_event according to the intersection result
        else
        {
            BoxCollider& l_left_collider = p_collision_heap.box_colliders.get(l_left_collider_token);
            if (l_left_collider.enabled)
            {
                obb l_left_projected = l_left_collider.local_box.add_position_rotation(l_left_collider.transform);

                // TODO -> In the future, we want to avoid to query the world
                p_collision_heap.box_colliders.foreach ([&](const Token<BoxCollider> l_right_collider_token, BoxCollider& l_right_collider) {
                    if (l_right_collider.enabled)
                    {
                        Token<ColliderDetector>& l_right_collider_detector = p_collision_heap.get_colliderdetector_from_boxcollider(l_right_collider_token);
                        if (!token_equals(l_right_collider_detector, token_build<ColliderDetector>(-1)))
                        {
                            obb l_right_projected = l_left_collider.local_box.add_position_rotation(l_right_collider.transform);

                            if (l_left_projected.overlap2(l_right_projected))
                            {
                                this->currentstep_enter_intersection_events.push_back_element(IntersectionEvent::build(l_right_collider_detector, l_left_collider_token));
                            }
                            else
                            {
                                this->currentstep_exit_intersection_events.push_back_element(IntersectionEvent::build(l_right_collider_detector, l_left_collider_token));
                            }
                        }
                    }
                });
            }
        }
    }

    this->in_colliders_processed.clear();
};

inline void CollisionDetectionStep::remove_intersectionevents_duplicate(Vector<IntersectionEvent>* in_out_intersection_events)
{
    for (loop(i, 0, in_out_intersection_events->Size))
    {
        IntersectionEvent& l_left_intersection_event = in_out_intersection_events->get(i);
        for (loop(j, i + 1, in_out_intersection_events->Size))
        {
            IntersectionEvent& l_right_intersection_event = in_out_intersection_events->get(j);

            if (l_left_intersection_event.equals_intersectionevent(l_right_intersection_event))
            {
                in_out_intersection_events->erase_element_at_always(j);
            }
        }
    }
};

/*
    Previous step may push the same intersection events. Because a collider detector and it's related box collider may have moved, so they both generate the same intersection event.
*/
inline void CollisionDetectionStep::remove_current_step_event_duplicates()
{
    this->remove_intersectionevents_duplicate(&this->currentstep_enter_intersection_events);
    this->remove_intersectionevents_duplicate(&this->currentstep_exit_intersection_events);
};

inline void CollisionDetectionStep::udpate_triggerstate_from_intersectionevents(CollisionHeap2& p_collision_heap)
{
    for (loop(i, 0, this->currentstep_enter_intersection_events.Size))
    {
        this->enter_collision(p_collision_heap, this->currentstep_enter_intersection_events.get(i));
    }

    for (loop(i, 0, this->currentstep_exit_intersection_events.Size))
    {
        this->exit_collision(p_collision_heap, this->currentstep_exit_intersection_events.get(i));
    }
};

inline void CollisionDetectionStep::clear_current_step_events()
{
    this->currentstep_enter_intersection_events.clear();
    this->currentstep_exit_intersection_events.clear();
};

inline void CollisionDetectionStep::set_triggerstate_matchingWith_boxcollider(CollisionHeap2& p_collision_heap, const Token<ColliderDetector> p_collision_detector,
                                                                              const Token<BoxCollider> p_matched_boxcollider, const Trigger::State p_trigger_state)
{
    ColliderDetector& l_collider_detector = p_collision_heap.collider_detectors.get(p_collision_detector);
    Slice<TriggerEvent> l_events = p_collision_heap.collider_detectors_events_2.get_vector(l_collider_detector.collision_events);
    for (loop(i, 0, l_events.Size))
    {
        TriggerEvent& l_trigger_event = l_events.get(i);
        if (token_equals(l_trigger_event.other, p_matched_boxcollider))
        {
            l_trigger_event.state = p_trigger_state;
        }
    }
};

inline void CollisionDetectionStep::udpate_triggerstate_from_lastframe_intersectionevents(CollisionHeap2& p_collision_heap)
{
    for (vector_loop(&this->is_waitingfor_trigger_stay_detector, i))
    {
        IntersectionEvent& l_intersection_event = this->is_waitingfor_trigger_stay_detector.get(i);
        this->set_triggerstate_matchingWith_boxcollider(p_collision_heap, l_intersection_event.detector, l_intersection_event.other, Trigger::State::TRIGGER_STAY);
    }

    for (vector_loop(&this->is_waitingfor_trigger_none_detector, i))
    {
        IntersectionEvent& l_intersection_event = this->is_waitingfor_trigger_none_detector.get(i);
        this->set_triggerstate_matchingWith_boxcollider(p_collision_heap, l_intersection_event.detector, l_intersection_event.other, Trigger::State::NONE);
    }

    this->is_waitingfor_trigger_stay_detector.clear();
    this->is_waitingfor_trigger_none_detector.clear();
};

inline void CollisionDetectionStep::free_deleted_colliders(CollisionHeap2& p_collision_heap)
{
    for (vector_loop(&this->deleted_colliders_from_last_step, i))
    {
        p_collision_heap.free_boxcollider(this->deleted_colliders_from_last_step.get(i));
    }
    this->deleted_colliders_from_last_step.clear();
};

/*
    The Collision engine provides a way to hook when a 3D geometric shape enters in collision with anoter one.
    It is currently a "brute-force" implementation. Meaning that Colliders are not spatially indexed.
*/
struct Collision2
{

    CollisionHeap2 collision_heap;
    CollisionDetectionStep collision_detection_step;

    inline static Collision2 allocate()
    {
        return Collision2{CollisionHeap2::allocate_default(), CollisionDetectionStep::allocate()};
    };

    inline void free()
    {
        this->step();

        this->collision_detection_step.free(this->collision_heap);
        this->collision_heap.free();
    };

    inline void step()
    {
        this->collision_detection_step.step(this->collision_heap);
    };

    inline Token<BoxCollider> allocate_boxcollider(const aabb& p_local_box)
    {
        return this->collision_heap.allocate_boxcollider(BoxCollider::build_from_local_aabb(true, p_local_box));
    };

    inline void on_collider_moved(const Token<BoxCollider> p_moved_collider, const transform_pa& p_world_transform)
    {
        this->collision_heap.push_boxcollider_transform(p_moved_collider, p_world_transform);
        this->collision_detection_step.push_collider_for_process(p_moved_collider);
    };

    inline void free_collider(const Token<BoxCollider> p_moved_collider)
    {
        this->collision_detection_step.push_collider_for_deletion(p_moved_collider);
    };

    inline BoxCollider get_box_collider_copy(const Token<BoxCollider> p_box_collider)
    {
        return this->collision_heap.box_colliders.get(p_box_collider);
    };

    inline int8 is_collider_queued_for_detection(const Token<BoxCollider> p_box_collider)
    {
        for (loop(i, 0, this->collision_detection_step.in_colliders_processed.Size))
        {
            if (token_value(this->collision_detection_step.in_colliders_processed.get(i)) == token_value(p_box_collider))
            {
                return 1;
            }
        }
        return 0;
    };

    inline Slice<TriggerEvent> get_collision_events(const Token<ColliderDetector> p_collider_detector)
    {
        return this->collision_heap.get_triggerevents_from_colliderdetector(p_collider_detector);
    };

    inline Token<ColliderDetector> allocate_colliderdetector(const Token<BoxCollider> p_box_collider)
    {
        return this->collision_heap.allocate_colliderdetector(p_box_collider);
    };

    inline void free_colliderdetector(const Token<BoxCollider> p_collider, const Token<ColliderDetector> p_collider_detector)
    {
        this->collision_detection_step.push_collider_detector_for_deletion(p_collider, p_collider_detector);
    };
};