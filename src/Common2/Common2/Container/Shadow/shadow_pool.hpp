#pragma once

// TODO -> shadow pool methods must be template

struct PoolAlgorithms
{
    template <class Container> inline static auto allocate_element_empty_v2(Container& p_pool) -> Token<typename Container::_ElementValue>
    {
        typename Container::_FreeBlocks l_free_blocks = p_pool.get_free_blocs();
        if (!VectorAlgorithm::empty(l_free_blocks))
        {
            return VectorAlgorithm::pop_back_return(l_free_blocks);
        }
        else
        {
            uimax p_index = p_pool._push_back_element_empty();
            return Token<typename Container::_ElementValue>{p_index};
        }
    };

    template <class Container, class ElementType> inline static Token<ElementType> allocate_element_v2(Container& p_pool, const ElementType& p_element)
    {
        typename Container::_FreeBlocks l_free_blocks = p_pool.get_free_blocs();
        if (!VectorAlgorithm::empty(l_free_blocks))
        {
            Token<ElementType> l_availble_token = VectorAlgorithm::pop_back_return(l_free_blocks);
            p_pool._set_element(l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = p_pool._push_back_element(p_element);
            return Token<ElementType>{p_index};
        }
    };

    template <class Container, class ElementType> inline static int8 is_element_free(Container& p_pool, const Token<ElementType> p_token)
    {
        int8 l_is_element_free = 0;
        typename Container::_FreeBlocks l_free_blocks = p_pool.get_free_blocs();
        VectorAlgorithm::iterator(l_free_blocks, [&](const Token<ElementType> p_pool_token) {
            if (token_equals(p_pool_token, p_token))
            {
                l_is_element_free = 1;
                return;
            }
        });
        return l_is_element_free;
    };
};