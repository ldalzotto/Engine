#pragma once


#if TOKEN_TYPE_SAFETY

template <class ElementType> struct Token
{
    union
    {
        token_t tok;
        struct Token_ internal;
    };
};

#define tk_bT(ElementType, TokenT)                                                                                                                                                                     \
    Token<ElementType>                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        (token_t)(TokenT)                                                                                                                                                                              \
    }

#define TokenT(ElementType) Token<ElementType>

#else

#define tk_bT(ElementType, TokenT) (token_t) TokenT

#define TokenT(ElementType) uimax

#endif

#define tk_bdT(ElementType) tk_bT(ElementType, -1)
#define tk_bfT(ElementType, SourceToken) tk_bT(ElementType, tk_v(SourceToken))