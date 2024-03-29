#pragma once

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

    using _Element = ElementType&;
    using _ElementValue = ElementType;

    using _FreeBlocksValue = Vector<Token<ElementType>>;
    using _FreeBlocks = _FreeBlocksValue&;

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

    inline Vector<ElementType>& get_memory()
    {
        return this->memory;
    };

    inline ElementType* get_memory_raw()
    {
        return this->memory.Memory.Memory;
    };

    inline iPool<Pool<ElementType>> to_ipool()
    {
        return iPool<Pool<ElementType>>{*this};
    };

    inline int8 has_allocated_elements()
    {
        return this->memory.Size != this->free_blocks.Size;
    };

    inline int8 is_element_free(const Token<ElementType> p_token)
    {
        return this->to_ipool().is_element_free(p_token);
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
        this->to_ipool().allocate_element_empty_v2();
    }

    // TODO -> finding a way to define a Token that is more restrictive. In this case, we want a token that is usable only for Pools.
    //         also, the token type must be very simple.
    inline Token<ElementType> alloc_element(const ElementType& p_element)
    {
        return this->to_ipool().allocate_element_v2(p_element);
    };

    inline void release_element(const Token<ElementType> p_token)
    {
#if __DEBUG
        this->element_not_free_check(p_token);
#endif

        this->free_blocks.push_back_element(p_token);
    };

    inline void _set_element(const Token<ElementType> p_token, const ElementType& p_element)
    {
        this->memory.set(token_value(p_token), p_element);
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