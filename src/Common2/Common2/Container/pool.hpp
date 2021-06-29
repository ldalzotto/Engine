#pragma once

template <class ElementType> struct Pool;
namespace Pool_Types
{
template <class ElementType> using Element = ElementType;
template <class ElementType> using ElementRef = ElementType&;
// TODO -> remove
template <class ElementType> using sTokenValue = iPool_element<Pool<ElementType>>;
template <class ElementType> using sToken = Token<sTokenValue<ElementType>>;
template <class ElementType> using Memory_Vector = Vector<ElementType>;
template <class ElementType> using FreeBlock_Vector = Vector<sToken<ElementType>>;
template <class ElementType> using FreeBlock_VectorRef = FreeBlock_Vector<ElementType>&;
}; // namespace Pool_Types

#define Pool_Types_forward(Prefix, ElementType)                                                                                                                                                        \
    using Prefix##Element = Pool_Types::Element<ElementType>;                                                                                                                                          \
    using Prefix##ElementRef = Pool_Types::ElementRef<ElementType>;                                                                                                                                    \
    using Prefix##sTokenValue = Pool_Types::sTokenValue<ElementType>;                                                                                                                                  \
    using Prefix##sToken = Pool_Types::sToken<ElementType>;                                                                                                                                            \
    using Prefix##Memory_Vector = Pool_Types::Memory_Vector<ElementType>;                                                                                                                              \
    using Prefix##FreeBlock_Vector = Pool_Types::FreeBlock_Vector<ElementType>;                                                                                                                        \
    using Prefix##FreeBlock_VectorRef = Pool_Types::FreeBlock_VectorRef<ElementType>;

/*
    A Pool is a non continous Vector where elements are acessed via Tokens.
    Generated Tokens are unique from it's source Pool.
    Even if pool memory is reallocate, generated Tokens are still valid.
    /!\ It is very unsafe to store raw pointer of an element. Because is Pool memory is reallocated, then the pointer is no longer valid.
*/
template <class ElementType> struct Pool
{
    Pool_Types_forward(, ElementType);

    Memory_Vector memory;
    FreeBlock_Vector free_blocks;

    inline static Pool<ElementType> build(const Memory_Vector& p_memory, const FreeBlock_Vector& p_free_blocks)
    {
        return Pool<ElementType>{p_memory, p_free_blocks};
    };

    inline static Pool<ElementType> allocate(const uimax p_memory_capacity)
    {
        return Pool<ElementType>{Memory_Vector::allocate(p_memory_capacity), FreeBlock_Vector::build_zero_size(cast(sToken*, NULL), 0)};
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

    inline FreeBlock_Vector& get_free_blocs()
    {
        return this->free_blocks;
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

    inline int8 is_element_free(const sToken p_token)
    {
        return this->to_ipool().is_element_free(p_token);
    };

    inline ElementType& get(const sToken p_token)
    {
#if __DEBUG
        this->element_free_check(p_token);
#endif

        return this->memory.get(token_value(p_token));
    };

    inline sToken alloc_element_empty()
    {
        this->to_ipool().allocate_element_empty_v2();
    }

    inline sToken alloc_element(const ElementType& p_element)
    {
        return this->to_ipool().allocate_element_v2(p_element);
    };

    inline void release_element(const sToken p_token)
    {
#if __DEBUG
        this->element_not_free_check(p_token);
#endif

        this->free_blocks.push_back_element(p_token);
    };

    inline void _set_element(const sToken p_token, const ElementType& p_element)
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
    inline void element_free_check(const sToken p_token)
    {
#if __DEBUG
        if (this->is_element_free(p_token))
        {
            abort();
        }
#endif
    };

    inline void element_not_free_check(const sToken p_token)
    {
#if __DEBUG
        if (this->is_element_free(p_token))
        {
            abort();
        }
#endif
    };
};
