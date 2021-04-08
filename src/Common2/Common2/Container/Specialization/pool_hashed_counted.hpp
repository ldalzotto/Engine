#pragma once

// TODO -> by default, all functions must use the hashed key value
template <class KeyType, class ElementType> struct PoolHashedCounted
{
    struct CountElement
    {
        Token<ElementType> token;
        uimax counter;
    };

    Pool<ElementType> pool;
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

    inline Token<ElementType> increment(const KeyType& p_key)
    {
        CountElement* l_counted_element = this->CountMap.get_value_nothashed(p_key);
        l_counted_element->counter += 1;
        return l_counted_element->token;
    };

    inline CountElement* decrement(const KeyType& p_key)
    {
        CountElement* l_counted_element = this->CountMap.get_value_nothashed(p_key);
#if __DEBUG
        assert_true(l_counted_element->counter != 0);
#endif
        l_counted_element->counter -= 1;
        return l_counted_element;
    };

    inline Token<ElementType> push_back_element(const KeyType& p_key, const ElementType& p_element)
    {
        Token<ElementType> l_allocated_token = this->pool.alloc_element(p_element);
        this->CountMap.push_key_value_nothashed(p_key, CountElement{l_allocated_token, 1});
        return l_allocated_token;
    };

    inline void remove_element(const KeyType& p_key, const Token<ElementType> p_element)
    {
        this->pool.release_element(p_element);
        this->CountMap.erase_key_nothashed(p_key);
    };

    inline ElementType& get(const KeyType& p_key)
    {
        return this->pool.get(this->CountMap.get_value_nothashed(p_key)->token);
    };

    template <class AllocationFunc> inline Token<ElementType> increment_or_allocate_v2(const KeyType& p_key, const AllocationFunc& p_allocation_func)
    {
        if (this->has_key_nothashed(p_key))
        {
            return this->increment(p_key);
        }
        else
        {
            return this->push_back_element(p_key, p_allocation_func());
        }
    };
};