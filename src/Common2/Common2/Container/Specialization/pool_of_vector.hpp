#pragma once

#include "vector_of_vector.hpp"

template <class ElementType> using PoolOfVectorToken = Token<Slice<ElementType>>;

/*
    A PoolOfVector is a wrapped VectorOfVector with pool allocation logic.
    Any operation on nested vectors must be called with the (poolofvector_element_*) functions.
    The PoolOfVector can be linked with a Pool.
*/
template <class ElementType> struct PoolOfVector
{
    VectorOfVector<ElementType> Memory;
    Vector<PoolOfVectorToken<ElementType>> FreeBlocks;

    using _ElementValue = Slice<ElementType>;
    using _FreeBlocks = Vector<PoolOfVectorToken<ElementType>>&;

    inline static PoolOfVector<ElementType> allocate_default()
    {
        return PoolOfVector<ElementType>{VectorOfVector<ElementType>::allocate_default(), Vector<PoolOfVectorToken<ElementType>>::allocate(0)};
    };

    inline void free()
    {
        this->Memory.free();
        this->FreeBlocks.free();
    };

    inline Vector<PoolOfVectorToken<ElementType>>& get_free_blocs()
    {
        return this->FreeBlocks;
    };

    inline int8 is_element_free(const PoolOfVectorToken<ElementType> p_token)
    {
        return PoolAlgorithms::is_element_free(*this, p_token);
    };

    inline VectorOfVector<ElementType>& get_memory()
    {
        return this->Memory;
    };

    inline int8 has_allocated_elements()
    {
        return this->Memory.varying_vector.get_size() != this->FreeBlocks.Size;
    }

    inline PoolOfVectorToken<ElementType> alloc_vector_with_values(const Slice<ElementType>& p_initial_elements)
    {
        return PoolAlgorithms::allocate_element_v2(*this, p_initial_elements);
    };

    inline PoolOfVectorToken<ElementType> alloc_vector()
    {
        return PoolAlgorithms::allocate_element_empty_v2(*this);
    };

    inline void release_vector(const PoolOfVectorToken<ElementType> p_token)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif

        this->Memory.element_clear(token_value(p_token));
        this->FreeBlocks.push_back_element(p_token);
    };

    inline Slice<ElementType> get_vector(const PoolOfVectorToken<ElementType> p_token)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif

        return this->Memory.get(token_value(p_token));
    };

    inline void element_push_back_element(const PoolOfVectorToken<ElementType> p_token, const ElementType& p_element)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif

        this->Memory.element_push_back_element(token_value(p_token), p_element);
    };

    inline void element_erase_element_at(const PoolOfVectorToken<ElementType> p_token, const uimax p_index)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif
        this->Memory.element_erase_element_at(token_value(p_token), p_index);
    };

    inline void element_erase_element_at_always(const PoolOfVectorToken<ElementType> p_token, const uimax p_index)
    {
#if __DEBUG
        this->token_not_free_check(p_token);
#endif
        this->Memory.element_erase_element_at_always(token_value(p_token), p_index);
    };

    inline void element_clear(const PoolOfVectorToken<ElementType> p_token)
    {

#if __DEBUG
        this->token_not_free_check(p_token);
#endif
        this->Memory.element_clear(token_value(p_token));
    };

    struct Element_ShadowVector
    {
        PoolOfVector<ElementType>* pool_of_vector;
        PoolOfVectorToken<ElementType> index;

        inline static Element_ShadowVector build(PoolOfVector<ElementType>* p_pool_of_vector, const PoolOfVectorToken<ElementType> p_index)
        {
            return Element_ShadowVector{p_pool_of_vector, p_index};
        };

        inline uimax sv_func_get_size()
        {
            return this->pool_of_vector->get_vector(this->index).Size;
        };

        inline ElementType& sv_func_get(const uimax p_index)
        {
            return this->pool_of_vector->get_vector(this->index).get(p_index);
        };

        inline void sv_func_push_back_element(const ElementType& p_element)
        {
            this->pool_of_vector->element_push_back_element(this->index, p_element);
        };

        inline void sv_func_erase_element_at_always(const uimax p_index)
        {
            this->pool_of_vector->element_erase_element_at_always(this->index, p_index);
        };

        inline Slice<ElementType> sv_func_to_slice()
        {
            return this->pool_of_vector->get_vector(this->index);
        };
    };

    inline Element_ShadowVector get_element_as_shadow_vector(const PoolOfVectorToken<ElementType> p_index)
    {
        return Element_ShadowVector::build(this, p_index);
    };

    inline void _set_element(const PoolOfVectorToken<ElementType> p_token, const Slice<ElementType>& p_element)
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
    inline void token_not_free_check(const PoolOfVectorToken<ElementType> p_token)
    {
#if __DEBUG
        if (this->is_element_free(p_token))
        {
            abort();
        }
#endif
    };
};