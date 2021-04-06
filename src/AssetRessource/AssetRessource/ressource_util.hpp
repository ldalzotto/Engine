#pragma once

/*
    Utility functions that handle the execution flow of ressource allocation/free event creation
*/
struct RessourceComposition
{
    enum class FreeRessourceCompositionReturnCode
    {
        DECREMENTED = 0,
        FREE_EVENT_PUSHED = 1,
        POOL_DEALLOCATION_AWAITING = 2
    };

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