#pragma once

template <class _Pool> struct iPool_element
{
    /* empty, for tokenization */
};

template <class _Pool> struct iPool
{
    _Pool& pool;

    using t_Element = typename _Pool::t_Element;
    using t_ElementRef = typename _Pool::t_ElementRef;
    using t_FreeBlocksVector = typename _Pool::t_FreeBlocksVector;
    using t_FreeBlocksVectorRef = typename _Pool::t_FreeBlocksVectorRef;

    using sTokenValue = iPool_element<_Pool>;
    using sToken = Token<sTokenValue>;

    inline t_FreeBlocksVectorRef get_free_blocks()
    {
        return this->pool.get_free_blocs();
    };

    inline sToken allocate_element_empty_v2()
    {
        t_FreeBlocksVectorRef __free_blocks = this->get_free_blocks();
        iVector<t_FreeBlocksVector> l_free_blocks = iVector<t_FreeBlocksVector>{__free_blocks};
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

    inline sToken allocate_element_v2(const t_Element& p_element)
    {
        t_FreeBlocksVectorRef __free_blocks = this->get_free_blocks();
        iVector<t_FreeBlocksVector> l_free_blocks = iVector<t_FreeBlocksVector>{__free_blocks};
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
        t_FreeBlocksVectorRef __free_blocks = this->get_free_blocks();
		iVector<t_FreeBlocksVector> l_free_blocks = iVector<t_FreeBlocksVector>{__free_blocks};
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

    inline t_ElementRef get(const sToken p_token)
    {
        return this->pool.get(p_token);
    };

  private:
    inline uimax _push_back_element_empty()
    {
        return this->pool._push_back_element_empty();
    };

    inline void _set_element(const sToken p_token, const t_Element& p_element)
    {
        this->pool._set_element(p_token, p_element);
    };

    inline uimax _push_back_element(const t_Element& p_element)
    {
        return this->pool._push_back_element(p_element);
    };
};
