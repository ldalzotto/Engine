#pragma once

template <class _Pool> struct iPool
{
    _Pool& pool;

    using _ElementValue = typename _Pool::_ElementValue;
    using _FreeBlocksValue = typename _Pool::_FreeBlocksValue;
    using _FreeBlocks = typename _Pool::_FreeBlocks;

    inline _FreeBlocks get_free_blocks()
    {
        return this->pool.get_free_blocs();
    };

    inline Token<_ElementValue> allocate_element_empty_v2()
    {
        _FreeBlocks __free_blocks = this->get_free_blocks();
        iVector<_FreeBlocksValue> l_free_blocks = __free_blocks.to_ivector();
        if (!l_free_blocks.empty())
        {
            return l_free_blocks.pop_back_return();
        }
        else
        {
            uimax p_index = _push_back_element_empty();
            return Token<_ElementValue>{p_index};
        }
    };

    inline Token<_ElementValue> allocate_element_v2(const _ElementValue& p_element)
    {
        _FreeBlocks __free_blocks = this->get_free_blocks();
        iVector<_FreeBlocksValue> l_free_blocks = __free_blocks.to_ivector();
        if (!l_free_blocks.empty())
        {
            Token<_ElementValue> l_availble_token = l_free_blocks.pop_back_return();
            _set_element(l_availble_token, p_element);
            return l_availble_token;
        }
        else
        {
            uimax p_index = _push_back_element(p_element);
            return Token<_ElementValue>{p_index};
        }
    };

    inline int8 is_element_free(const Token<_ElementValue> p_token)
    {
        _FreeBlocks __free_blocks = this->get_free_blocks();
        iVector<_FreeBlocksValue> l_free_blocks = __free_blocks.to_ivector();
        for (loop(i, 0, l_free_blocks.get_size()))
        {
            Token<_ElementValue> l_pool_token = l_free_blocks.get(i);
            if (token_equals(l_pool_token, p_token))
            {
                return 1;
            }
        }
        return 0;
    };

  private:
    inline uimax _push_back_element_empty()
    {
        return this->pool._push_back_element_empty();
    };

    inline void _set_element(const Token<_ElementValue> p_token, const _ElementValue& p_element)
    {
        this->pool._set_element(p_token, p_element);
    };

    inline uimax _push_back_element(const _ElementValue& p_element)
    {
        return this->pool._push_back_element(p_element);
    };
};
