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

#if TOKEN_TYPE_SAFETY

typedef struct Token_
{
    token_t tok;
} Token_;

#define Token(ElementType) Token_##ElementType

#define Token_declare(ElementType)                                                                                                                                                                     \
    typedef struct s_Token_##ElementType                                                                                                                                                               \
    {                                                                                                                                                                                                  \
        union                                                                                                                                                                                          \
        {                                                                                                                                                                                              \
            token_t tok;                                                                                                                                                                               \
            struct Token_ internal;                                                                                                                                                                    \
        };                                                                                                                                                                                             \
    } Token(ElementType)

#define tk_b(ElementType, TokenT)                                                                                                                                                                      \
    (Token(ElementType))                                                                                                                                                                               \
    {                                                                                                                                                                                                  \
        .tok = (TokenT)                                                                                                                                                                                \
    }

#define tk_v(TokenVariable) ((TokenVariable).tok)

#else

#define tk_b(ElementType, TokenT) (token_t) TokenT

#define tk_v(TokenVariable) (TokenVariable)

#endif

#define tk_bd(ElementType) tk_b(ElementType, -1)
#define tk_bf(ElementType, SourceToken) tk_b(ElementType, tk_v(SourceToken))
#define tk_eq(Left, Right) (tk_v(Left) == tk_v(Right))
#define tk_neq(Left, Right) (tk_v(Left) != tk_v(Right))