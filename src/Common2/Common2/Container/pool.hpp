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

/*
    A Pool is a non continous Vector where elements are acessed via Tokens.
    Generated Tokens are unique from it's source Pool.
    Even if pool memory is reallocate, generated Tokens are still valid.
    /!\ It is very unsafe to store raw pointer of an element. Because is Pool memory is reallocated, then the pointer is no longer valid.
*/
template <class ElementType> struct Pool
{
    Vector<ElementType> memory;
    Vector<Token<ElementType>> free_blocks;

    struct CompileType
    {
        ElementType Element;
    };

    inline static Pool<ElementType> build(const Vector<ElementType>& p_memory, const Vector<Token<ElementType>>& p_free_blocks)
    {
        return Pool<ElementType>{p_memory, p_free_blocks};
    };

    inline static Pool<ElementType> allocate(const uimax p_memory_capacity)
    {
        return Pool<ElementType>{Vector<ElementType>::allocate(p_memory_capacity), Vector<Token<ElementType>>::build_zero_size(cast(Token<ElementType>*, NULL), 0)};
    };

    inline void free()
    {
        this->memory.free();
        this->free_blocks.free();
    };

    inline uimax get_size()
    {
        return this->memory.Size;
    };

    inline uimax get_capacity()
    {
        return this->memory.Memory.Capacity;
    };

    inline uimax get_free_size()
    {
        return this->free_blocks.Size;
    };

    inline Vector<Token<ElementType>>& get_free_blocs()
    {
        return this->free_blocks;
    };

    inline ElementType* get_memory()
    {
        return this->memory.Memory.Memory;
    };

    inline int8 has_allocated_elements()
    {
        return this->memory.Size != this->free_blocks.Size;
    };

    inline int8 is_element_free(const Token<ElementType> p_token)
    {
        for (vector_loop(&this->free_blocks, i))
        {
            if (token_value(this->free_blocks.get(i)) == token_value(p_token))
            {
                return 1;
            };
        };

        return 0;
    };

    inline ElementType& get(const Token<ElementType> p_token)
    {
#if __DEBUG
        this->element_free_check(p_token);
#endif

        return this->memory.get(token_value(p_token));
    };

    struct ShadowPool
    {
        Pool<ElementType>& pool;
    };

    inline Token<ElementType> alloc_element_empty()
    {
        PoolAlgorithms::allocate_element_empty_v2(*this);
    }

    inline Token<ElementType> alloc_element(const ElementType& p_element)
    {
        return PoolAlgorithms::allocate_element_v2(*this, p_element);
    };

    inline void _set_element(const Token<ElementType> p_token, const ElementType& p_element)
    {
        this->memory.get(token_value(p_token)) = p_element;
    };

    inline uimax _push_back_element_empty()
    {
        this->memory.push_back_element_empty();
        return this->memory.Size - 1;
    };

    inline uimax _push_back_element(const ElementType& p_element)
    {
        this->memory.push_back_element(p_element);
        return this->memory.Size - 1;
    };

    inline void release_element(const Token<ElementType> p_token)
    {
#if __DEBUG
        this->element_not_free_check(p_token);
#endif

        this->free_blocks.push_back_element(p_token);
    };

  private:
    inline void element_free_check(const Token<ElementType> p_token)
    {
#if __DEBUG
        if (this->is_element_free(p_token))
        {
            abort();
        }
#endif
    };

    inline void element_not_free_check(const Token<ElementType> p_token)
    {
#if __DEBUG
        if (this->is_element_free(p_token))
        {
            abort();
        }
#endif
    };
};