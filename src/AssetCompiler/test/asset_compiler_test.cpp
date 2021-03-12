
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_database_test_utils.hpp"

inline void shader_module_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("shad.frag"));

    {
        Span<int8> l_raw_file = AssetCompiler_open_and_read_asset_file(l_asset_root_path.slice, slice_int8_build_rawstr("shad.frag"));
        l_raw_file.get(l_raw_file.Capacity - 1) = (int8)NULL;
        ShaderCompiled l_shader = p_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, l_raw_file.slice);

        l_raw_file.free();

        Span<int8> l_shader_module_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("shad.frag")));

        assert_true(l_shader_module_compiled.slice.compare(l_shader.get_compiled_binary()));

        l_shader_module_compiled.free();
        l_shader.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

inline void shader_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("shader_asset_test.json"));

    {
        Span<int8> l_shader_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));
        v2::ShaderRessource::Asset::Value l_shader_value = v2::ShaderRessource::Asset::Value::build_from_asset(v2::ShaderRessource::Asset::build_from_binary(l_shader_ressource_compiled));

        assert_true(l_shader_value.execution_order == 1001);
        assert_true(l_shader_value.shader_configuration.zwrite == 0);
        assert_true(l_shader_value.shader_configuration.ztest == v2::ShaderConfiguration::CompareOp::Always);
        assert_true(l_shader_value.specific_parameters.compare(SliceN<v2::ShaderLayoutParameterType, 1>{v2::ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice()));

        l_shader_ressource_compiled.free();
    }
    {
        Span<int8> l_shader_dependencies_compiled = l_asset_database.get_asset_dependencies_blob(HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));
        v2::ShaderRessource::AssetDependencies::Value l_shader_dependencies =
            v2::ShaderRessource::AssetDependencies::Value::build_from_asset(v2::ShaderRessource::AssetDependencies{l_shader_dependencies_compiled});

        assert_true(l_shader_dependencies.vertex_module == HashSlice(slice_int8_build_rawstr("shad.vert")));
        assert_true(l_shader_dependencies.fragment_module == HashSlice(slice_int8_build_rawstr("shad.frag")));

        l_shader_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

inline void material_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("material_asset_test.json"));

    {
        Span<int8> l_material_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("material_asset_test.json")));
        v2::MaterialRessource::Asset::Value l_material_value = v2::MaterialRessource::Asset::Value::build_from_asset(v2::MaterialRessource::Asset::build_from_binary(l_material_ressource_compiled));

        assert_true(l_material_value.parameters.parameters.get_size() == 5);
        {
            hash_t l_texture_hash = HashSlice(slice_int8_build_rawstr("link_to_png.png"));
            assert_true(l_texture_hash == *l_material_value.parameters.get_parameter_texture_gpu_value(0));
        }
        {
            float32 l_param_value = slice_cast<float32>(l_material_value.parameters.get_parameter_uniform_host_value(1)).get(0);
            assert_true(l_param_value == 1294.0f);
        }
        {
            v2f l_param_value = slice_cast<v2f>(l_material_value.parameters.get_parameter_uniform_host_value(2)).get(0);
            assert_true(l_param_value == v2f{1294.0f, 1295.0f});
        }
        {
            v3f l_param_value = slice_cast<v3f>(l_material_value.parameters.get_parameter_uniform_host_value(3)).get(0);
            assert_true(l_param_value == v3f{1294.0f, 1295.0f, 1296.0f});
        }
        {
            v4f l_param_value = slice_cast<v4f>(l_material_value.parameters.get_parameter_uniform_host_value(4)).get(0);
            assert_true(l_param_value == v4f{1294.0f, 1295.0f, 1296.0f, 1297.0f});
        }
        l_material_ressource_compiled.free();
    }
    {
        Span<int8> l_material_dependencies_compiled = l_asset_database.get_asset_dependencies_blob(HashSlice(slice_int8_build_rawstr("material_asset_test.json")));
        v2::MaterialRessource::AssetDependencies::Value l_material_dependencies =
            v2::MaterialRessource::AssetDependencies::Value::build_from_asset(v2::MaterialRessource::AssetDependencies{l_material_dependencies_compiled});

        assert_true(l_material_dependencies.shader == HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));
        assert_true(l_material_dependencies.textures.Size == 1);
        assert_true(l_material_dependencies.textures.get(0) == HashSlice(slice_int8_build_rawstr("link_to_png.png")));

        l_material_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

inline void mesh_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("cube.obj"));
    {
        Span<int8> l_mesh_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("cube.obj")));
        v2::MeshRessource::Asset::Value l_mesh_value = v2::MeshRessource::Asset::Value::build_from_asset(v2::MeshRessource::Asset::build_from_binary(l_mesh_ressource_compiled));

        v3f l_positions[8] = {v3f{-1.0f, -1.0f, 1.0f}, v3f{-1.0f, 1.0f, 1.0f}, v3f{-1.0f, -1.0f, -1.0f}, v3f{-1.0f, 1.0f, -1.0f},
                              v3f{1.0f, -1.0f, 1.0f},  v3f{1.0f, 1.0f, 1.0f},  v3f{1.0f, -1.0f, -1.0f},  v3f{1.0f, 1.0f, -1.0f}};

        v2f l_uvs[14] = {v2f{0.625f, 0.0f},  v2f{0.375f, 0.25f}, v2f{0.375f, 0.0f},  v2f{0.625f, 0.25f}, v2f{0.375f, 0.5f},  v2f{0.625f, 0.5f},  v2f{0.375f, 0.75f},
                         v2f{0.625f, 0.75f}, v2f{0.375f, 1.00f}, v2f{0.125f, 0.75f}, v2f{0.125f, 0.50f}, v2f{0.875f, 0.50f}, v2f{0.625f, 1.00f}, v2f{0.875f, 0.75f}};

        for(loop(i, 0, 14))
        {
            l_uvs[i].y = 1.0f - l_uvs[i].y;
        }

        SliceN<Vertex, 14> l_vertices = SliceN<Vertex, 14>{Vertex{l_positions[1], l_uvs[0]},  Vertex{l_positions[2], l_uvs[1]}, Vertex{l_positions[0], l_uvs[2]},  Vertex{l_positions[3], l_uvs[3]},
                                 Vertex{l_positions[6], l_uvs[4]},  Vertex{l_positions[7], l_uvs[5]}, Vertex{l_positions[4], l_uvs[6]},  Vertex{l_positions[5], l_uvs[7]},
                                 Vertex{l_positions[0], l_uvs[8]},  Vertex{l_positions[0], l_uvs[9]}, Vertex{l_positions[2], l_uvs[10]}, Vertex{l_positions[3], l_uvs[11]},
                                 Vertex{l_positions[1], l_uvs[12]}, Vertex{l_positions[1], l_uvs[13]}};
        SliceN<uint32, 36> l_indices = {0, 1, 2, 3, 4, 1, 5, 6, 4, 7, 8, 6, 4, 9, 10, 11, 7, 5, 0, 3, 1, 3, 5, 4, 5, 7, 6, 7, 12, 8, 4, 6, 9, 11, 13, 7};

        assert_true(l_mesh_value.initial_vertices.compare(l_vertices.to_slice()));
        assert_true(l_mesh_value.initial_indices.compare(l_indices.to_slice()));

        l_mesh_ressource_compiled.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

inline void texture_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("texture.png"));
    {
        Span<int8> l_texture_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("texture.png")));

        v2::TextureRessource::Asset::Value l_texture_value = v2::TextureRessource::Asset::Value::build_from_asset(v2::TextureRessource::Asset{l_texture_ressource_compiled});
        assert_true(l_texture_value.size == v3ui{4, 4, 1});
        assert_true(l_texture_value.channel_nb == 4);
        Slice<color> l_pixels = slice_cast<color>(l_texture_value.pixels);
        assert_true(l_pixels.Size == 16);
        Slice<color> l_awaited_pixels = SliceN<color, 16>{color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                                          color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                                          color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                                          color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX}}
                                            .to_slice();

        assert_true(l_pixels.compare(l_awaited_pixels));
        l_texture_ressource_compiled.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
}

int main()
{
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

    shader_module_compilation(l_shader_compiler);
    shader_asset_compilation(l_shader_compiler);
    material_asset_compilation(l_shader_compiler);
    mesh_asset_compilation(l_shader_compiler);
    texture_asset_compilation(l_shader_compiler);
    
    l_shader_compiler.free();

    memleak_ckeck();
}