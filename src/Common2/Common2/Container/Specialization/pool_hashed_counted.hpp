#pragma once

template <class KeyType, class ElementType> struct PoolHashedCounted
{
    using t_MemoryPool = Pool<ElementType>;
    using sToken = typename t_MemoryPool::sToken;
    using sTokenValue = typename t_MemoryPool::sTokenValue;

    struct CountElement
    {
        sToken token;
        uimax counter;
    };

    t_MemoryPool pool;
    HashMap<KeyType, CountElement> CountMap;

    inline static PoolHashedCounted<KeyType, ElementType> allocate_default()
    {
        return PoolHashedCounted<KeyType, ElementType>{Pool<ElementType>::allocate(0), HashMap<KeyType, CountElement>::allocate_default()};
    };

    inline void free()
    {
        this->pool.free();
        this->CountMap.free();
    };

    inline int8 empty()
    {
        return !this->pool.has_allocated_elements() && this->CountMap.empty();
    };

    inline int8 has_key_nothashed(const KeyType& p_key)
    {
        return this->CountMap.has_key_nothashed(p_key);
    };

    inline sToken increment(const hash_t p_key)
    {
        CountElement* l_counted_element = this->CountMap.get_value(p_key);
        l_counted_element->counter += 1;
        return l_counted_element->token;
    };

    inline sToken increment_nothashed(const KeyType& p_key)
    {
        return this->increment(this->CountMap.hash_key(p_key));
    };

    inline CountElement* decrement(const hash_t p_key)
    {
        CountElement* l_counted_element = this->CountMap.get_value(p_key);
#if __DEBUG
        assert_true(l_counted_element->counter != 0);
#endif
        l_counted_element->counter -= 1;
        return l_counted_element;
    };

    inline CountElement* decrement_nothashed(const KeyType& p_key)
    {
        return this->decrement(this->CountMap.hash_key(p_key));
    };

    inline sToken push_back_element_nothashed(const KeyType& p_key, const ElementType& p_element)
    {
        sToken l_allocated_token = token_build_from<sTokenValue>(this->pool.alloc_element(p_element));
        this->CountMap.push_key_value_nothashed(p_key, CountElement{l_allocated_token, 1});
        return l_allocated_token;
    };

    inline void remove_element(const hash_t p_key, const sToken p_element)
    {
        this->pool.release_element(p_element);
        this->CountMap.erase_key(p_key);
    };

    inline void remove_element_nothashed(const KeyType& p_key, const sToken p_element)
    {
        this->remove_element(this->CountMap.hash_key(p_key), p_element);
    };

    inline ElementType& get_from_pool(const sToken p_token)
    {
        return this->pool.get(p_token);
    };

    inline ElementType& get(const hash_t p_key)
    {
        return this->get_from_pool(this->CountMap.get_value(p_key)->token);
    };

    inline ElementType& get_nothashed(const KeyType& p_key)
    {
        return this->get(this->CountMap.hash_key(p_key));
    };

    template <class AllocationFunc> inline sToken increment_or_allocate_v2(const KeyType& p_key, const AllocationFunc& p_allocation_func)
    {
        if (this->has_key_nothashed(p_key))
        {
            return this->increment_nothashed(p_key);
        }
        else
        {
            return this->push_back_element_nothashed(p_key, p_allocation_func());
        }
    };
};