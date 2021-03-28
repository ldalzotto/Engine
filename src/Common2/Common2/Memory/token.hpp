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

#if __TOKEN

template <class type_element> struct Token
{
    token_t tok;
};

#define token_build(type_element, value)                                                                                                                                                               \
    Token<type_element>                                                                                                                                                                                \
    {                                                                                                                                                                                                  \
        (token_t)(value)                                                                                                                                                                               \
    }

#define token_get_value(thiz) ((thiz).tok)

#define Token(type_element) Token<type_element>

#else

#define token_build(type_element, value) (token_t) value

#define token_get_value(thiz) (thiz)

#define Token(type_element) uimax

#endif

#define token_build_default(type_element) token_build(type_element, -1)
#define token_build_from(type_element, SourceToken) token_build(type_element, token_get_value(SourceToken))
#define token_equals(Left, Right) (token_get_value(Left) == token_get_value(Right))
#define token_not_equals(Left, Right) (token_get_value(Left) != token_get_value(Right))