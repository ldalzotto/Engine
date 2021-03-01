#pragma once

#include "AssetRessource/asset_ressource.hpp"

using namespace v2;

struct AssetCompiler
{
};

inline void AssetCompiler_compile_single_file(AssetCompiler* thiz, const File& p_asset_file)
{
    Slice<int8> l_asset_path = p_asset_file.path_slice;

    uimax l_last_dot_index = -1;
    while (l_asset_path.find(slice_int8_build_rawstr("."), &l_last_dot_index))
    {
    };

    if (l_last_dot_index < l_asset_path.Size)
    {
        Slice<int8> l_extension = Slice<int8>::build(l_asset_path.Begin, l_last_dot_index, l_asset_path.Size);
        if (l_extension.compare(slice_int8_build_rawstr(".vert")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            l_buffer.free();
        }
        else if (l_extension.compare(slice_int8_build_rawstr(".frag")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            l_buffer.free();
        }
    }
};