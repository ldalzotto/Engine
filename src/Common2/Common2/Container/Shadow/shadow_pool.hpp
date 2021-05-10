#pragma once

#define ShadowPool(ElementType) ShadowPool_##ElementType

#define ShadowPool_static_assert_element_type(p_shadow_pool_type, p_compared_type)                                                                                                                     \
    static_assert(sizeof(decltype(p_shadow_pool_type::CompileType::Element)) == sizeof(p_compared_type), "ShadowPool type assertion")

#define ShadowPool_func_allocate_element_empty() alloc_element_empty()
#define ShadowPool_func_get_free_blocks() get_free_blocs()
#define ShadowPool_func_get_memory() get_memory()

#define ShadowPool_c_allocate_element_empty(p_shadow_pool) (p_shadow_pool).ShadowPool_func_allocate_element_empty()
#define ShadowPool_c_get_free_blocks(p_shadow_pool) (p_shadow_pool).ShadowPool_func_get_free_blocks()
#define ShadowPool_c_get_memory(p_shadow_pool) (p_shadow_pool).ShadowPool_func_get_memory()

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
            uimax p_index = _push_back_element_empty(p_pool);
            return Token<ElementType>{p_index};
        }
    };

    template <class ShadowPool(ElementType), class ElementType> inline static Token<ElementType> allocate_element_v2(ShadowPool(ElementType) & p_pool, const ElementType& p_element)
    {
        ShadowPool_static_assert_element_type(ShadowPool(ElementType), ElementType);

        using ShadowVector(FreeBlocks) = decltype(ShadowPool_c_get_free_blocks(p_pool));
        ShadowVector(FreeBlocks) l_free_blocks = ShadowPool_c_get_free_blocks(p_pool);
        if (!sv_c_empty(l_free_blocks))
        {
            Token<ElementType> l_availble_token = sv_c_get(l_free_blocks, sv_c_get_size(l_free_blocks) - 1);
            sv_c_pop_back(l_free_blocks);
            _set_element(p_pool, l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = _push_back_element(p_pool, p_element);
            return Token<ElementType>{p_index};
        }
    };

    template <class ShadowPool(ElementType), class ElementType> inline static int8 is_element_free(ShadowPool(ElementType) & p_pool, const Token<ElementType> p_token)
    {
        ShadowPool_static_assert_element_type(ShadowPool(ElementType), ElementType);

        using ShadowVector(FreeBlocks) = decltype(ShadowPool_c_get_free_blocks(p_pool));
        ShadowVector(FreeBlocks) l_free_blocks = ShadowPool_c_get_free_blocks(p_pool);

        for (loop(i, 0, sv_c_get_size(l_free_blocks)))
        {
            if (token_equals(sv_c_get(l_free_blocks, i), p_token))
            {
                return 1;
            }
        }
        return 0;
    };

  private:
    template <class ShadowPool(ElementType), class ElementType> inline static void _set_element(ShadowPool(ElementType) & p_pool, const Token<ElementType> p_token, const ElementType& p_element)
    {
        ShadowPool_static_assert_element_type(ShadowPool(ElementType), ElementType);

        using ShadowVector(Memory) = decltype(ShadowPool_c_get_memory(p_pool));
        ShadowVector(Memory)& l_pool_memory = ShadowPool_c_get_memory(p_pool);
        sv_c_set(l_pool_memory, token_value(p_token), p_element);
    };

    template <class ShadowPool(ElementType)> inline static uimax _push_back_element_empty(ShadowPool(ElementType) & p_pool)
    {
        using ShadowVector(Memory) = decltype(ShadowPool_c_get_memory(p_pool));
        ShadowVector(Memory)& l_pool_memory = ShadowPool_c_get_memory(p_pool);

        sv_c_push_back_element_empty(l_pool_memory);

        return sv_c_get_size(l_pool_memory) - 1;
    };

    template <class ShadowPool(ElementType), class ElementType> inline static uimax _push_back_element(ShadowPool(ElementType) & p_pool, const ElementType& p_element)
    {
        ShadowPool_static_assert_element_type(ShadowPool(ElementType), ElementType);

        using ShadowVector(Memory) = decltype(ShadowPool_c_get_memory(p_pool));
        ShadowVector(Memory)& l_pool_memory = ShadowPool_c_get_memory(p_pool);

        sv_c_push_back_element(l_pool_memory, p_element);

        return sv_c_get_size(l_pool_memory) - 1;
    };
};