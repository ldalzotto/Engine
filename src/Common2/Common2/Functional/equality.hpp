#pragma once

struct Equality
{
    struct Default
    {
        template <class Left, class Right>
        inline int8 operator()(const Left& p_left, const Right& p_right) const
        {
            return p_left == p_right;
        };
    };
};