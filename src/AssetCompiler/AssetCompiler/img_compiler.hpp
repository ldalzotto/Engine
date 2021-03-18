#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

struct ImgCompiler
{
    inline static void compile(const Slice<int8>& p_img_content, v3ui* out_size, int8* out_channel_nb, Span<int8>* out_pixels)
    {
        int32 l_channel_number;
        out_pixels->Memory = (int8*)stbi_load_from_memory((const uint8*)p_img_content.Begin, (int32)p_img_content.Size, (int32*)&out_size->x, (int32*)&out_size->y, &l_channel_number, STBI_rgb_alpha);
        out_pixels->Capacity = 4 * out_size->x * out_size->y;
        out_size->z = 1;
        *out_channel_nb = 4;

#if __MEMLEAK
        push_ptr_to_tracked(out_pixels->Memory);
#endif
    };

    inline static void write_to_image(const Slice<int8>& p_path, const uint32 p_width, const uint32 p_height, const int8 p_channel_number, const Slice<int8>& p_pixels)
    {
#if __DEBUG
        p_path.assert_null_terminated();
#endif
        stbi_write_png(p_path.Begin, p_width, p_height, p_channel_number, p_pixels.Begin, p_width * p_channel_number * sizeof(int8));
    };

    inline static Span<int8> read_image(const Slice<int8>& p_path)
    {
#if __DEBUG
        p_path.assert_null_terminated();
#endif

        Span<int8> l_out;
        int32 l_channel_number;
        v3ui l_size;
        l_out.Memory = (int8*)stbi_load(p_path.Begin, (int32*)&l_size.x, (int32*)&l_size.y, (int32*)&l_channel_number, 4);
        l_out.Capacity = 4 * l_size.x * l_size.y;
#if __MEMLEAK
        push_ptr_to_tracked(l_out.Memory);
#endif
        return l_out;
    };
};