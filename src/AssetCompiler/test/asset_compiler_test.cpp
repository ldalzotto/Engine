
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_metadata_database_test_utils.hpp"

struct AssetCompilerTestCtx
{
    DatabaseConnection connection;
    AssetDatabase asset_database;
    AssetMetadataDatabase assetmetadata_database;

    inline static AssetCompilerTestCtx allocate(const Slice<int8>& p_database_path)
    {
        AssetCompilerTestCtx l_ctx;
        l_ctx.connection = DatabaseConnection::allocate(p_database_path);
        l_ctx.asset_database = AssetDatabase::allocate(l_ctx.connection);
        l_ctx.assetmetadata_database = AssetMetadataDatabase::allocate(l_ctx.connection);
        return l_ctx;
    };

    inline void free()
    {
        this->assetmetadata_database.free(this->connection);
        this->asset_database.free(this->connection);
        this->connection.free();
    }
};

inline void asset_metatadata_get_by_type()
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Slice<int8> l_path_1 = slice_int8_build_rawstr("path1");
    Slice<int8> l_type_1 = slice_int8_build_rawstr("type1");
    time_t l_modification_ts_1 = 12;
    time_t l_insertion_ts_1 = 48;
    Slice<int8> l_path_2 = slice_int8_build_rawstr("path2");
    Slice<int8> l_type_2 = slice_int8_build_rawstr("type2");
    time_t l_modification_ts_2 = 471;
    time_t l_insertion_ts_2 = 889;
    Slice<int8> l_path_3 = slice_int8_build_rawstr("path3");
    time_t l_modification_ts_3 = 7105;
    time_t l_insertion_ts_3 = 411;

    l_ctx.assetmetadata_database.insert_or_update_metadata(l_ctx.connection, l_path_1, l_type_1, l_modification_ts_1, l_insertion_ts_1);
    l_ctx.assetmetadata_database.insert_or_update_metadata(l_ctx.connection, l_path_2, l_type_1, l_modification_ts_2, l_insertion_ts_2);
    l_ctx.assetmetadata_database.insert_or_update_metadata(l_ctx.connection, l_path_3, l_type_2, l_modification_ts_3, l_insertion_ts_3);

    {
        AssetMetadataDatabase::Paths l_paths = l_ctx.assetmetadata_database.get_all_path_from_type(l_ctx.connection, l_type_1);

        assert_true(l_paths.data_v2.get_size() == 2);
        assert_true(l_paths.data_v2.get(0).compare(l_path_1));
        assert_true(l_paths.data_v2.get(1).compare(l_path_2));

        l_paths.free();
    }

    l_ctx.free();
    l_asset_database_path.free();
};

inline void compile_modificationts_cache(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    String l_tmp_asset_path;
    File l_tmp_file;
    {
        String l_src_asset_path = String::allocate_elements_2(l_asset_root_path.slice, slice_int8_build_rawstr("material_asset_test.json"));

        l_tmp_asset_path = String::allocate_elements_2(l_asset_root_path.slice, slice_int8_build_rawstr("tmp.json"));
        l_tmp_file = File::create_or_open(l_tmp_asset_path.to_slice());
        File l_src_file = File::open(l_src_asset_path.to_slice());

        Span<int8> l_buf = l_src_file.read_file_allocate();
        l_tmp_file.write_file(l_buf.slice);

        l_buf.free();
        l_src_asset_path.free();
        l_src_file.free();
    }

    assert_true(AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                       slice_int8_build_rawstr("tmp.json")) == 1);
    AssetMetadataDatabase::MetadataTS l_ts_before = l_ctx.assetmetadata_database.get_timestamps(l_ctx.connection, slice_int8_build_rawstr("tmp.json"));
    assert_true(AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                       slice_int8_build_rawstr("tmp.json")) == 1);
    AssetMetadataDatabase::MetadataTS l_ts_after = l_ctx.assetmetadata_database.get_timestamps(l_ctx.connection, slice_int8_build_rawstr("tmp.json"));
    assert_true(l_ts_before.file_modification_ts == l_ts_after.file_modification_ts);
    assert_true(l_ts_before.insert_ts == l_ts_after.insert_ts);

// TODO -> having a better way to handle this ?
// We sleep to ensure that some time have elapsed
#if __linux
    Thread::sleep(Thread::get_current_thread(), 1000);
#endif

    // We simulate a write operation
    {
        String l_src_asset_path = String::allocate_elements_2(l_asset_root_path.slice, slice_int8_build_rawstr("material_asset_test.json"));
        File l_src_file = File::open(l_src_asset_path.to_slice());
        Span<int8> l_buf = l_src_file.read_file_allocate();
        l_tmp_file.write_file(l_buf.slice);
        l_buf.free();
        l_src_asset_path.free();
        l_src_file.free();
    }

    assert_true(AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                       slice_int8_build_rawstr("tmp.json")) == 1);
    AssetMetadataDatabase::MetadataTS l_ts_after_2 = l_ctx.assetmetadata_database.get_timestamps(l_ctx.connection, slice_int8_build_rawstr("tmp.json"));
    assert_true(l_ts_after_2.file_modification_ts != l_ts_after.file_modification_ts);
    assert_true(l_ts_after_2.insert_ts != l_ts_after.insert_ts);

    l_tmp_file.erase();
    l_tmp_asset_path.free();

    l_ctx.free();
    l_asset_database_path.free();
    l_asset_root_path.free();
};

inline void compile_invalid_file(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));

    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    int8 l_compilation_result = AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                                       slice_int8_build_rawstr("aeiajeoijaef"));

    assert_true(l_compilation_result == 0);

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void shader_module_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));

    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           slice_int8_build_rawstr("shad.frag"));

    {
        Span<int8> l_raw_file = AssetCompiler_open_and_read_asset_file(l_asset_root_path.slice, slice_int8_build_rawstr("shad.frag"));
        l_raw_file.get(l_raw_file.Capacity - 1) = (int8)NULL;
        ShaderCompiled l_shader = p_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, l_raw_file.slice);

        l_raw_file.free();

        Span<int8> l_shader_module_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shad.frag")));
        assert_true(l_shader_module_compiled.slice.compare(l_shader.get_compiled_binary()));

        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shad.frag")));
        assert_true(l_metadata.type.slice.compare(AssetType_Const::SHADER_MODULE_NAME));
        assert_true(l_metadata.path.slice.compare(slice_int8_build_rawstr("shad.frag")));
        l_metadata.free();

        l_shader_module_compiled.free();
        l_shader.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void shader_module_error_compilation(ShaderCompiler& p_shader_compiler)
{
    printf("shader_module_error_compilation - BEGIN - error message awaited \n");
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));

    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           slice_int8_build_rawstr("shad_wrong.frag"));

    {
        assert_true(l_ctx.asset_database.does_asset_exists(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shad_wrong.frag"))) == 0);
        assert_true(l_ctx.assetmetadata_database.does_assetmetadata_exists(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shad_wrong.frag"))) == 0);
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();

    printf("shader_module_error_compilation - END\n");
};

inline void shader_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           slice_int8_build_rawstr("shader_asset_test.json"));

    {
        Span<int8> l_shader_resource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shader_asset_test.json")));
        ShaderResource::Asset::Value l_shader_value = ShaderResource::Asset::Value::build_from_asset(ShaderResource::Asset::build_from_binary(l_shader_resource_compiled));
        SliceN<ShaderLayoutParameterType, 1> l_shader_layout_arr{ShaderLayoutParameterType::TEXTURE_FRAGMENT};

        assert_true(l_shader_value.execution_order == 1001);
        assert_true(l_shader_value.shader_configuration.zwrite == 0);
        assert_true(l_shader_value.shader_configuration.ztest == ShaderConfiguration::CompareOp::Always);
        assert_true(l_shader_value.specific_parameters.compare(slice_from_slicen(&l_shader_layout_arr)));

        l_shader_resource_compiled.free();

        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shader_asset_test.json")));
        assert_true(l_metadata.type.slice.compare(AssetType_Const::SHADER_NAME));
        assert_true(l_metadata.path.slice.compare(slice_int8_build_rawstr("shader_asset_test.json")));
        l_metadata.free();
    }
    {
        Span<int8> l_shader_dependencies_compiled = l_ctx.asset_database.get_asset_dependencies_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("shader_asset_test.json")));
        ShaderResource::AssetDependencies::Value l_shader_dependencies = ShaderResource::AssetDependencies::Value::build_from_asset(ShaderResource::AssetDependencies{l_shader_dependencies_compiled});

        assert_true(l_shader_dependencies.vertex_module == HashFunctions::hash(slice_int8_build_rawstr("shad.vert")));
        assert_true(l_shader_dependencies.fragment_module == HashFunctions::hash(slice_int8_build_rawstr("shad.frag")));

        l_shader_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void material_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           slice_int8_build_rawstr("material_asset_test.json"));

    {
        Span<int8> l_material_resource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("material_asset_test.json")));
        MaterialResource::Asset::Value l_material_value = MaterialResource::Asset::Value::build_from_asset(MaterialResource::Asset::build_from_binary(l_material_resource_compiled));

        assert_true(l_material_value.parameters.parameters.get_size() == 5);
        {
            hash_t l_texture_hash = HashFunctions::hash(slice_int8_build_rawstr("link_to_png.png"));
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
        l_material_resource_compiled.free();
    }
    {
        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("material_asset_test.json")));
        assert_true(l_metadata.type.slice.compare(AssetType_Const::MATERIAL_NAME));
        assert_true(l_metadata.path.slice.compare(slice_int8_build_rawstr("material_asset_test.json")));
        l_metadata.free();
    }
    {
        Span<int8> l_material_dependencies_compiled = l_ctx.asset_database.get_asset_dependencies_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("material_asset_test.json")));
        MaterialResource::AssetDependencies::Value l_material_dependencies =
            MaterialResource::AssetDependencies::Value::build_from_asset(MaterialResource::AssetDependencies{l_material_dependencies_compiled});

        assert_true(l_material_dependencies.shader == HashFunctions::hash(slice_int8_build_rawstr("shader_asset_test.json")));
        assert_true(l_material_dependencies.textures.Size == 1);
        assert_true(l_material_dependencies.textures.get(0) == HashFunctions::hash(slice_int8_build_rawstr("link_to_png.png")));

        l_material_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void mesh_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           slice_int8_build_rawstr("cube.obj"));
    {
        Span<int8> l_mesh_resource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("cube.obj")));
        MeshResource::Asset::Value l_mesh_value = MeshResource::Asset::Value::build_from_asset(MeshResource::Asset::build_from_binary(l_mesh_resource_compiled));

        v3f l_positions[8] = {v3f{-1.0f, -1.0f, 1.0f}, v3f{-1.0f, 1.0f, 1.0f}, v3f{-1.0f, -1.0f, -1.0f}, v3f{-1.0f, 1.0f, -1.0f},
                              v3f{1.0f, -1.0f, 1.0f},  v3f{1.0f, 1.0f, 1.0f},  v3f{1.0f, -1.0f, -1.0f},  v3f{1.0f, 1.0f, -1.0f}};

        v2f l_uvs[14] = {v2f{0.625f, 0.0f},  v2f{0.375f, 0.25f}, v2f{0.375f, 0.0f},  v2f{0.625f, 0.25f}, v2f{0.375f, 0.5f},  v2f{0.625f, 0.5f},  v2f{0.375f, 0.75f},
                         v2f{0.625f, 0.75f}, v2f{0.375f, 1.00f}, v2f{0.125f, 0.75f}, v2f{0.125f, 0.50f}, v2f{0.875f, 0.50f}, v2f{0.625f, 1.00f}, v2f{0.875f, 0.75f}};

        for (loop(i, 0, 14))
        {
            l_uvs[i].y = 1.0f - l_uvs[i].y;
        }

        SliceN<Vertex, 14> l_vertices = SliceN<Vertex, 14>{Vertex{l_positions[1], l_uvs[0]},  Vertex{l_positions[2], l_uvs[1]}, Vertex{l_positions[0], l_uvs[2]},  Vertex{l_positions[3], l_uvs[3]},
                                                           Vertex{l_positions[6], l_uvs[4]},  Vertex{l_positions[7], l_uvs[5]}, Vertex{l_positions[4], l_uvs[6]},  Vertex{l_positions[5], l_uvs[7]},
                                                           Vertex{l_positions[0], l_uvs[8]},  Vertex{l_positions[0], l_uvs[9]}, Vertex{l_positions[2], l_uvs[10]}, Vertex{l_positions[3], l_uvs[11]},
                                                           Vertex{l_positions[1], l_uvs[12]}, Vertex{l_positions[1], l_uvs[13]}};
        SliceN<uint32, 36> l_indices = {0, 1, 2, 3, 4, 1, 5, 6, 4, 7, 8, 6, 4, 9, 10, 11, 7, 5, 0, 3, 1, 3, 5, 4, 5, 7, 6, 7, 12, 8, 4, 6, 9, 11, 13, 7};

        assert_true(l_mesh_value.initial_vertices.compare(slice_from_slicen(&l_vertices)));
        assert_true(l_mesh_value.initial_indices.compare(slice_from_slicen(&l_indices)));

        l_mesh_resource_compiled.free();
    }
    {
        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("cube.obj")));
        assert_true(l_metadata.type.slice.compare(AssetType_Const::MESH_NAME));
        assert_true(l_metadata.path.slice.compare(slice_int8_build_rawstr("cube.obj")));
        l_metadata.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void texture_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           slice_int8_build_rawstr("texture.png"));
    {
        Span<int8> l_texture_resource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("texture.png")));

        TextureResource::Asset::Value l_texture_value = TextureResource::Asset::Value::build_from_asset(TextureResource::Asset{l_texture_resource_compiled});
        assert_true(l_texture_value.size == v3ui{4, 4, 1});
        assert_true(l_texture_value.channel_nb == 4);
        Slice<color> l_pixels = slice_cast<color>(l_texture_value.pixels);
        assert_true(l_pixels.Size == 16);
        SliceN<color, 16> l_awaited_pixels_arr{color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                               color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                               color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                               color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX}};

        assert_true(l_pixels.compare(slice_from_slicen(&l_awaited_pixels_arr)));
        l_texture_resource_compiled.free();
    }
    {
        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashFunctions::hash(slice_int8_build_rawstr("texture.png")));
        assert_true(l_metadata.type.slice.compare(AssetType_Const::TEXTURE_NAME));
        assert_true(l_metadata.path.slice.compare(slice_int8_build_rawstr("texture.png")));
        l_metadata.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
}

inline void compilation_pass(ShaderCompiler& p_shader_compiler)
{
    String l_db_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset.db"));
    String l_db_2_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset_2.db"));
    {
        File l_db = File::create_or_open(l_db_path.to_slice());
        l_db.erase();
    }

    AssetCompilerPassHeap l_heap = AssetCompilerPassHeap::allocate();
    AssetCompilerConfigurationJSON l_asset_compiler_configuration{};

    AssetCompilationPassStatic l_static_pass = AssetCompilationPassStatic::allocate_default();
    l_static_pass.root_path.append(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_static_pass.assets_to_compile.push_back_element(String::allocate_elements(slice_int8_build_rawstr("cube.obj")));
    l_asset_compiler_configuration.static_passes.push_back_element(l_static_pass);

    AssetCompilationPass l_pass_1 = AssetCompilationPass::allocate_default();
    l_pass_1.root_path.append(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_pass_1.database_path.append(l_db_path.to_slice());
    l_pass_1.assets_to_compile.push_back_element(String::allocate_elements(slice_int8_build_rawstr("material_asset_test.json")));

    AssetCompilationPass l_pass_2 = AssetCompilationPass::allocate_default();
    l_pass_2.root_path.append(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_pass_2.database_path.append(l_db_2_path.to_slice());
    l_pass_2.assets_to_compile.push_back_element(String::allocate_elements(slice_int8_build_rawstr("material_asset_test.json")));
    l_pass_2.assets_to_compile.push_back_element(String::allocate_elements(slice_int8_build_rawstr("texture.png")));

    l_asset_compiler_configuration.local_passes.push_back_element(l_pass_1);
    l_asset_compiler_configuration.local_passes.push_back_element(l_pass_2);

    Vector<Token<AssetCompilationPass>> l_passes = Vector<Token<AssetCompilationPass>>::allocate(0);
    AssetCompilerPassComposition::allocate_passes_from_inline(l_heap, l_asset_compiler_configuration, slice_int8_build_rawstr(ASSET_FOLDER_PATH), &l_passes);

    assert_true(l_passes.Size == 4);

    for (loop(i, 0, l_passes.Size))
    {
        AssetCompilerPassComposition::ExecutionState l_execution = AssetCompilerPassComposition::ExecutionState::allocate(p_shader_compiler, l_heap.asset_compilation_passes.get(l_passes.get(i)));
        l_execution.compile_all();
        l_execution.free();

        l_heap.free_asset_compiler_pass(l_passes.get(i));
    }

    {
        DatabaseConnection l_db_connection = DatabaseConnection::allocate(l_db_path.to_slice());
        AssetDatabase l_asset_db = AssetDatabase::allocate(l_db_connection);
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashFunctions::hash(slice_int8_build_rawstr("cube.obj"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashFunctions::hash(slice_int8_build_rawstr("material_asset_test.json"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashFunctions::hash(slice_int8_build_rawstr("texture.png"))) == 0);
        l_asset_db.free(l_db_connection);
        l_db_connection.free();
    }
    {
        DatabaseConnection l_db_connection = DatabaseConnection::allocate(l_db_2_path.to_slice());
        AssetDatabase l_asset_db = AssetDatabase::allocate(l_db_connection);
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashFunctions::hash(slice_int8_build_rawstr("cube.obj"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashFunctions::hash(slice_int8_build_rawstr("material_asset_test.json"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashFunctions::hash(slice_int8_build_rawstr("texture.png"))));
        l_asset_db.free(l_db_connection);
        l_db_connection.free();
    }

    {
        File l_db = File::create_or_open(l_db_path.to_slice());
        l_db.erase();
    }
    {
        File l_db = File::create_or_open(l_db_2_path.to_slice());
        l_db.erase();
    }

    l_passes.free();
    l_heap.free();
    l_db_path.free();
    l_db_2_path.free();
};

inline void compilation_pass_configuration_serialization()
{
    struct AssetCompilationPassAsserter
    {
        inline static void _assert(const AssetCompilationPass& p_pass, const Slice<int8>& p_root_path, const Slice<int8>& p_database_path, const Slice<Slice<int8>>& p_asset_to_compiles)
        {
            assert_true(p_root_path.compare(p_pass.root_path.to_slice()));
            assert_true(p_database_path.compare(p_pass.database_path.to_slice()));
            assert_true(p_pass.assets_to_compile.Size == p_asset_to_compiles.Size);
            for (loop(i, 0, p_pass.assets_to_compile.Size))
            {
                assert_true(p_asset_to_compiles.get(i).compare(p_pass.assets_to_compile.get(i).to_slice()));
            }
        };
    };

    struct AssetsToCompile
    {
        Vector<Span<int8>> assets_to_compile;

        inline static AssetsToCompile allocate_default()
        {
            return AssetsToCompile{Vector<Span<int8>>::allocate(0)};
        };

        inline void push_asset(const Span<int8>& p_asset)
        {
            this->assets_to_compile.push_back_element(p_asset);
        }

        inline Slice<Slice<int8>> to_slice()
        {
            Slice<Span<int8>> l_slice = this->assets_to_compile.to_slice();
            return *(Slice<Slice<int8>>*)&l_slice;
        };

        inline void clear()
        {
            for (loop(i, 0, this->assets_to_compile.Size))
            {
                this->assets_to_compile.get(i).free();
            }
            this->assets_to_compile.clear();
        };
        inline void free()
        {
            for (loop(i, 0, this->assets_to_compile.Size))
            {
                this->assets_to_compile.get(i).free();
            }
            this->assets_to_compile.free();
        };
    };

    String l_configuration_file_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("compilation_pass_configuration_serialization.json"));

    const int8* l_path_absolute_prefix = "absolute_prefix/";
    String l_common_asset_path = String::allocate_elements_2(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("common/"));

    const int8* l_asset_file = "asset.db";

    AssetCompilerConfigurationJSON l_configuration_json =
        AssetCompilerConfigurationJSON::allocate_from_json_file(l_configuration_file_path.to_slice(), slice_int8_build_rawstr(l_path_absolute_prefix));
    Vector<AssetCompilationPass> l_asset_configuration_passes = l_configuration_json.consume_and_merge_to_passes();

    assert_true(l_asset_configuration_passes.Size == 2 * 3);

    AssetsToCompile l_assets_to_compile_test = AssetsToCompile::allocate_default();
    {
        Span<int8> l_asset_database_path = Span<int8>::allocate_slice_3(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("pass_1/"), slice_int8_build_rawstr(l_asset_file));
        Span<int8> l_pass_1_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("pass_1/"));

        AssetCompilationPass& l_pass_0 = l_asset_configuration_passes.get(0);
        AssetCompilationPass& l_pass_1 = l_asset_configuration_passes.get(1);

        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("common1")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("common2")));
        AssetCompilationPassAsserter::_assert(l_pass_0, l_common_asset_path.to_slice(), l_asset_database_path.slice, l_assets_to_compile_test.to_slice());
        l_assets_to_compile_test.clear();
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("file_1")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("file_2")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("file_3")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("file_4")));
        AssetCompilationPassAsserter::_assert(l_pass_1, l_pass_1_root_path.slice, l_asset_database_path.slice, l_assets_to_compile_test.to_slice());

        l_assets_to_compile_test.clear();
        l_pass_1_root_path.free();
        l_asset_database_path.free();
    }
    {
        Span<int8> l_asset_database_path = Span<int8>::allocate_slice_3(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("pass_2/"), slice_int8_build_rawstr(l_asset_file));
        Span<int8> l_pass_1_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("pass_2/"));

        AssetCompilationPass& l_pass_2 = l_asset_configuration_passes.get(2);
        AssetCompilationPass& l_pass_3 = l_asset_configuration_passes.get(3);

        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("common1")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("common2")));
        AssetCompilationPassAsserter::_assert(l_pass_2, l_common_asset_path.to_slice(), l_asset_database_path.slice, l_assets_to_compile_test.to_slice());
        l_assets_to_compile_test.clear();
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("file_1")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("file_2")));
        AssetCompilationPassAsserter::_assert(l_pass_3, l_pass_1_root_path.slice, l_asset_database_path.slice, l_assets_to_compile_test.to_slice());

        l_assets_to_compile_test.clear();
        l_pass_1_root_path.free();
        l_asset_database_path.free();
    }
    {
        Span<int8> l_asset_database_path = Span<int8>::allocate_slice_3(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("pass_3/"), slice_int8_build_rawstr(l_asset_file));
        Span<int8> l_pass_1_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(l_path_absolute_prefix), slice_int8_build_rawstr("pass_3/"));

        AssetCompilationPass& l_pass_4 = l_asset_configuration_passes.get(4);
        AssetCompilationPass& l_pass_5 = l_asset_configuration_passes.get(5);

        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("common1")));
        l_assets_to_compile_test.push_asset(Span<int8>::allocate_slice(slice_int8_build_rawstr("common2")));
        AssetCompilationPassAsserter::_assert(l_pass_4, l_common_asset_path.to_slice(), l_asset_database_path.slice, l_assets_to_compile_test.to_slice());
        l_assets_to_compile_test.clear();
        AssetCompilationPassAsserter::_assert(l_pass_5, l_pass_1_root_path.slice, l_asset_database_path.slice, l_assets_to_compile_test.to_slice());

        l_assets_to_compile_test.clear();
        l_pass_1_root_path.free();
        l_asset_database_path.free();
    }

    l_assets_to_compile_test.free();

    for (loop(i, 0, l_asset_configuration_passes.Size))
    {
        l_asset_configuration_passes.get(i).free();
    }
    l_asset_configuration_passes.free();

    l_common_asset_path.free();
    l_configuration_file_path.free();
};

int main()
{
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

    asset_metatadata_get_by_type();

    compile_modificationts_cache(l_shader_compiler);
    compile_invalid_file(l_shader_compiler);
    shader_module_compilation(l_shader_compiler);
    shader_module_error_compilation(l_shader_compiler);
    shader_asset_compilation(l_shader_compiler);
    material_asset_compilation(l_shader_compiler);
    mesh_asset_compilation(l_shader_compiler);
    texture_asset_compilation(l_shader_compiler);

    compilation_pass(l_shader_compiler);

    l_shader_compiler.free();

    compilation_pass_configuration_serialization();

    memleak_ckeck();
}

#include "Common2/common2_external_implementation.hpp"
#include "GPU/gpu_external_implementation.hpp"