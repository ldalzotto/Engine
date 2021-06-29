#pragma once

enum class ResourceAllocationType
{
    UNKNOWN = 0,
    INLINE = 1,
    ASSET_DATABASE = 2
};

struct ResourceIdentifiedHeader
{
    ResourceAllocationType allocation_type;
    int8 allocated;
    hash_t id;

    inline static ResourceIdentifiedHeader build_inline_with_id(const hash_t p_id)
    {
        return ResourceIdentifiedHeader{ResourceAllocationType::INLINE, 0, p_id};
    };

    inline static ResourceIdentifiedHeader build_database_with_id(const hash_t p_id)
    {
        return ResourceIdentifiedHeader{ResourceAllocationType::ASSET_DATABASE, 0, p_id};
    };
};

template <class _Resource> struct iResource_Token
{
    // Empty, for tokenization
    using sTokenValue = iResource_Token<_Resource>;
    using sToken = Token<sTokenValue>;

    using t_HashPoolToken = typename PoolHashedCounted<hash_t, _Resource>::sToken;
    using t_HashPoolTokenValue = typename PoolHashedCounted<hash_t, _Resource>::sTokenValue;
};

template <class _Resource> struct iResource
{
    using t_Resource_Token = typename iResource_Token<_Resource>::sToken;
    using t_Resource_TokenValue = typename iResource_Token<_Resource>::sTokenValue;

    _Resource& resource;

    inline ResourceIdentifiedHeader& get_header()
    {
        return this->resource.header;
    };

    inline static _Resource build_resource(const ResourceIdentifiedHeader& p_header, const t_Resource_Token p_resource_token)
    {
        return _Resource{p_header, p_resource_token};
    };
};

template <class _DatabaseAllocationEvent> struct iDatabaseAllocationEvent
{
    _DatabaseAllocationEvent& database_allocation_event;

    using _ResourceValue = typename _DatabaseAllocationEvent::_ResourceValue;
    using t_Resource_Token = typename iResource<_ResourceValue>::t_Resource_Token;

    inline t_Resource_Token get_allocated_resource()
    {
        return this->database_allocation_event.allocated_resource;
    };

    inline hash_t get_id()
    {
        return this->database_allocation_event.id;
    };
};

template <class _InlineAllocationEvent> struct iInlineAllocationEvent
{
    _InlineAllocationEvent& inline_allocation_event;

    using _ResourceValue = typename _InlineAllocationEvent::_ResourceValue;
    using t_Resource_Token = typename iResource<_ResourceValue>::t_Resource_Token;

    using _Asset = typename _InlineAllocationEvent::_Asset;
    using _AssetValue = typename _InlineAllocationEvent::_AssetValue;

    inline t_Resource_Token get_allocated_resource()
    {
        return this->inline_allocation_event.allocated_resource;
    };

    inline _Asset get_asset()
    {
        return this->inline_allocation_event.asset;
    };
};

template <class _FreeEvent> struct iFreeEvent
{
    _FreeEvent& free_event;

    using _ResourceValue = typename _FreeEvent::_ResourceValue;
    using t_Resource_Token = typename iResource<_ResourceValue>::t_Resource_Token;

    inline t_Resource_Token get_allocated_resource()
    {
        return this->free_event.allocated_resource;
    };
};

template <class _ResourceUnit> struct iResourceUnit
{
    _ResourceUnit& resource_unit;

    using _Resource = typename _ResourceUnit::_Resource;
    using _ResourceValue = typename _ResourceUnit::_ResourceValue;
    using t_Resource_Token = typename iResource<_ResourceValue>::t_Resource_Token;
    using t_Resource_TokenValue = typename iResource<_ResourceValue>::t_Resource_TokenValue;

    using _AssetBinary = typename _ResourceUnit::_AssetBinary;
    using _AssetBinaryValue = typename _ResourceUnit::_AssetBinaryValue;

    using _Asset = typename _ResourceUnit::_Asset;
    using _AssetValue = typename _ResourceUnit::_AssetValue;

    using _ResourcePool = typename _ResourceUnit::_ResourcePool;
    using _ResourcePoolValue = typename _ResourceUnit::_ResourcePoolValue;

    using _DatabaseAllocationEvents = typename _ResourceUnit::_DatabaseAllocationEvents;
    using _DatabaseAllocationEventsValue = typename _ResourceUnit::_DatabaseAllocationEventsValue;

    using _DatabaseAllocationEvent = typename _ResourceUnit::_DatabaseAllocationEvent;
    using _DatabaseAllocationEventValue = typename _ResourceUnit::_DatabaseAllocationEventValue;

    using _InlineAllocationEvents = typename _ResourceUnit::_InlineAllocationEvents;
    using _InlineAllocationEventsValue = typename _ResourceUnit::_InlineAllocationEventsValue;

    using _InlineAllocationEvent = typename _ResourceUnit::_InlineAllocationEvent;
    using _InlineAllocationEventValue = typename _ResourceUnit::_InlineAllocationEventValue;

    using _FreeEvents = typename _ResourceUnit::_FreeEvents;
    using _FreeEventsValue = typename _ResourceUnit::_FreeEventsValue;

    using _FreeEvent = typename _ResourceUnit::_FreeEvent;
    using _FreeEventValue = typename _ResourceUnit::_FreeEventValue;

    inline PoolHashedCounted<hash_t, _ResourceValue>& get_pool()
    {
        return this->resource_unit.get_pool();
    };

    inline _DatabaseAllocationEvents get_database_allocation_events()
    {
        return this->resource_unit.get_database_allocation_events();
    };

    inline _InlineAllocationEvents get_inline_allocation_events()
    {
        return this->resource_unit.get_inline_allocation_events();
    };

    inline _FreeEvents get_free_events()
    {
        return this->resource_unit.get_free_events();
    };

    inline _ResourceValue& get(const t_Resource_Token p_token)
    {
        return this->get_pool().get_from_pool(token_build_from<PoolHashedCounted<hash_t, _ResourceValue>::sTokenValue>(p_token));
    };

    template <class ResourceAllocationFunc>
    inline void allocation_step(DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database, const ResourceAllocationFunc& p_resource_allocation_func)
    {
        _DatabaseAllocationEvents __database_allocation_events = this->get_database_allocation_events();
        iVector<_DatabaseAllocationEventsValue> l_allocation_events = __database_allocation_events.to_ivector();
        for (loop(i, 0, l_allocation_events.get_size()))
        {
            _DatabaseAllocationEvent __l_event = l_allocation_events.get(i);
            iDatabaseAllocationEvent<_DatabaseAllocationEventValue> l_event = iDatabaseAllocationEvent<_DatabaseAllocationEventValue>{__l_event};

            _AssetBinaryValue l_asset_binary = _AssetBinaryValue{p_asset_database.get_asset_blob(p_database_connection, l_event.get_id())};
            _AssetValue l_asset_value = _ResourceValue::Asset::Value::build_from_asset(l_asset_binary);

            _Resource __resource = this->get(l_event.get_allocated_resource());
            p_resource_allocation_func(__resource, l_asset_value);
            __resource.header.allocated = 1;
            l_asset_binary.free();
        }
        l_allocation_events.clear();

        _InlineAllocationEvents __inline_allocation_events = this->get_inline_allocation_events();
        iVector<_InlineAllocationEventsValue> l_inline_allocation_events = __inline_allocation_events.to_ivector();
        for (loop(i, 0, l_inline_allocation_events.get_size()))
        {
            _InlineAllocationEvent __l_event = l_inline_allocation_events.get(i);
            iInlineAllocationEvent<_InlineAllocationEventValue> l_event = iInlineAllocationEvent<_InlineAllocationEventValue>{__l_event};

            _AssetValue l_asset_value = _ResourceValue::Asset::Value::build_from_asset(l_event.get_asset());

            _Resource __resource = this->get(l_event.get_allocated_resource());
            p_resource_allocation_func(__resource, l_asset_value);
            __resource.header.allocated = 1;
            l_event.get_asset().free();
        }
        l_inline_allocation_events.clear();
    };

    template <class ResourceDeallocationFunc> inline void deallocation_step(const ResourceDeallocationFunc& p_resource_deallocation_func)
    {
        _FreeEvents __resource_free_events = this->get_free_events();
        iVector<_FreeEventsValue> l_resource_free_events = __resource_free_events.to_ivector();

        for (loop(i, 0, l_resource_free_events.get_size()))
        {
            _FreeEvent __l_event = l_resource_free_events.get(i);
            iFreeEvent<_FreeEventValue> l_event = iFreeEvent<_FreeEventValue>{__l_event};
            t_Resource_Token l_allocated_resource = l_event.get_allocated_resource();

            _Resource l_resource = this->get(l_allocated_resource);
            p_resource_deallocation_func(l_resource);
            this->_release_pool_resource(l_event.get_allocated_resource());
        }
        l_resource_free_events.clear();
    };

    template <class ResourceValueBuilderFunc>
    inline t_Resource_Token allocate_or_increment_inline(const hash_t p_id, const _AssetBinaryValue& p_asset_binary, const ResourceValueBuilderFunc& p_resource_builder)
    {
        PoolHashedCounted<hash_t, _ResourceValue>& l_resource_pool = this->get_pool();
        if (l_resource_pool.has_key_nothashed(p_id))
        {
#if __DEBUG
            _Resource __l_resource = l_resource_pool.get_nothashed(p_id);
            iResource<_ResourceValue> l_resource = iResource<_ResourceValue>{__l_resource};
            assert_true(l_resource.get_header().allocation_type == ResourceAllocationType::INLINE);
#endif
            return token_build_from<t_Resource_TokenValue>(l_resource_pool.increment_nothashed(p_id));
        }
        else
        {
            // iResource<>
            _ResourceValue l_resource = p_resource_builder(p_id);
            return this->push_resource_to_be_allocated_inline(l_resource, p_id, p_asset_binary);
        }
    };

    template <class ResourceValueBuilderFunc> inline t_Resource_Token allocate_or_increment_database(const hash_t p_id, const ResourceValueBuilderFunc& p_resource_builder)
    {
        PoolHashedCounted<hash_t, _ResourceValue>& l_resource_pool = this->get_pool();
        if (l_resource_pool.has_key_nothashed(p_id))
        {
#if __DEBUG
            assert_true(l_resource_pool.get_nothashed(p_id).header.allocation_type == ResourceAllocationType::ASSET_DATABASE);
#endif
            return token_build_from<t_Resource_TokenValue>(l_resource_pool.increment_nothashed(p_id));
        }
        else
        {
            _ResourceValue l_resource = p_resource_builder(p_id);
            return this->push_resource_to_be_allocated_database(l_resource, p_id);
        }
    };

    template <class ResourceReleaseFunc> inline void decrement_or_release_resource_by_token(const t_Resource_Token p_resource_token, const ResourceReleaseFunc& p_resource_release_func)
    {
        PoolHashedCounted<hash_t, _ResourceValue>& l_resource_pool = this->get_pool();
        _Resource __l_resource = this->get(p_resource_token);
        iResource<_ResourceValue> l_resource = iResource<_ResourceValue>{__l_resource};
        hash_t l_hashed_id = l_resource_pool.CountMap.hash_key(l_resource.get_header().id);

        if (l_resource.get_header().allocated)
        {
            auto* l_counted_element = l_resource_pool.decrement(l_hashed_id);
            if (l_counted_element->counter == 0)
            {
                _FreeEvents __l_free_events = this->get_free_events();
                iVector<_FreeEventsValue> l_free_events = __l_free_events.to_ivector();
                l_free_events.push_back_element(_FreeEventValue{token_build_from<t_Resource_TokenValue>(l_counted_element->token)});
                // We don't remove the pool because it is handles by the free event
                l_resource_pool.CountMap.erase_key(l_hashed_id);
                p_resource_release_func(__l_resource);
            }
        }
        else
        {
            auto* l_counted_element = l_resource_pool.decrement(l_hashed_id);
            if (l_counted_element->counter == 0)
            {
                _InlineAllocationEvents __l_inline_allocation_events = this->get_inline_allocation_events();
                iVector<_InlineAllocationEventsValue> l_inline_allocation_events = __l_inline_allocation_events.to_ivector();
                // iInlineAllocationEvent<_InlineAllocationEvents> l_inline_allocation_events = iInlineAllocationEvent<_InlineAllocationEvents>{__l_inline_allocation_events};
                for (loop_reverse(i, 0, l_inline_allocation_events.get_size()))
                {
                    _InlineAllocationEvent __l_event = l_inline_allocation_events.get(i);
                    iInlineAllocationEvent<_InlineAllocationEventValue> l_event = iInlineAllocationEvent<_InlineAllocationEventValue>{__l_event};
                    if (token_equals(l_event.get_allocated_resource(), l_counted_element->token))
                    {
                        l_event.get_asset().free();
                        l_inline_allocation_events.erase_element_at_always(i);
                    }
                }

                _DatabaseAllocationEvents __l_database_allocation_events = this->get_database_allocation_events();
                iVector<_DatabaseAllocationEventsValue> l_database_allocation_events = iVector<_DatabaseAllocationEventsValue>{__l_database_allocation_events};

                for (loop_reverse(i, 0, l_database_allocation_events.get_size()))
                {
                    _DatabaseAllocationEvent __l_event = l_database_allocation_events.get(i);
                    iDatabaseAllocationEvent<_DatabaseAllocationEventValue> l_event = iDatabaseAllocationEvent<_DatabaseAllocationEventValue>{__l_event};
                    if (token_equals(l_event.get_allocated_resource(), l_counted_element->token))
                    {
                        l_database_allocation_events.erase_element_at_always(i);
                    }
                }

                l_resource_pool.CountMap.erase_key(l_hashed_id);
                p_resource_release_func(__l_resource);
                this->_release_pool_resource(p_resource_token);
            }
        }
    };

  private:
    inline t_Resource_Token push_resource_to_be_allocated_inline(const _ResourceValue& p_resource, const hash_t p_id, const _AssetBinaryValue& p_asset_binary)
    {
        PoolHashedCounted<hash_t, _ResourceValue>& l_resource_pool = this->get_pool();
        t_Resource_Token l_resource = token_build_from<t_Resource_TokenValue>(l_resource_pool.push_back_element_nothashed(p_id, p_resource));
        _InlineAllocationEvents __l_inline_allocation_events = this->get_inline_allocation_events();
        iVector<_InlineAllocationEventsValue> l_inline_allocation_events = __l_inline_allocation_events.to_ivector();
        l_inline_allocation_events.push_back_element(_InlineAllocationEventValue{p_asset_binary, l_resource});
        return l_resource;
    };

    inline t_Resource_Token push_resource_to_be_allocated_database(const _ResourceValue& p_resource, const hash_t p_id)
    {
        PoolHashedCounted<hash_t, _ResourceValue>& l_resource_pool = this->get_pool();
        t_Resource_Token l_resource = token_build_from<t_Resource_TokenValue>(l_resource_pool.push_back_element_nothashed(p_id, p_resource));
        _DatabaseAllocationEvents __l_database_allocation_events = this->get_database_allocation_events();
        iVector<_DatabaseAllocationEventsValue> l_database_allocation_events = __l_database_allocation_events.to_ivector();
        l_database_allocation_events.push_back_element(_DatabaseAllocationEventValue{p_id, l_resource});
        return l_resource;
    };

    inline void _release_pool_resource(const t_Resource_Token p_token)
    {
        this->get_pool().pool.release_element(token_build_from<PoolHashedCounted<hash_t, _ResourceValue>::sTokenValue>(p_token));
    };
};

#define RESOURCEUNIT_DECLARE_TYPES(ResourceValue)                                                                                                                                                      \
    using _Resource = ResourceValue&;                                                                                                                                                                  \
    using _ResourceValue = ResourceValue;                                                                                                                                                              \
                                                                                                                                                                                                       \
    using _AssetBinary = ResourceValue::Asset&;                                                                                                                                                        \
    using _AssetBinaryValue = ResourceValue::Asset;                                                                                                                                                    \
                                                                                                                                                                                                       \
    using _Asset = ResourceValue::Asset::Value&;                                                                                                                                                       \
    using _AssetValue = ResourceValue::Asset::Value;                                                                                                                                                   \
                                                                                                                                                                                                       \
    using _ResourcePool = Pool<ResourceValue>&;                                                                                                                                                        \
    using _ResourcePoolValue = Pool<ResourceValue>;                                                                                                                                                    \
                                                                                                                                                                                                       \
    using _InlineAllocationEvents = Vector<ResourceValue::InlineAllocationEvent>&;                                                                                                                     \
    using _InlineAllocationEventsValue = Vector<ResourceValue::InlineAllocationEvent>;                                                                                                                 \
                                                                                                                                                                                                       \
    using _InlineAllocationEvent = ResourceValue::InlineAllocationEvent&;                                                                                                                              \
    using _InlineAllocationEventValue = ResourceValue::InlineAllocationEvent;                                                                                                                          \
                                                                                                                                                                                                       \
    using _DatabaseAllocationEvents = Vector<ResourceValue::DatabaseAllocationEvent>&;                                                                                                                 \
    using _DatabaseAllocationEventsValue = Vector<ResourceValue::DatabaseAllocationEvent>;                                                                                                             \
                                                                                                                                                                                                       \
    using _DatabaseAllocationEvent = ResourceValue::DatabaseAllocationEvent&;                                                                                                                          \
    using _DatabaseAllocationEventValue = ResourceValue::DatabaseAllocationEvent;                                                                                                                      \
                                                                                                                                                                                                       \
    using _FreeEvents = Vector<ResourceValue::FreeEvent>&;                                                                                                                                             \
    using _FreeEventsValue = Vector<ResourceValue::FreeEvent>;                                                                                                                                         \
                                                                                                                                                                                                       \
    using _FreeEvent = ResourceValue::FreeEvent&;                                                                                                                                                      \
    using _FreeEventValue = ResourceValue::FreeEvent;

#define RESOURCE_DECLARE_TYPES(InternalResourceValue) using _SystemObjectValue = InternalResourceValue;
#define RESOURCE_DatabaseAllocationEvent_DECLARE_TYPES(ResourceValue) using _ResourceValue = ResourceValue;
#define RESOURCE_InlineAllocationEvent_DECLARE_TYPES(ResourceValue)                                                                                                                                    \
    using _ResourceValue = ResourceValue;                                                                                                                                                              \
    using _Asset = Asset&;                                                                                                                                                                             \
    using _AssetValue = Asset;
#define RESOURCE_FreeEvent_DECLARE_TYPES(ResourceValue) using _ResourceValue = ResourceValue;