#pragma once

/*
    Utility functions that handle the execution flow of ressource allocation/free event creation
*/
struct RessourceAlgorithm
{
    // TODO -> remove this :(
    enum class FreeRessourceCompositionReturnCode
    {
        DECREMENTED = 0,
        FREE_EVENT_PUSHED = 1,
        POOL_DEALLOCATION_AWAITING = 2
    };

    // TODO -> remove this :(
    template <class t_RessourceType, class t_RessourceAllocationEventType, class t_RessourceFreeEventType>
    inline static FreeRessourceCompositionReturnCode free_ressource_composition_2(PoolHashedCounted<hash_t, t_RessourceType>& p_pool_hashed_counted,
                                                                                  Vector<t_RessourceFreeEventType>& p_ressource_free_events,
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
                    if (token_equals(l_event.allocated_ressource, l_counted_element->token))
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

    // TODO -> update the PoolHashedCounted for hashed key value call
    template <class RessourceType, class RessourceInlineAllocationEventType, class RessourceDatabaseAllocationEventType, class RessourceFreeEventType, class RessourceReleaseFunc>
    inline static void decrement_or_release_ressource_by_token_v3(PoolHashedCounted<hash_t, RessourceType>& p_pool_hashed_counted, Vector<RessourceFreeEventType>& p_ressource_free_events,
                                                                  Vector<RessourceInlineAllocationEventType>& p_ressource_inline_allocation_events,
                                                                  Vector<RessourceDatabaseAllocationEventType>& p_ressource_database_allocation_events, const Token<RessourceType> p_ressource_token, const RessourceReleaseFunc& p_ressource_release_func)
    {
        RessourceType& l_ressource = p_pool_hashed_counted.pool.get(p_ressource_token);
        hash_t l_id = l_ressource.header.id;

        if (l_ressource.header.allocated)
        {
            auto* l_counted_element = p_pool_hashed_counted.decrement(l_id);
            if (l_counted_element->counter == 0)
            {
                p_ressource_free_events.push_back_element(RessourceFreeEventType{l_counted_element->token});
                // We don't remove the pool because it is handles by the free event
                p_pool_hashed_counted.CountMap.erase_key_nothashed(l_id);
                p_ressource_release_func(l_ressource);
            }
        }
        else
        {
            auto* l_counted_element = p_pool_hashed_counted.decrement(l_id);
            if (l_counted_element->counter == 0)
            {
                for (loop_reverse(i, 0, p_ressource_inline_allocation_events.Size))
                {
                    auto& l_event = p_ressource_inline_allocation_events.get(i);
                    if (token_equals(l_event.allocated_ressource, l_counted_element->token))
                    {
                        l_event.asset.free();
                        p_ressource_inline_allocation_events.erase_element_at_always(i);
                    }
                }

                for (loop_reverse(i, 0, p_ressource_database_allocation_events.Size))
                {
                    auto& l_event = p_ressource_database_allocation_events.get(i);
                    if (token_equals(l_event.allocated_ressource, l_counted_element->token))
                    {
                        p_ressource_database_allocation_events.erase_element_at_always(i);
                    }
                }

                p_pool_hashed_counted.CountMap.erase_key_nothashed(l_id);
                p_ressource_release_func(l_ressource);
                p_pool_hashed_counted.pool.release_element(p_ressource_token);
            }
        }
    };

    template <class RessourceType> inline static Token<RessourceType> increment_ressource(PoolHashedCounted<hash_t, RessourceType>& p_ressources, const hash_t p_id)
    {
        return p_ressources.increment(p_id);
    };

    template <class RessourceType, class RessourceAssetType, class RessourceInlineAllocationEventType>
    inline static Token<RessourceType> push_ressource_to_be_allocated_inline(PoolHashedCounted<hash_t, RessourceType>& p_ressources,
                                                                             Vector<RessourceInlineAllocationEventType>& p_ressource_allocation_events, const hash_t p_id,
                                                                             const RessourceAssetType& p_asset)
    {
        Token<RessourceType> l_ressource = p_ressources.push_back_element(p_id, RessourceType{RessourceIdentifiedHeader{RessourceAllocationType::INLINE, 0, p_id}});
        p_ressource_allocation_events.push_back_element(RessourceInlineAllocationEventType{p_asset, l_ressource});
        return l_ressource;
    };

    template <class RessourceType, class RessourceAssetType, class RessourceInlineAllocationEventType>
    inline static Token<RessourceType> allocate_or_increment_inline(PoolHashedCounted<hash_t, RessourceType>& p_ressources, Vector<RessourceInlineAllocationEventType>& p_ressource_allocation_events,
                                                                    const hash_t p_id, const RessourceAssetType& p_asset)
    {
        if (p_ressources.has_key_nothashed(p_id))
        {
#if __DEBUG
            assert_true(p_ressources.get(p_id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return RessourceAlgorithm::increment_ressource(p_ressources, p_id);
        }
        else
        {
            return RessourceAlgorithm::push_ressource_to_be_allocated_inline(p_ressources, p_ressource_allocation_events, p_id, p_asset);
        }
    };

    template <class RessourceType, class RessourceDatabaseAllocationEventType>
    inline static Token<RessourceType> push_ressource_to_be_allocated_database(PoolHashedCounted<hash_t, RessourceType>& p_ressources,
                                                                               Vector<RessourceDatabaseAllocationEventType>& p_ressource_allocation_events, const hash_t p_id)
    {
        Token<RessourceType> l_ressource = p_ressources.push_back_element(p_id, RessourceType{RessourceIdentifiedHeader{RessourceAllocationType::ASSET_DATABASE, 0, p_id}});
        p_ressource_allocation_events.push_back_element(RessourceDatabaseAllocationEventType{p_id, l_ressource});
        return l_ressource;
    };

    template <class RessourceType, class RessourceDatabaseAllocationEventType>
    inline static Token<RessourceType> allocate_or_increment_database(PoolHashedCounted<hash_t, RessourceType>& p_ressources,
                                                                      Vector<RessourceDatabaseAllocationEventType>& p_ressource_allocation_events, const hash_t p_id)
    {
        if (p_ressources.has_key_nothashed(p_id))
        {
#if __DEBUG
            assert_true(p_ressources.get(p_id).header.allocation_type == RessourceAllocationType::ASSET_DATABASE);
#endif
            return RessourceAlgorithm::increment_ressource(p_ressources, p_id);
        }
        else
        {
            return RessourceAlgorithm::push_ressource_to_be_allocated_database(p_ressources, p_ressource_allocation_events, p_id);
        }
    };

    template <class RessourceType, class RessourceDatabaseAllocationEventType, class RessourceInlineAllocationEventType, class RessourceAllocationFunc>
    inline static void allocation_step(PoolHashedCounted<hash_t, RessourceType>& p_ressources, Vector<RessourceDatabaseAllocationEventType>& p_ressource_database_allocation_events,
                                       Vector<RessourceInlineAllocationEventType>& p_ressource_inline_allocation_events, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database,
                                       const RessourceAllocationFunc& p_ressource_allocation_func)
    {
        for (loop(i, 0, p_ressource_database_allocation_events.Size))
        {
            auto& l_event = p_ressource_database_allocation_events.get(i);
            RessourceType& l_ressource = p_ressources.pool.get(l_event.allocated_ressource);
            RessourceType::Asset l_mesh_asset = RessourceType::Asset::build_from_binary(p_asset_database.get_asset_blob(p_database_connection, l_event.id));
            RessourceType::Asset::Value l_value = RessourceType::Asset::Value::build_from_asset(l_mesh_asset);
            p_ressource_allocation_func(l_ressource, l_value);
            l_ressource.header.allocated = 1;
            l_mesh_asset.free();
        }
        p_ressource_database_allocation_events.clear();

        for (loop(i, 0, p_ressource_inline_allocation_events.Size))
        {
            auto& l_event = p_ressource_inline_allocation_events.get(i);

            RessourceType& l_ressource = p_ressources.pool.get(l_event.allocated_ressource);
            RessourceType::Asset::Value l_value = RessourceType::Asset::Value::build_from_asset(l_event.asset);
            p_ressource_allocation_func(l_ressource, l_value);
            l_ressource.header.allocated = 1;
            l_event.asset.free();
        }
        p_ressource_inline_allocation_events.clear();
    };

    template <class RessourceType, class RessourceFreeEventType, class RessourceDeallocationFunc>
    inline static void deallocation_step(PoolHashedCounted<hash_t, RessourceType>& p_pool_hashed_counted, Vector<RessourceFreeEventType>& p_ressource_free_events,
                                         const RessourceDeallocationFunc& p_ressource_deallocation_func)
    {
        for (loop(i, 0, p_ressource_free_events.Size))
        {
            auto& l_event = p_ressource_free_events.get(i);
            RessourceType& l_ressource = p_pool_hashed_counted.pool.get(l_event.allocated_ressource);
            p_ressource_deallocation_func(l_ressource);
            p_pool_hashed_counted.pool.release_element(l_event.allocated_ressource);
        }
        p_ressource_free_events.clear();
    }

    /*
    template <class t_RessourceType> inline static Token<t_RessourceType> allocate_or_increment_inline(const hash_t p_id, )
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.meshes.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource_inline(p_inline_input.id, p_inline_input.asset);
        }
    };

     */

    /*
        If the the Ressource is allocated from the asset database, this function try to retrieve asset from database by it's id and map it to the asset value
    */
    /*
     * TODO -> we want to remove this function. What we want to be able to do is to not check allocation type but to have an array of asset database task that are executing in chunk, before the actual
     *         allocation. In fact, this check is already done by the ressource allocation function called.
     */
    template <class t_RessourceAssetType>
    inline static void retrieve_ressource_asset_from_database_if_necessary(DatabaseConnection& p_database_conncetion, AssetDatabase& p_asset_database,
                                                                           const RessourceIdentifiedHeader& p_ressource_header, t_RessourceAssetType* in_out_asset)
    {
        switch (p_ressource_header.allocation_type)
        {
        case RessourceAllocationType::INLINE:
            break;
        case RessourceAllocationType::ASSET_DATABASE:
            *in_out_asset = in_out_asset->build_from_binary(p_asset_database.get_asset_blob(p_database_conncetion, p_ressource_header.id));
            break;
        default:
            abort();
        }
    };
};