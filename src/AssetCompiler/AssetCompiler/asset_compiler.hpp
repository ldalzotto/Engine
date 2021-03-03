#pragma once

#include "AssetRessource/asset_ressource.hpp"

#include "./shader_compiler.hpp"

using namespace v2;

// TODO -> inserting asset dependencies

// TODO -> handling errors by using the shader compiler silent :)
inline Span<int8> AssetCompiler_compile_single_file(ShaderCompiler& p_shader_compiler, const File& p_asset_file)
{
    Slice<int8> l_asset_path = p_asset_file.path_slice;

    uimax l_last_dot_index = -1;
    while (l_asset_path.find(slice_int8_build_rawstr("."), &l_last_dot_index))
    {
        l_asset_path.slide(l_last_dot_index + 1);
    };

    if (l_asset_path.Size > 0)
    {
        if (l_asset_path.compare(slice_int8_build_rawstr("vert")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            l_buffer.get(l_buffer.Capacity - 1) = (int8)NULL;
            ShaderCompiled l_compiled_shader = p_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, l_buffer.slice);
            Span<int8> l_compiled_buffer = Span<int8>::allocate_slice(l_compiled_shader.get_compiled_binary());
            l_compiled_shader.free();
            l_buffer.free();
            return l_compiled_buffer;
        }
        else if (l_asset_path.compare(slice_int8_build_rawstr("frag")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            l_buffer.get(l_buffer.Capacity - 1) = (int8)NULL;
            ShaderCompiled l_compiled_shader = p_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, l_buffer.slice);
            Span<int8> l_compiled_buffer = Span<int8>::allocate_slice(l_compiled_shader.get_compiled_binary());
            l_compiled_shader.free();
            l_buffer.free();
            return l_compiled_buffer;
        }
        else if (l_asset_path.compare(slice_int8_build_rawstr("json")))
        {
            return Span<int8>::build_default();
            // TODO we want to handle json asset files, texture and models
        }
    }

    return Span<int8>::build_default();
};

inline void AssetCompiler_compile_and_push_to_database_single_file(ShaderCompiler& p_shader_compiler, AssetDatabase& p_asset_database, const File& p_asset_file,
                                                                   const Slice<int8>& p_asset_path_used_for_asset_id)
{
    Span<int8> l_compiled_asset = AssetCompiler_compile_single_file(p_shader_compiler, p_asset_file);
    if (l_compiled_asset.Memory)
    {
        p_asset_database.insert_or_update_asset_blob(p_asset_path_used_for_asset_id, l_compiled_asset.slice);
        l_compiled_asset.free();
    }
};