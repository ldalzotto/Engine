#pragma once

#define RessourceAllocationEvent_member_allocated_ressource allocated_ressource
#define RessourceAllocationEvent_member_asset asset

enum class RessourceAllocationType
{
    UNKNOWN = 0,
    INLINE = 1,
    ASSET_DATABASE = 2
};

struct RessourceIdentifiedHeader
{
    RessourceAllocationType allocation_type;
    int8 allocated;
    hash_t id;

    inline static RessourceIdentifiedHeader build_inline_with_id(const hash_t p_id)
    {
        return RessourceIdentifiedHeader{RessourceAllocationType::INLINE, 0, p_id};
    };

    inline static RessourceIdentifiedHeader build_database_with_id(const hash_t p_id)
    {
        return RessourceIdentifiedHeader{RessourceAllocationType::ASSET_DATABASE, 0, p_id};
    };
};

/*
    Utility functions that handle the execution flow of ressource allocation/free event creation
*/
struct RessourceComposition
{

    struct AllocationEventFoundSlot
    {
        struct Nil
        {
            template <class RessourceAllocationEvent_t> inline static void slot(const RessourceAllocationEvent_t& p_allocation_event) {}
        };

        struct FreeAsset
        {
            template <class RessourceAllocationEvent_t> inline static void slot(RessourceAllocationEvent_t& p_allocation_event)
            {
                p_allocation_event.RessourceAllocationEvent_member_asset.free();
            };
        };
    };

    /*
        Iterates over the vector of allocation events and remove those whom ressource token is the same as input,
        calling AllocationEventFoundSlot_t on the way.
    */
    template <class t_RessourceAllocationEventType, class t_RessourceType, class AllocationEventFoundSlot_t>
    inline static void remove_reference_from_allocation_events_explicit(Vector<t_RessourceAllocationEventType>& p_allocation_events, const Token(t_RessourceType) p_ressource,
                                                                        const AllocationEventFoundSlot_t& p_allocation_event_found_slot)
    {
        for (loop_reverse(i, 0, p_allocation_events.sv_get_size()))
        {
            t_RessourceAllocationEventType& l_event = p_allocation_events.get(i);
            if (tk_eq(l_event.RessourceAllocationEvent_member_allocated_ressource, p_ressource))
            {
                p_allocation_event_found_slot.slot(l_event);
                p_allocation_events.erase_element_at_always(i);
            }
        }
    };

    /*
        Default version of remove_reference_from_allocation_events_explicit
    */
    template <class t_RessourceAllocationEventType, class t_RessourceType>
    inline static void remove_reference_from_allocation_events(Vector<t_RessourceAllocationEventType>& p_allocation_events, const Token(t_RessourceType) p_ressource)
    {
        remove_reference_from_allocation_events_explicit(p_allocation_events, p_ressource, AllocationEventFoundSlot::Nil{});
    };

    template <class t_RessourceIdType, class t_RessourceType, class t_RessourceAllocationEventType, class RessourceBuilderFunc_t, class RessourceAllocationEventBuilderFunc_t>
    inline static Token(t_RessourceType)
        allocate_ressource_composition_explicit(PoolHashedCounted<t_RessourceIdType, t_RessourceType>& p_pool_hashed_counted, Vector<t_RessourceAllocationEventType>& p_ressource_allocation_events,
                                                hash_t p_ressource_id, const RessourceBuilderFunc_t& p_ressource_builder_func,
                                                RessourceAllocationEventBuilderFunc_t& p_ressource_allocation_event_builder_func)
    {

        struct __OnRessourceAllocated
        {
            Vector<t_RessourceAllocationEventType>& p_ressource_allocation_events;
            const RessourceAllocationEventBuilderFunc_t& p_ressource_allocation_event_builder_func;

            inline void operator()(const Token(t_RessourceType) p_allocated_ressource) const
            {
                p_ressource_allocation_events.push_back_element(p_ressource_allocation_event_builder_func(p_allocated_ressource));
            };
        } _OnRessourceAllocated = {p_ressource_allocation_events, p_ressource_allocation_event_builder_func};

        return p_pool_hashed_counted.increment_or_allocate_explicit(p_ressource_id, p_ressource_builder_func, _OnRessourceAllocated);
    };

    /*
        The entry point of Ressource deallocation execution flow.
    */
    template <class t_RessourceIdType, class t_RessourceType, class t_RessourceAllocationEventType, class t_RessourceFreeEventType, class RessourceFreeEventBuilderFunc_t,
              class AllocationEventFoundSlot_t>
    inline static int8 free_ressource_composition_explicit(
        PoolHashedCounted<t_RessourceIdType, t_RessourceType>& p_pool_hashed_counted, Vector<t_RessourceAllocationEventType>& p_ressource_allocation_events,
        Vector<t_RessourceFreeEventType>& p_ressource_free_events, const RessourceIdentifiedHeader& p_ressource_header,
        const RessourceFreeEventBuilderFunc_t& p_free_event_builder,    // Builds the RessourceFreeEvent if an event has to be pushed
        const AllocationEventFoundSlot_t& p_allocation_event_found_slot // Slot executed when an existing RessourceAllocationEvent with Ressource reference has been found
    )
    {
        int8 l_return = 0;
        if (p_ressource_header.allocated)
        {
            struct __closure
            {
                int8& l_return;
                Vector<t_RessourceFreeEventType>& p_ressource_free_events;
                const RessourceFreeEventBuilderFunc_t& p_free_event_builder;
            } _closure = {l_return, p_ressource_free_events, p_free_event_builder};

            PoolHashedCounted_decrement_and_deallocate_pool_not_modified_explicit(&p_pool_hashed_counted, p_ressource_header.id, [&_closure](auto p_deallocated_token) {
                _closure.p_ressource_free_events.push_back_element(_closure.p_free_event_builder(p_deallocated_token));
                _closure.l_return = 1;
            });
        }
        else
        {
            struct __closure
            {
                int8& l_return;
                PoolHashedCounted<t_RessourceIdType, t_RessourceType>& p_pool_hashed_counted;
                Vector<t_RessourceAllocationEventType>& p_ressource_allocation_events;
                const AllocationEventFoundSlot_t& p_allocation_event_found_slot;
            } _closure = {l_return, p_pool_hashed_counted, p_ressource_allocation_events, p_allocation_event_found_slot};

            PoolHashedCounted_decrement_and_deallocate_pool_not_modified_explicit(&p_pool_hashed_counted, p_ressource_header.id, [&_closure](auto p_deallocated_token) {
                RessourceComposition::remove_reference_from_allocation_events_explicit(_closure.p_ressource_allocation_events, p_deallocated_token, _closure.p_allocation_event_found_slot);
                _closure.p_pool_hashed_counted.pool.release_element(p_deallocated_token);
                _closure.l_return = 1;
            });
        }
        return l_return;
    };

    /*
        Default version of  free_ressource_composition_explicit
    */
    template <class t_RessourceIdType, class t_RessourceType, class t_RessourceAllocationEventType, class t_RessourceFreeEventType, class RessourceFreeEventBuilder>
    inline static int8 free_ressource_composition(PoolHashedCounted<t_RessourceIdType, t_RessourceType>& p_pool_hashed_counted, Vector<t_RessourceAllocationEventType>& p_ressource_allocation_events,
                                                  Vector<t_RessourceFreeEventType>& p_ressource_free_events, const RessourceIdentifiedHeader& p_ressource_header,
                                                  const RessourceFreeEventBuilder& p_free_event_builder)
    {
        return RessourceComposition::free_ressource_composition_explicit(p_pool_hashed_counted, p_ressource_allocation_events, p_ressource_free_events, p_ressource_header, p_free_event_builder,
                                                                         AllocationEventFoundSlot::Nil{});
    };

    enum class FreeRessourceCompositionReturnCode
    {
        DECREMENTED = 0,
        FREE_EVENT_PUSHED = 1,
        POOL_DEALLOCATION_AWAITING = 2
    };

    template <class t_RessourceType, class t_RessourceAllocationEventType, class t_RessourceFreeEventType>
    inline static FreeRessourceCompositionReturnCode free_ressource_composition_2(PoolHashedCounted<hash_t, t_RessourceType>& p_pool_hashed_counted, Vector<t_RessourceFreeEventType>& p_ressource_free_events,
                                                    Vector<t_RessourceAllocationEventType>& p_ressource_allocation_events, const t_RessourceType& p_material)
    {
        hash_t l_key = p_material.header.id;

        if (p_material.header.allocated)
        {
            auto* l_counted_element = p_pool_hashed_counted.decrement(l_key);
            if (l_counted_element->counter == 0)
            {
                p_ressource_free_events.push_back_element(t_RessourceFreeEventType{l_counted_element->token});
                // We don't remove the pool because it handles by the free event
                p_pool_hashed_counted.CountMap.erase_key_nothashed(l_key);
                return FreeRessourceCompositionReturnCode::FREE_EVENT_PUSHED;
            }
        }
        else
        {
            auto* l_counted_element = p_pool_hashed_counted.decrement(l_key);
            if (l_counted_element->counter == 0)
            {
                for (loop_reverse(i, 0, p_ressource_allocation_events.Size))
                {
                    auto& l_event = p_ressource_allocation_events.get(i);
                    if (tk_eq(l_event.allocated_ressource, l_counted_element->token))
                    {
                        l_event.asset.free();
                        p_ressource_allocation_events.erase_element_at_always(i);
                    }
                }

                p_pool_hashed_counted.CountMap.erase_key_nothashed(l_key);
                // p_pool_hashed_counted.remove_element(l_key, l_counted_element->token);
                return FreeRessourceCompositionReturnCode::POOL_DEALLOCATION_AWAITING;
            }
        }

        return FreeRessourceCompositionReturnCode::DECREMENTED;
    };

    template <class t_RessourceIdType, class t_RessourceType, class t_DynamicDependency, class DynamicDependencyTokenAllocationSlot_t>
    inline static Token(Slice<t_DynamicDependency>)
        allocate_dynamic_dependencies(PoolHashedCounted<t_RessourceIdType, t_RessourceType>& p_source_pool_hashed_counted, PoolOfVector<t_DynamicDependency>& p_dynamic_dependency_pool,
                                      const hash_t p_initial_ressource_id, const DynamicDependencyTokenAllocationSlot_t& p_dynamic_depenedency_allocation_slot)
    {
        if (p_source_pool_hashed_counted.has_key_nothashed(p_initial_ressource_id))
        {
            return tk_bf(Slice<t_DynamicDependency>, p_source_pool_hashed_counted.CountMap.get_value(p_initial_ressource_id)->token);
        }
        else
        {
            return p_dynamic_depenedency_allocation_slot();
        }
    };

    /*
        If the the Ressource is allocated from the asset database, this function try to retrieve asset from database by it's id and map it to the asset value
    */
    template <class t_RessourceAssetType>
    inline static void retrieve_ressource_asset_from_database_if_necessary(AssetDatabase& p_asset_database, const RessourceIdentifiedHeader& p_ressource_header, t_RessourceAssetType* in_out_asset)
    {
        switch (p_ressource_header.allocation_type)
        {
        case RessourceAllocationType::INLINE:
            break;
        case RessourceAllocationType::ASSET_DATABASE:
            *in_out_asset = in_out_asset->build_from_binary(p_asset_database.get_asset_blob(p_ressource_header.id));
            break;
        default:
            abort();
        }
    };

  private:
    // /!\ Deallocation is not complete, as the token is not removed from the pool
    // This is to be able to retrieve the value for deferred operations
    template <class t_RessourceIdType, class t_RessourceType, class t_DeallocationFunc>
    inline static void PoolHashedCounted_decrement_and_deallocate_pool_not_modified_explicit(PoolHashedCounted<t_RessourceIdType, t_RessourceType>* thiz, const t_RessourceIdType& p_key,
                                                                                             const t_DeallocationFunc& p_deallocation_slot)
    {
        auto* l_counted_element = thiz->decrement(p_key);
        if (l_counted_element->counter == 0)
        {
            thiz->CountMap.erase_key_nothashed(p_key);
            p_deallocation_slot(l_counted_element->token);
        }
    };
};