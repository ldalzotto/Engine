#pragma once

struct ShadowPool
{
    template <class Container> inline static typename Container::_FreeBlocks get_free_blocks(Container& p_container)
    {
        return p_container.get_free_blocs();
    };

    template <class Container> inline static uimax _push_back_element_empty(Container& p_container)
    {
        return p_container._push_back_element_empty();
    };

    template <class Container> inline static void _set_element(Container& p_container, const Token<typename Container::_ElementValue> p_token, const typename Container::_ElementValue& p_element)
    {
        p_container._set_element(p_token, p_element);
    };

    template <class Container> inline static uimax _push_back_element(Container& p_container, const typename Container::_ElementValue& p_element)
    {
        return p_container._push_back_element(p_element);
    };
};

struct PoolAlgorithms
{
    template <class Container> inline static Token<typename Container::_ElementValue> allocate_element_empty_v2(Container& p_pool)
    {
        typename Container::_FreeBlocks __free_blocks = ShadowPool::get_free_blocks(p_pool);
        ShadowVector_v3<typename Container::_FreeBlocksValue> l_free_blocks = __free_blocks.to_shadow_vector();
        if (!l_free_blocks.empty())
        {
            return VectorAlgorithm::pop_back_return(l_free_blocks);
        }
        else
        {
            uimax p_index = ShadowPool::_push_back_element_empty(p_pool);
            return Token<typename Container::_ElementValue>{p_index};
        }
    };

    template <class Container> inline static Token<typename Container::_ElementValue> allocate_element_v2(Container& p_pool, const typename Container::_ElementValue& p_element)
    {
        typename Container::_FreeBlocks __free_blocks = ShadowPool::get_free_blocks(p_pool);
        ShadowVector_v3<typename Container::_FreeBlocksValue> l_free_blocks = __free_blocks.to_shadow_vector();
        if (!l_free_blocks.empty())
        {
            Token<typename Container::_ElementValue> l_availble_token = VectorAlgorithm::pop_back_return(l_free_blocks);
            ShadowPool::_set_element(p_pool, l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = ShadowPool::_push_back_element(p_pool, p_element);
            return Token<typename Container::_ElementValue>{p_index};
        }
    };

    template <class Container, class ElementType> inline static int8 is_element_free(Container& p_pool, const Token<ElementType> p_token)
    {
        typename Container::_FreeBlocks __free_blocks = ShadowPool::get_free_blocks(p_pool);
        ShadowVector_v3<typename Container::_FreeBlocksValue> l_free_blocks = __free_blocks.to_shadow_vector();
        for (loop(i, 0, l_free_blocks.get_size()))
        {
            Token<ElementType> l_pool_token = l_free_blocks.get(i);
            if (token_equals(l_pool_token, p_token))
            {
                return 1;
            }
        }
        return 0;
    };
};