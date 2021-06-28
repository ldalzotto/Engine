#pragma once

#include "vector_of_vector.hpp"

/*
    A PoolOfVector is a wrapped VectorOfVector with pool allocation logic.
    Any operation on nested vectors must be called with the (poolofvector_element_*) functions.
    The PoolOfVector can be linked with a Pool.
*/
template <class ElementType> struct PoolOfVector
{
    using sToken = Token<iPool_element<PoolOfVector<ElementType>>>;

    VectorOfVector<ElementType> Memory;
    Vector<sToken> FreeBlocks;

    using tElement = Slice<ElementType>;
    using tElementRef = Slice<ElementType>;

    using tFreeBlocksVector = Vector<sToken>;
    using tFreeBlocksVectorRef = tFreeBlocksVector&;

    inline static PoolOfVector<ElementType> allocate_default()
    {
        return PoolOfVector<ElementType>{VectorOfVector<ElementType>::allocate_default(), Vector<sToken>::allocate(0)};
    };

    inline void free()
    {
        this->Memory.free();
        this->FreeBlocks.free();
    };

    inline Vector<sToken>& get_free_blocs()
    {
        return this->FreeBlocks;
    };

    inline iPool<PoolOfVector<ElementType>> to_ipool()
    {
        return iPool<PoolOfVector<ElementType>>{*this};
    };

    inline int8 is_element_free(const sToken p_token)
    {
        return this->to_ipool().is_element_free(p_token);
    };

    inline VectorOfVector<ElementType>& get_memory()
    {
        return this->Memory;
    };

    inline int8 has_allocated_elements()
    {
        return this->Memory.varying_vector.get_size() != this->FreeBlocks.Size;
    }

    inline sToken alloc_vector_with_values(const Slice<ElementType>& p_initial_elements)
    {
        return this->to_ipool().allocate_element_v2(p_initial_elements);
    };

    inline sToken alloc_vector()
    {
        return this->to_ipool().allocate_element_empty_v2();
    };

    inline void release_vector(const sToken p_token)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif

        this->Memory.element_clear(token_value(p_token));
        this->FreeBlocks.push_back_element(p_token);
    };

    inline Slice<ElementType> get_vector(const sToken p_token)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif

        return this->Memory.get(token_value(p_token));
    };

    inline void element_push_back_element(const sToken p_token, const ElementType& p_element)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif

        this->Memory.element_push_back_element(token_value(p_token), p_element);
    };

    inline void element_erase_element_at(const sToken p_token, const uimax p_index)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif
        this->Memory.element_erase_element_at(token_value(p_token), p_index);
    };

    inline void element_erase_element_at_always(const sToken p_token, const uimax p_index)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif
        this->Memory.element_erase_element_at_always(token_value(p_token), p_index);
    };

    inline void element_clear(const sToken p_token)
    {

#if __DEBUG
        this->token_not_free_check(p_token);
#endif
        this->Memory.element_clear(token_value(p_token));
    };

    struct Element_iVector
    {
        PoolOfVector<ElementType>* pool_of_vector;
        sToken index;

        inline static Element_iVector build(PoolOfVector<ElementType>* p_pool_of_vector, const sToken p_index)
        {
            return Element_iVector{p_pool_of_vector, p_index};
        };

        inline uimax get_size()
        {
            return this->pool_of_vector->get_vector(this->index).Size;
        };

        inline ElementType& get(const uimax p_index)
        {
            return this->pool_of_vector->get_vector(this->index).get(p_index);
        };

        inline void push_back_element(const ElementType& p_element)
        {
            this->pool_of_vector->element_push_back_element(this->index, p_element);
        };

        inline void erase_element_at_always(const uimax p_index)
        {
            this->pool_of_vector->element_erase_element_at_always(this->index, p_index);
        };

        inline Slice<ElementType> to_slice()
        {
            return this->pool_of_vector->get_vector(this->index);
        };
    };

    inline Element_iVector get_element_as_iVector(const sToken p_index)
    {
        return Element_iVector::build(this, p_index);
    };

    inline void _set_element(const sToken p_token, const Slice<ElementType>& p_element)
    {
        this->Memory.set(token_value(p_token), p_element);
    };

    inline uimax _push_back_element_empty()
    {
        this->Memory.push_back_element_empty();
        return this->Memory.get_size() - 1;
    };

    inline uimax _push_back_element(const Slice<ElementType>& p_element)
    {
        this->Memory.push_back_element(p_element);
        return this->Memory.get_size() - 1;
    };

  private:
    inline void token_not_free_check(const sToken p_token)
    {
#if __DEBUG
        if (this->is_element_free(p_token))
        {
            abort();
        }
#endif
    };
};