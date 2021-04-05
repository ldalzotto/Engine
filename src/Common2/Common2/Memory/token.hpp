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

#if __DEBUG

template <class ElementType> struct Token
{
    token_t tok;
};

template <class ElementType, class OtherType> inline int8 token_equals(const Token<ElementType> p_left, const Token<OtherType> p_right)
{
    return p_left.tok == p_right.tok;
};

template <class CastedType, class ElementType> inline Token<CastedType> token_build_from(const Token<ElementType> p_token)
{
    return Token<CastedType>{p_token.tok};
};

template <class ElementType> inline token_t token_value(const Token<ElementType> p_token)
{
    return p_token.tok;
};

template <class ElementType> inline Token<ElementType> token_build_default()
{
    return Token<ElementType>{(token_t)-1};
};

template <class ElementType> inline Token<ElementType> token_build(const token_t p_token)
{
    return Token<ElementType>{p_token};
};

#elif __RELEASE

template <class ElementType> using Token = token_t;

inline int8 token_equals(const token_t p_left, const token_t p_right)
{
    return p_left == p_right;
}

template <class CastedType>
inline token_t token_build_from(const token_t p_token)
{
    return p_token;
};

inline token_t token_value(const token_t p_token)
{
    return p_token;
};

template <class ElementType>
inline token_t token_build_default()
{
    return (token_t)-1;
};

template <class ElementType>
inline token_t token_build(const token_t p_token)
{
    return p_token;
};

#endif


