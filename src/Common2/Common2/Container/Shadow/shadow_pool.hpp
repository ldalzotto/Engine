#pragma once


#define ShadowPool(ElementType) ShadowPool_##ElementType

#define ShadowPool_func_allocate_element_empty() alloc_element_empty()
#define ShadowPool_func_get_free_blocks() get_free_blocs()
#define ShadowPool_func__set_element(p_token, p_element) _set_element(p_token, p_element)
#define ShadowPool_func__push_back_element(p_element) _push_back_element(p_element)
#define ShadowPool_func__push_back_element_empty() _push_back_element_empty()

#define ShadowPool_c_allocate_element_empty(p_shadow_pool) (p_shadow_pool).ShadowPool_func_allocate_element_empty()
#define ShadowPool_c_get_free_blocks(p_shadow_pool) (p_shadow_pool).ShadowPool_func_get_free_blocks()
#define ShadowPool_c__set_element(p_shadow_pool, p_token, p_element) (p_shadow_pool).ShadowPool_func__set_element(p_token, p_element)
#define ShadowPool_c__push_back_element(p_shadow_pool, p_element) (p_shadow_pool).ShadowPool_func__push_back_element(p_element)
#define ShadowPool_c__push_back_element_empty(p_shadow_pool) (p_shadow_pool).ShadowPool_func__push_back_element_empty()

struct PoolAlgorithms
{

    template <class ShadowPool(ElementType)> inline static auto allocate_element_empty_v2(ShadowPool(ElementType) & p_pool) -> Token<decltype(ShadowPool(ElementType)::CompileType::Element)>
    {
        using ElementType = decltype(ShadowPool(ElementType)::CompileType::Element);
        using ShadowVector(FreeBlocks) = decltype(ShadowPool_c_get_free_blocks(p_pool));
        ShadowVector(FreeBlocks) l_free_blocks = ShadowPool_c_get_free_blocks(p_pool);
        if (!sv_c_empty(l_free_blocks))
        {
            Token<ElementType> l_availble_token = sv_c_get(l_free_blocks, l_free_blocks.Size - 1);
            sv_c_pop_back(l_free_blocks);
            return l_availble_token;
        }
        else
        {
            uimax p_index = ShadowPool_c__push_back_element_empty(p_pool);
            return Token<ElementType>{p_index};
        }
    };

    template <class ShadowPool(ElementType), class ElementType> inline static Token<ElementType> allocate_element_v2(ShadowPool(ElementType) & p_pool, const ElementType& p_element)
    {
        using ShadowVector(FreeBlocks) = decltype(ShadowPool_c_get_free_blocks(p_pool));
        ShadowVector(FreeBlocks) l_free_blocks = ShadowPool_c_get_free_blocks(p_pool);
        if (!sv_c_empty(l_free_blocks))
        {
            Token<ElementType> l_availble_token = sv_c_get(l_free_blocks, sv_c_get_size(l_free_blocks) - 1);
            sv_c_pop_back(l_free_blocks);
            ShadowPool_c__set_element(p_pool, l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = ShadowPool_c__push_back_element(p_pool, p_element);
            return Token<ElementType>{p_index};
        }
    };
};