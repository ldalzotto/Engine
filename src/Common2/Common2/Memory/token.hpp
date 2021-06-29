#pragma once

typedef uimax token_t;

inline token_t tokent_build_default()
{
    return -1;
};

inline void tokent_reset(token_t* p_token)
{
    *p_token = tokent_build_default();
};

inline int8 tokent_is_valid(const token_t* p_token)
{
    return *p_token != (token_t)-1;
};

template <class ElementType> struct Token
{
    token_t tok;

    inline static Token<ElementType> build(const token_t p_value)
    {
        return Token<ElementType>{p_value};
    };

    template <class OtherToken> inline static Token<ElementType> build_from(const OtherToken p_other)
    {
        return Token<ElementType>{p_other.tok};
    };

    inline static Token<ElementType> build_default()
    {
        return Token<ElementType>{(token_t)-1};
    };

    inline void equals(const Token<ElementType> p_other)
    {
        return this->tok == p_other.tok;
    };

    template <class CastedToken> inline CastedToken tcast()
    {
        return CastedToken{this->tok};
    };
};

// TODO -> remove
template <class ElementType, class OtherType> inline int8 token_equals(const Token<ElementType> p_left, const Token<OtherType> p_right)
{
    return p_left.tok == p_right.tok;
};

// TODO -> remove
template <class CastedType, class ElementType> inline Token<CastedType> token_build_from(const Token<ElementType> p_token)
{
    return Token<CastedType>{p_token.tok};
};

// TODO -> remove
template <class ElementType> inline token_t token_value(const Token<ElementType> p_token)
{
    return p_token.tok;
};

// TODO -> remove
template <class ElementType> inline token_t* token_ptr(Token<ElementType>& p_token)
{
    return &p_token.tok;
};

// TODO -> remove
template <class ElementType> inline const token_t* token_ptr(const Token<ElementType>& p_token)
{
    return &p_token.tok;
};

// TODO -> remove
template <class ElementType> inline Token<ElementType> token_build_default()
{
    return Token<ElementType>{(token_t)-1};
};

// TODO -> remove
template <class ElementType> inline Token<ElementType> token_build(const token_t p_token)
{
    return Token<ElementType>{p_token};
};
