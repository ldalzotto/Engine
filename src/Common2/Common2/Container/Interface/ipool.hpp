#pragma once

template <class _Pool> struct iPool_element
{
    /* empty, for tokenization */
};

template <class _Pool> struct iPool
{
    _Pool& pool;

    using tElement = typename _Pool::tElement;
    using tElementRef = typename _Pool::tElementRef;
    using tFreeBlocksVector = typename _Pool::tFreeBlocksVector;
    using tFreeBlocksVectorRef = typename _Pool::tFreeBlocksVectorRef;

    using sTokenValue = iPool_element<_Pool>;
    using sToken = Token<sTokenValue>;

    inline tFreeBlocksVectorRef get_free_blocks()
    {
        return this->pool.get_free_blocs();
    };

    inline sToken allocate_element_empty_v2()
    {
        tFreeBlocksVectorRef __free_blocks = this->get_free_blocks();
        iVector<tFreeBlocksVector> l_free_blocks = iVector<tFreeBlocksVector>{__free_blocks};
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

    inline sToken allocate_element_v2(const tElement& p_element)
    {
		tFreeBlocksVectorRef __free_blocks = this->get_free_blocks();
        iVector<tFreeBlocksVector> l_free_blocks = iVector<tFreeBlocksVector>{__free_blocks};
        if (!l_free_blocks.empty())
        {
            sToken l_availble_token = token_build_from<sTokenValue>(l_free_blocks.pop_back_return());
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
		tFreeBlocksVectorRef __free_blocks = this->get_free_blocks();
		iVector<tFreeBlocksVector> l_free_blocks = iVector<tFreeBlocksVector>{__free_blocks};
        for (loop(i, 0, l_free_blocks.get_size()))
        {
            sToken l_pool_token = token_build_from<sTokenValue>(l_free_blocks.get(i));
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

    inline tElementRef get(const sToken p_token)
    {
        return this->pool.get(p_token);
    };

  private:
    inline uimax _push_back_element_empty()
    {
        return this->pool._push_back_element_empty();
    };

    inline void _set_element(const sToken p_token, const tElement& p_element)
    {
        this->pool._set_element(p_token, p_element);
    };

    inline uimax _push_back_element(const tElement& p_element)
    {
        return this->pool._push_back_element(p_element);
    };
};
