#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

struct ImgCompiler
{
    inline static void compile(Slice<int8>& p_img_content, v3ui* out_size, int8* out_channel_nb, Span<int8>* out_pixels)
    {
        int32 l_channel_number;
        out_pixels->Memory = (int8*)stbi_load_from_memory((const uint8*)p_img_content.Begin, (int32)p_img_content.Size, (int32*)&out_size->x, (int32*)&out_size->y, &l_channel_number, STBI_rgb_alpha);
        out_pixels->Capacity = 4 * out_size->x * out_size->y;
        out_size->z = 1;
        *out_channel_nb = 4;

#if MEM_LEAK_DETECTION
        push_ptr_to_tracked(out_pixels->Memory);
#endif
    };
};