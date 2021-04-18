#pragma once

/*
    Utility functions that handle the execution flow of resource allocation/free event creation
*/
struct ResourceAlgorithm
{
    template <class ResourceType, class ResourceInlineAllocationEventType, class ResourceDatabaseAllocationEventType, class ResourceFreeEventType, class ResourceReleaseFunc>
    inline static void decrement_or_release_resource_by_token_v3(PoolHashedCounted<hash_t, ResourceType>& p_pool_hashed_counted, Vector<ResourceFreeEventType>& p_resource_free_events,
                                                                  Vector<ResourceInlineAllocationEventType>& p_resource_inline_allocation_events,
                                                                  Vector<ResourceDatabaseAllocationEventType>& p_resource_database_allocation_events, const Token<ResourceType> p_resource_token,
                                                                  const ResourceReleaseFunc& p_resource_release_func)
    {
        ResourceType& l_resource = p_pool_hashed_counted.pool.get(p_resource_token);
        hash_t l_hashed_id = p_pool_hashed_counted.CountMap.hash_key(l_resource.header.id);

        if (l_resource.header.allocated)
        {
            auto* l_counted_element = p_pool_hashed_counted.decrement(l_hashed_id);
            if (l_counted_element->counter == 0)
            {
                p_resource_free_events.push_back_element(ResourceFreeEventType{l_counted_element->token});
                // We don't remove the pool because it is handles by the free event
                p_pool_hashed_counted.CountMap.erase_key(l_hashed_id);
                p_resource_release_func(l_resource);
            }
        }
        else
        {
            auto* l_counted_element = p_pool_hashed_counted.decrement(l_hashed_id);
            if (l_counted_element->counter == 0)
            {
                for (loop_reverse(i, 0, p_resource_inline_allocation_events.Size))
                {
                    auto& l_event = p_resource_inline_allocation_events.get(i);
                    if (token_equals(l_event.allocated_resource, l_counted_element->token))
                    {
                        l_event.asset.free();
                        p_resource_inline_allocation_events.erase_element_at_always(i);
                    }
                }

                for (loop_reverse(i, 0, p_resource_database_allocation_events.Size))
                {
                    auto& l_event = p_resource_database_allocation_events.get(i);
                    if (token_equals(l_event.allocated_resource, l_counted_element->token))
                    {
                        p_resource_database_allocation_events.erase_element_at_always(i);
                    }
                }

                p_pool_hashed_counted.CountMap.erase_key(l_hashed_id);
                p_resource_release_func(l_resource);
                p_pool_hashed_counted.pool.release_element(p_resource_token);
            }
        }
    };

    template <class ResourceType> inline static Token<ResourceType> increment_resource(PoolHashedCounted<hash_t, ResourceType>& p_resources, const hash_t p_id)
    {
        return p_resources.increment_nothashed(p_id);
    };

    template <class ResourceType, class ResourceAllocationEventType>
    inline static Token<ResourceType> push_resource_to_be_allocated_database_v2(PoolHashedCounted<hash_t, ResourceType>& p_resources, const ResourceType& p_resource,
                                                                      Vector<ResourceAllocationEventType>& p_resource_allocation_events, const hash_t p_id)
    {
        Token<ResourceType> l_resource = p_resources.push_back_element_nothashed(p_id, p_resource);
        p_resource_allocation_events.push_back_element(ResourceAllocationEventType{p_id, l_resource});
        return l_resource;
    };

    template <class ResourceType, class ResourceAssetType, class ResourceAllocationEventType>
    inline static Token<ResourceType> push_resource_to_be_allocated_inline_v2(PoolHashedCounted<hash_t, ResourceType>& p_resources,const ResourceType& p_resource,
                                                                                  Vector<ResourceAllocationEventType>& p_resource_allocation_events, const hash_t p_id, const ResourceAssetType& p_asset)
    {
        Token<ResourceType> l_resource = p_resources.push_back_element_nothashed(p_id, p_resource);
        p_resource_allocation_events.push_back_element(ResourceAllocationEventType{p_asset, l_resource});
        return l_resource;
    };

    template <class ResourceType, class ResourceDatabaseEventAllocator>
    inline static Token<ResourceType> allocate_or_increment_inline_v2(PoolHashedCounted<hash_t, ResourceType>& p_resources, const hash_t p_id,  const ResourceDatabaseEventAllocator& p_resource_database_event_allocator)
    {
        if (p_resources.has_key_nothashed(p_id))
        {
#if __DEBUG
            assert_true(p_resources.get_nothashed(p_id).header.allocation_type == ResourceAllocationType::INLINE);
#endif
            return ResourceAlgorithm::increment_resource(p_resources, p_id);
        }
        else
        {
            return p_resource_database_event_allocator(ResourceIdentifiedHeader{ResourceAllocationType::INLINE, 0, p_id});
        }
    };

    template <class ResourceType, class ResourceDatabaseEventAllocator>
    inline static Token<ResourceType> allocate_or_increment_database_v2(PoolHashedCounted<hash_t, ResourceType>& p_resources, const hash_t p_id,
                                                                         const ResourceDatabaseEventAllocator& p_resource_database_event_allocator)
    {
        if (p_resources.has_key_nothashed(p_id))
        {
#if __DEBUG
            assert_true(p_resources.get_nothashed(p_id).header.allocation_type == ResourceAllocationType::ASSET_DATABASE);
#endif
            return ResourceAlgorithm::increment_resource(p_resources, p_id);
        }
        else
        {
            return p_resource_database_event_allocator(ResourceIdentifiedHeader{ResourceAllocationType::ASSET_DATABASE, 0, p_id});
        }
    };

    template <class ResourceType, class ResourceDatabaseAllocationEventType, class ResourceInlineAllocationEventType, class ResourceAllocationFunc>
    inline static void allocation_step(PoolHashedCounted<hash_t, ResourceType>& p_resources, Vector<ResourceDatabaseAllocationEventType>& p_resource_database_allocation_events,
                                       Vector<ResourceInlineAllocationEventType>& p_resource_inline_allocation_events, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database,
                                       const ResourceAllocationFunc& p_resource_allocation_func)
    {
        for (loop(i, 0, p_resource_database_allocation_events.Size))
        {
            auto& l_event = p_resource_database_allocation_events.get(i);
            ResourceType& l_resource = p_resources.pool.get(l_event.allocated_resource);
            auto l_mesh_asset = ResourceType::Asset::build_from_binary(p_asset_database.get_asset_blob(p_database_connection, l_event.id));
            auto l_value = ResourceType::Asset::Value::build_from_asset(l_mesh_asset);
            p_resource_allocation_func(l_resource, l_value);
            l_resource.header.allocated = 1;
            l_mesh_asset.free();
        }
        p_resource_database_allocation_events.clear();

        for (loop(i, 0, p_resource_inline_allocation_events.Size))
        {
            auto& l_event = p_resource_inline_allocation_events.get(i);

            ResourceType& l_resource = p_resources.pool.get(l_event.allocated_resource);
            auto l_value = ResourceType::Asset::Value::build_from_asset(l_event.asset);
            p_resource_allocation_func(l_resource, l_value);
            l_resource.header.allocated = 1;
            l_event.asset.free();
        }
        p_resource_inline_allocation_events.clear();
    };

    template <class ResourceType, class ResourceFreeEventType, class ResourceDeallocationFunc>
    inline static void deallocation_step(PoolHashedCounted<hash_t, ResourceType>& p_pool_hashed_counted, Vector<ResourceFreeEventType>& p_resource_free_events,
                                         const ResourceDeallocationFunc& p_resource_deallocation_func)
    {
        for (loop(i, 0, p_resource_free_events.Size))
        {
            auto& l_event = p_resource_free_events.get(i);
            ResourceType& l_resource = p_pool_hashed_counted.pool.get(l_event.allocated_resource);
            p_resource_deallocation_func(l_resource);
            p_pool_hashed_counted.pool.release_element(l_event.allocated_resource);
        }
        p_resource_free_events.clear();
    }
};