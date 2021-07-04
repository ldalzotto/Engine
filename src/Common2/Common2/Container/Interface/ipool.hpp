#pragma once

template <class _Pool> struct iPool_element
{
    /* empty, for tokenization */
};

namespace iPool_Types
{
template <class _Pool> using sToken = Token<iPool_element<_Pool>>;
template <class _Pool> using Element = typename _Pool::Element;
template <class _Pool> using ElementRef = typename _Pool::ElementRef;
template <class _Pool> using FreeBlock_Vector = typename _Pool::FreeBlock_Vector;
template <class _Pool> using FreeBlock_VectorRef = typename _Pool::FreeBlock_VectorRef;
}; // namespace iPool_Types

#define iPool_Types_forward(PoolType)                                                                                                                                                                  \
    using sToken = iPool_Types::sToken<PoolType>;                                                                                                                                                      \
    using Element = iPool_Types::Element<PoolType>;                                                                                                                                                    \
    using ElementRef = iPool_Types::ElementRef<PoolType>;                                                                                                                                              \
    using FreeBlock_Vector = iPool_Types::FreeBlock_Vector<PoolType>;                                                                                                                                  \
    using FreeBlock_VectorRef = iPool_Types::FreeBlock_VectorRef<PoolType>;

template <class _Pool> struct iPool
{
    _Pool& pool;
    iPool_Types_forward(_Pool);

    inline FreeBlock_VectorRef get_free_blocks()
    {
        return this->pool.get_free_blocs();
    };

    inline sToken allocate_element_empty_v2()
    {
        FreeBlock_VectorRef __free_blocks = this->get_free_blocks();
        iVector_v2<FreeBlock_Vector> l_free_blocks = iVector_v2<FreeBlock_Vector>{__free_blocks};
        if (!l_free_blocks.empty())
        {
            return l_free_blocks.pop_back_return();
        }
        else
        {
            uimax p_index = _push_back_element_empty();
            return sToken{p_index};
        }
    };

    inline sToken allocate_element_v2(const Element& p_element)
    {
        FreeBlock_VectorRef __free_blocks = this->get_free_blocks();
        iVector_v2<FreeBlock_Vector> l_free_blocks = iVector_v2<FreeBlock_Vector>{__free_blocks};
        if (!l_free_blocks.empty())
        {
            sToken l_availble_token = sToken::build_from(l_free_blocks.pop_back_return());
            _set_element(l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = _push_back_element(p_element);
            return sToken{p_index};
        }
    };

    inline int8 is_element_free(const sToken p_token)
    {
        FreeBlock_VectorRef __free_blocks = this->get_free_blocks();
        iVector_v2<FreeBlock_Vector> l_free_blocks = iVector_v2<FreeBlock_Vector>{__free_blocks};
        for (loop(i, 0, l_free_blocks.get_size()))
        {
            sToken l_pool_token = sToken::build_from(l_free_blocks.get(i));
            if (token_equals(l_pool_token, p_token))
            {
                return 1;
            }
        }
        return 0;
    };

    inline void release_element(const sToken p_token)
    {
        this->pool.release_element(p_token);
    };

    inline ElementRef get(const sToken p_token)
    {
        return this->pool.get(p_token);
    };

  private:
    inline uimax _push_back_element_empty()
    {
        return this->pool._push_back_element_empty();
    };

    inline void _set_element(const sToken p_token, const Element& p_element)
    {
        this->pool._set_element(p_token, p_element);
    };

    inline uimax _push_back_element(const Element& p_element)
    {
        return this->pool._push_back_element(p_element);
    };
};

// TODO -> remove ?
#define iPool_types_declare(p_pool_name, p_pool_type)                                                                                                                                                  \
    using t_##p_pool_name = p_pool_type;                                                                                                                                                               \
    using t_##p_pool_name##_element = typename t_##p_pool_name::Element;                                                                                                                               \
    using t_##p_pool_name##_sToken = typename t_##p_pool_name::sToken;                                                                                                                                 \
    using t_##p_pool_name##_sTokenValue = typename t_##p_pool_name::sTokenValue;