#pragma once

Slice_declare(Token_);
SpanC_declare(Token_);
Vector_declare(Token_);

typedef struct s_Pool_ {
    Vector_ memory;
    Vector_Token_ free_blocks;
} Pool_;

