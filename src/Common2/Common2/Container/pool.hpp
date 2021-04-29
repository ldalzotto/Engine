#pragma once

// TODO -> finding a better way to do this ? Can we have an abstract ShadowPool structure with multiple types ?
struct PoolGenerics
{
    template <class TokenOuterType, class PoolType, class Pool_PushBackElementEmpty_Func>
    inline static TokenOuterType allocate_element_empty(PoolType& p_pool, Vector<TokenOuterType>& p_free_blocks, const Pool_PushBackElementEmpty_Func& p_pool_pushback_element_empty_func)
    {
        if (!p_free_blocks.empty())
        {
            TokenOuterType l_availble_token = p_free_blocks.get(p_free_blocks.Size - 1);
            p_free_blocks.pop_back();
            return l_availble_token;
        }
        else
        {
            uimax p_index = p_pool_pushback_element_empty_func(p_pool);
            return TokenOuterType{p_index};
        }
    };

    template <class ElementType, class PoolType, class Pool_SetElement_Func, class Pool_PushBackelement_Func>
    inline static Token<ElementType> allocate_element(PoolType& p_pool, const ElementType& p_element, Vector<Token<ElementType>>& p_free_blocks, const Pool_SetElement_Func& p_pool_set_element_func,
                                                      const Pool_PushBackelement_Func& p_pool_pushback_element_func)
    {
        if (!p_free_blocks.empty())
        {
            Token<ElementType> l_availble_token = p_free_blocks.get(p_free_blocks.Size - 1);
            p_free_blocks.pop_back();
            p_pool_set_element_func(p_pool, l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = p_pool_pushback_element_func(p_pool, p_element);
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

    inline Token<ElementType> alloc_element_empty()
    {
        PoolGenerics::allocate_element_empty(*this, this->free_blocks, push_back_element_empty);
    }

    inline Token<ElementType> alloc_element(const ElementType& p_element)
    {
        return PoolGenerics::allocate_element(*this, p_element, this->free_blocks, set_element, push_back_element);
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

    inline static uimax push_back_element_empty(Pool<ElementType>& p_pool)
    {
        p_pool.memory.push_back_element_empty();
        return p_pool.memory.Size - 1;
    };

    inline static uimax push_back_element(Pool<ElementType>& p_pool, const ElementType& p_element)
    {
        p_pool.memory.push_back_element(p_element);
        return p_pool.memory.Size - 1;
    };

    inline static void set_element(Pool<ElementType>& p_pool, const Token<ElementType> p_token, const ElementType& p_element)
    {
        p_pool.memory.get(token_value(p_token)) = p_element;
    }
};