#pragma once

static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline Span<int8> encode_base64(const Slice<int8>& p_input)
{
    uint8* pos;
    const uint8 *end, *in;
    uimax olen;
    int line_len;

    olen = p_input.Size * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
    olen += olen / 72;               /* line feeds */
    olen++;                          /* nul termination */
    if (olen < p_input.Size)
    {
        return Span<int8>::build_default(); /* integer overflow */
    }

    Span<int8> l_return = Span<int8>::allocate(olen);

    end = (uint8*)p_input.Begin + p_input.Size;
    in = (uint8*)p_input.Begin;
    pos = (uint8*)l_return.Memory;
    line_len = 0;
    while (end - in >= 3)
    {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
        line_len += 4;
        if (line_len >= 72)
        {
            *pos++ = '\n';
            line_len = 0;
        }
    }

    if (end - in)
    {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
        line_len += 4;
    }

    l_return.Capacity = pos - (uint8*)l_return.Memory;

    return l_return;
};