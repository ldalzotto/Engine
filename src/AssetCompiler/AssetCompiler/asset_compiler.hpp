#pragma once

#include "AssetRessource/asset_ressource.hpp"
#include "./asset_metadata.hpp"
#include "./asset_types_json.hpp"
#include "./obj_compiler.hpp"
#include "./img_compiler.hpp"
#include "./shader_compiler.hpp"

struct AssetCompiled
{
    AssetType type;
    Span<int8> compiled_data;

    inline static AssetCompiled build(const AssetType p_type, const Span<int8>& p_compiled_data)
    {
        return AssetCompiled{p_type, p_compiled_data};
    };

    inline static AssetCompiled build_default()
    {
        return AssetCompiled{AssetType::UNDEFINED, Span<int8>::build_default()};
    };

    inline void free()
    {
        this->compiled_data.free();
    };
};

inline int8 AssetCompiler_compile_single_file(ShaderCompiler& p_shader_compiler, const File& p_asset_file, AssetCompiled* out_asset_compiled)
{
    Slice<int8> l_asset_path = p_asset_file.path_slice;

    uimax l_last_dot_index = -1;
    while (l_asset_path.find(Slice_int8_build_rawstr("."), &l_last_dot_index))
    {
        Slice_slide(&l_asset_path, l_last_dot_index + 1);
    };

    if (l_asset_path.Size > 0)
    {
        if (l_asset_path.compare(Slice_int8_build_rawstr("vert")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            l_buffer.get(l_buffer.Capacity - 1) = (int8)NULL;
            ShaderCompiled l_compiled_shader;
            int l_compilation_result = p_shader_compiler.compile_shader_silent(ShaderModuleStage::VERTEX, l_buffer.slice, &l_compiled_shader);
            l_buffer.free();
            if (l_compilation_result)
            {
                Span<int8> l_compiled_buffer = Span<int8>::allocate_slice(l_compiled_shader.get_compiled_binary());
                l_compiled_shader.free();
                *out_asset_compiled = AssetCompiled::build(AssetType::SHADER_MODULE, l_compiled_buffer);
                return 1;
            };
        }
        else if (l_asset_path.compare(Slice_int8_build_rawstr("frag")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            l_buffer.get(l_buffer.Capacity - 1) = (int8)NULL;
            ShaderCompiled l_compiled_shader;
            int l_compilation_result = p_shader_compiler.compile_shader_silent(ShaderModuleStage::FRAGMENT, l_buffer.slice, &l_compiled_shader);
            l_buffer.free();
            if (l_compilation_result)
            {
                Span<int8> l_compiled_buffer = Span<int8>::allocate_slice(l_compiled_shader.get_compiled_binary());
                l_compiled_shader.free();
                *out_asset_compiled = AssetCompiled::build(AssetType::SHADER_MODULE, l_compiled_buffer);
                return 1;
            }
        }
        else if (l_asset_path.compare(Slice_int8_build_rawstr("obj")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            Vector<Vertex> l_vertices = Vector<Vertex>::allocate(0);
            Vector<uint32> l_indices = Vector<uint32>::allocate(0);
            ObjCompiler::ReadObj(l_buffer.slice, l_vertices, l_indices);

            MeshRessource::Asset l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices.to_slice(), l_indices.to_slice()});

            l_vertices.free();
            l_indices.free();
            l_buffer.free();
            *out_asset_compiled = AssetCompiled::build(AssetType::MESH, l_mesh_asset.allocated_binary);
            return 1;
        }
        else if (l_asset_path.compare(Slice_int8_build_rawstr("jpg")) || l_asset_path.compare(Slice_int8_build_rawstr("png")))
        {
            Span<int8> l_buffer = p_asset_file.read_file_allocate();

            v3ui l_size;
            int8 l_channel_nb;
            Span<int8> l_pixels;
            ImgCompiler::compile(l_buffer.slice, &l_size, &l_channel_nb, &l_pixels);

            TextureRessource::Asset l_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{l_size, l_channel_nb, l_pixels.slice});

            l_pixels.free();
            l_buffer.free();
            *out_asset_compiled = AssetCompiled::build(AssetType::TEXTURE, l_texture_asset.allocated_binary);
            return 1;
        }
        else if (l_asset_path.compare(Slice_int8_build_rawstr("json")))
        {
            AssetCompiled l_compiled_asset = AssetCompiled::build_default();
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            Vector<int8> l_buffer_vector = Vector<int8>{l_buffer.Capacity, l_buffer};
            JSONDeserializer l_json_deserializer = JSONDeserializer::start(l_buffer_vector);
            JSONDeserializer l_json_value_deserializer;

            l_compiled_asset.type = AssetJSON::get_value_of_asset_json(&l_json_deserializer, &l_json_value_deserializer);
            switch (l_compiled_asset.type)
            {
            case AssetType::SHADER:
            {
                l_compiled_asset.compiled_data = ShaderAssetJSON::allocate_asset_from_json(&l_json_value_deserializer);
            }
            break;
            case AssetType::MATERIAL:
            {
                l_compiled_asset.compiled_data = MaterialAssetJSON::allocate_asset_from_json(&l_json_value_deserializer);
            }
            break;
            default:
                abort();
            }

            l_json_value_deserializer.free();
            l_json_deserializer.free();
            l_buffer.free();
            *out_asset_compiled = l_compiled_asset;
            return 1;
        }
    }

    return 0;
};

inline Span<int8> AssetCompiler_compile_dependencies_of_file(ShaderCompiler& p_shader_compiler, const Slice<int8>& p_root_path, const File& p_asset_file)
{

    Slice<int8> l_asset_path = p_asset_file.path_slice;

    uimax l_last_dot_index = -1;
    while (l_asset_path.find(Slice_int8_build_rawstr("."), &l_last_dot_index))
    {
        Slice_slide(&l_asset_path, l_last_dot_index + 1);
    };

    if (l_asset_path.Size > 0)
    {
        if (l_asset_path.compare(Slice_int8_build_rawstr("json")))
        {
            Span<int8> l_compiled_dependencies;
            Span<int8> l_buffer = p_asset_file.read_file_allocate();
            Vector<int8> l_buffer_vector = Vector<int8>{l_buffer.Capacity, l_buffer};
            JSONDeserializer l_json_deserializer = JSONDeserializer::start(l_buffer_vector);

            JSONDeserializer l_json_value_deserializer;
            switch (AssetJSON::get_value_of_asset_json(&l_json_deserializer, &l_json_value_deserializer))
            {
            case AssetType::SHADER:
            {
                l_compiled_dependencies = ShaderAssetJSON::allocate_dependencies_from_json(&l_json_value_deserializer).allocated_binary;
            }
            break;
            case AssetType::MATERIAL:
            {
                l_compiled_dependencies = MaterialAssetJSON::allocate_dependencies_from_json(&l_json_value_deserializer, p_root_path).allocated_binary;
            }
            break;
            default:
                abort();
            }

            l_json_value_deserializer.free();
            l_json_deserializer.free();
            l_buffer.free();
            return l_compiled_dependencies;
        }
    }

    return Span<int8>::build_default();
};

inline int8 AssetCompiler_compile_and_push_to_database_single_file(ShaderCompiler& p_shader_compiler, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database,
                                                                   AssetMetadataDatabase& p_asset_metadata_database, const Slice<int8>& p_root_path, const Slice<int8>& p_relative_asset_path)
{
    int8 l_return = 0;
    Span<int8> l_asset_full_path = Span<int8>::allocate_slice_3(p_root_path, p_relative_asset_path, Slice_build_begin_end<int8>("\0", 0, 1));
    File l_asset_file = File::open_silent(l_asset_full_path.slice);
    if (l_asset_file.is_valid())
    {
        if (p_asset_metadata_database.does_assetmetadata_exists(p_database_connection, HashSlice(p_relative_asset_path)))
        {
            AssetMetadataDatabase::MetadataTS l_ts = p_asset_metadata_database.get_timestamps(p_database_connection, p_relative_asset_path);
            if (l_ts.file_modification_ts >= l_asset_file.get_modification_ts())
            {
                l_return = 1;
            }
        }
        if (!l_return)
        {
            AssetCompiled l_compiled_asset;
            if (AssetCompiler_compile_single_file(p_shader_compiler, l_asset_file, &l_compiled_asset))
            {
                p_asset_database.insert_or_update_asset_blob(p_database_connection, p_relative_asset_path, l_compiled_asset.compiled_data.slice);
                p_asset_metadata_database.insert_or_update_metadata(p_database_connection, p_relative_asset_path, AssetType_getName(l_compiled_asset.type), l_asset_file.get_modification_ts(),
                                                                    clock_currenttime_mics());

                l_compiled_asset.free();

                Span<int8> l_compiled_dependencies = AssetCompiler_compile_dependencies_of_file(p_shader_compiler, p_root_path, l_asset_file);
                if (l_compiled_dependencies.Memory)
                {
                    p_asset_database.insert_or_update_asset_dependencies(p_database_connection, p_relative_asset_path, l_compiled_dependencies.slice);
                    l_compiled_dependencies.free();
                }
                l_return = 1;
            }
        }

        l_asset_file.free();
    }
    l_asset_full_path.free();

    return l_return;
};


#include "./asset_compiler_pass.hpp"