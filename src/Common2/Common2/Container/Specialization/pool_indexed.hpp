#pragma once

// TODO -> adding tests
template <class ElementType> struct PoolIndexed
{
    using t_Element = ElementType;
    using t_ElementRef = ElementType&;

    using t_MemoryPool = Pool<t_Element>;

    using sTokenValue = typename t_MemoryPool::sTokenValue;
    using sToken = typename t_MemoryPool::sToken;

    using t_IndicesVector = Vector<sToken>;

    t_MemoryPool Memory;
    t_IndicesVector Indices;

    inline static PoolIndexed<ElementType> allocate_default()
    {
        return PoolIndexed<ElementType>{t_MemoryPool::allocate(0), t_IndicesVector::allocate(0)};
    };

    inline void free()
    {
        this->Memory.free();
        this->Indices.free();
    };

    inline int8 has_allocated_elements()
    {
        return this->Memory.has_allocated_elements();
    };

    inline sToken alloc_element(const ElementType& p_element)
    {
        sToken l_token = this->Memory.alloc_element(p_element);
        this->Indices.push_back_element(l_token);
        return l_token;
    };

    inline void release_element(const sToken p_element)
    {
        this->Memory.release_element(p_element);
        for (loop(i, 0, this->Indices.Size))
        {
            if (token_equals(this->Indices.get(i), p_element))
            {
                this->Indices.erase_element_at_always(i);
                break;
            }
        };
    };

    inline ElementType& get(const sToken p_element)
    {
        return this->Memory.get(p_element);
    };

    inline ElementType& get_by_index(const uimax p_index)
    {
        sToken l_token = this->Indices.get(p_index);
        return this->Memory.get(l_token);
    };

    template <class ForeachSlot_t> inline void foreach (const ForeachSlot_t& p_foreach)
    {
        for (loop(i, 0, this->Indices.Size))
        {
            sToken l_token = this->Indices.get(i);
            p_foreach(l_token, this->Memory.get(l_token));
        }
    }
    template <class ForeachSlot_t> inline void foreach_breakable(const ForeachSlot_t& p_foreach)
    {
        for (loop(i, 0, this->Indices.Size))
        {
            sToken l_token = this->Indices.get(i);
            if (p_foreach(l_token, this->Memory.get(l_token)))
            {
                break;
            };
        }
    }

    template <class ForeachSlot_t> inline void foreach_reverse(const ForeachSlot_t& p_foreach)
    {
        for (loop_reverse(i, 0, this->Indices.Size))
        {
            sToken l_token = this->Indices.get(i);
            p_foreach(l_token, this->Memory.get(l_token));
        }
    }

#if 0
    // TODO to do the same common2 test
    struct iPool
    {
        using t_Element = PoolIndexed<ElementType>::t_Element;
        using t_ElementRef = PoolIndexed<ElementType>::t_ElementRef;

        using t_FreeBlocksVector = typename PoolIndexed<ElementType>::t_MemoryPool::t_FreeBlocksVector;
        using t_FreeBlocksVectorRef = typename PoolIndexed<ElementType>::t_MemoryPool::t_FreeBlocksVectorRef;

        PoolIndexed<ElementType>& thiz;

        inline t_FreeBlocksVectorRef get_free_blocs()
        {
            return thiz.Memory.get_free_blocs();
        };
    };

    inline iPool to_iPool()
    {
        return iPool{*this};
    };
#endif
};