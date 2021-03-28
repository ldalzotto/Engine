
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
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Slice<int8> l_path_1 = Slice_int8_build_rawstr("path1");
    Slice<int8> l_type_1 = Slice_int8_build_rawstr("type1");
    time_t l_modification_ts_1 = 12;
    time_t l_insertion_ts_1 = 48;
    Slice<int8> l_path_2 = Slice_int8_build_rawstr("path2");
    Slice<int8> l_type_2 = Slice_int8_build_rawstr("type2");
    time_t l_modification_ts_2 = 471;
    time_t l_insertion_ts_2 = 889;
    Slice<int8> l_path_3 = Slice_int8_build_rawstr("path3");
    time_t l_modification_ts_3 = 7105;
    time_t l_insertion_ts_3 = 411;

    l_ctx.assetmetadata_database.insert_or_update_metadata(l_ctx.connection, l_path_1, l_type_1, l_modification_ts_1, l_insertion_ts_1);
    l_ctx.assetmetadata_database.insert_or_update_metadata(l_ctx.connection, l_path_2, l_type_1, l_modification_ts_2, l_insertion_ts_2);
    l_ctx.assetmetadata_database.insert_or_update_metadata(l_ctx.connection, l_path_3, l_type_2, l_modification_ts_3, l_insertion_ts_3);

    {
        AssetMetadataDatabase::Paths l_paths = l_ctx.assetmetadata_database.get_all_path_from_type(l_ctx.connection, l_type_1);

        assert_true(l_paths.data.Size == 2);
        assert_true(l_paths.data.get(0).slice.Slice_compare(l_path_1));
        assert_true(l_paths.data.get(1).slice.Slice_compare(l_path_2));

        l_paths.free();
    }

    l_ctx.free();
    l_asset_database_path.free();
};

inline void compile_modificationts_cache(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    String l_tmp_asset_path;
    File l_tmp_file;
    {
        String l_src_asset_path = String::allocate_elements(l_asset_root_path.slice);
        l_src_asset_path.append(Slice_int8_build_rawstr("material_asset_test.json"));

        l_tmp_asset_path = String::allocate_elements(l_asset_root_path.slice);
        l_tmp_asset_path.append(Slice_int8_build_rawstr("tmp.json"));
        l_tmp_file = File::create_or_open(l_tmp_asset_path.to_slice());
        File l_src_file = File::open(l_src_asset_path.to_slice());

        Span<int8> l_buf = l_src_file.read_file_allocate();
        l_tmp_file.write_file(l_buf.slice);

        l_buf.free();
        l_src_asset_path.free();
        l_src_file.free();
    }

    assert_true(AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                       Slice_int8_build_rawstr("tmp.json")) == 1);
    AssetMetadataDatabase::MetadataTS l_ts_before = l_ctx.assetmetadata_database.get_timestamps(l_ctx.connection, Slice_int8_build_rawstr("tmp.json"));
    assert_true(AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                       Slice_int8_build_rawstr("tmp.json")) == 1);
    AssetMetadataDatabase::MetadataTS l_ts_after = l_ctx.assetmetadata_database.get_timestamps(l_ctx.connection, Slice_int8_build_rawstr("tmp.json"));
    assert_true(l_ts_before.file_modification_ts == l_ts_after.file_modification_ts);
    assert_true(l_ts_before.insert_ts == l_ts_after.insert_ts);

    // We simulate a write operation
    {
        String l_src_asset_path = String::allocate_elements(l_asset_root_path.slice);
        l_src_asset_path.append(Slice_int8_build_rawstr("material_asset_test.json"));
        File l_src_file = File::open(l_src_asset_path.to_slice());
        Span<int8> l_buf = l_src_file.read_file_allocate();
        l_tmp_file.write_file(l_buf.slice);
        l_buf.free();
        l_src_asset_path.free();
        l_src_file.free();
    }

    assert_true(AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                       Slice_int8_build_rawstr("tmp.json")) == 1);
    AssetMetadataDatabase::MetadataTS l_ts_after_2 = l_ctx.assetmetadata_database.get_timestamps(l_ctx.connection, Slice_int8_build_rawstr("tmp.json"));
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
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));

    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    int8 l_compilation_result = AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                                                       Slice_int8_build_rawstr("aeiajeoijaef"));

    assert_true(l_compilation_result == 0);

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void shader_module_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));

    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           Slice_int8_build_rawstr("shad.frag"));

    {
        Span<int8> l_raw_file = AssetCompiler_open_and_read_asset_file(l_asset_root_path.slice, Slice_int8_build_rawstr("shad.frag"));
        l_raw_file.get(l_raw_file.Capacity - 1) = (int8)NULL;
        ShaderCompiled l_shader = p_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, l_raw_file.slice);

        l_raw_file.free();

        Span<int8> l_shader_module_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shad.frag")));
        assert_true(l_shader_module_compiled.slice.Slice_compare(l_shader.get_compiled_binary()));

        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shad.frag")));
        assert_true(l_metadata.type.slice.Slice_compare(AssetType_Const::SHADER_MODULE_NAME));
        assert_true(l_metadata.path.slice.Slice_compare(Slice_int8_build_rawstr("shad.frag")));
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
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));

    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           Slice_int8_build_rawstr("shad_wrong.frag"));

    {
        assert_true(l_ctx.asset_database.does_asset_exists(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shad_wrong.frag"))) == 0);
        assert_true(l_ctx.assetmetadata_database.does_assetmetadata_exists(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shad_wrong.frag"))) == 0);
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();

    printf("shader_module_error_compilation - END\n");
};

inline void shader_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           Slice_int8_build_rawstr("shader_asset_test.json"));

    {
        Span<int8> l_shader_ressource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shader_asset_test.json")));
        ShaderRessource::Asset::Value l_shader_value = ShaderRessource::Asset::Value::build_from_asset(ShaderRessource::Asset::build_from_binary(l_shader_ressource_compiled));
        SliceN<ShaderLayoutParameterType, 1> l_shader_layout_arr{ShaderLayoutParameterType::TEXTURE_FRAGMENT};

        assert_true(l_shader_value.execution_order == 1001);
        assert_true(l_shader_value.shader_configuration.zwrite == 0);
        assert_true(l_shader_value.shader_configuration.ztest == ShaderConfiguration::CompareOp::Always);
        assert_true(l_shader_value.specific_parameters.Slice_compare(slice_from_slicen(&l_shader_layout_arr)));

        l_shader_ressource_compiled.free();

        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shader_asset_test.json")));
        assert_true(l_metadata.type.slice.Slice_compare(AssetType_Const::SHADER_NAME));
        assert_true(l_metadata.path.slice.Slice_compare(Slice_int8_build_rawstr("shader_asset_test.json")));
        l_metadata.free();
    }
    {
        Span<int8> l_shader_dependencies_compiled = l_ctx.asset_database.get_asset_dependencies_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("shader_asset_test.json")));
        ShaderRessource::AssetDependencies::Value l_shader_dependencies =
            ShaderRessource::AssetDependencies::Value::build_from_asset(ShaderRessource::AssetDependencies{l_shader_dependencies_compiled});

        assert_true(l_shader_dependencies.vertex_module == HashSlice(Slice_int8_build_rawstr("shad.vert")));
        assert_true(l_shader_dependencies.fragment_module == HashSlice(Slice_int8_build_rawstr("shad.frag")));

        l_shader_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void material_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           Slice_int8_build_rawstr("material_asset_test.json"));

    {
        Span<int8> l_material_ressource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("material_asset_test.json")));
        MaterialRessource::Asset::Value l_material_value = MaterialRessource::Asset::Value::build_from_asset(MaterialRessource::Asset::build_from_binary(l_material_ressource_compiled));

        assert_true(l_material_value.parameters.parameters.get_size() == 5);
        {
            hash_t l_texture_hash = HashSlice(Slice_int8_build_rawstr("link_to_png.png"));
            assert_true(l_texture_hash == *l_material_value.parameters.get_parameter_texture_gpu_value(0));
        }
        {
            Slice<int8> tmp_uniform_host_value_int8 = l_material_value.parameters.get_parameter_uniform_host_value(1);
            Slice<float32> tmp_uniform_host_value = Slice_cast<float32>(&tmp_uniform_host_value_int8);
            assert_true(*Slice_get(&tmp_uniform_host_value, 0) == 1294.0f);
        }
        {
            Slice<int8> tmp_uniform_host_value_int8 = l_material_value.parameters.get_parameter_uniform_host_value(2);
            Slice<v2f> tmp_uniform_host_value = Slice_cast<v2f>(&tmp_uniform_host_value_int8);
            assert_true(*Slice_get(&tmp_uniform_host_value, 0) == v2f{1294.0f, 1295.0f});
        }
        {
            Slice<int8> tmp_uniform_host_value_int8 = l_material_value.parameters.get_parameter_uniform_host_value(3);
            Slice<v3f> tmp_uniform_host_value = Slice_cast<v3f>(&tmp_uniform_host_value_int8);
            assert_true(*Slice_get(&tmp_uniform_host_value, 0) == v3f{1294.0f, 1295.0f, 1296.0f});
        }
        {
            Slice<int8> tmp_uniform_host_value = l_material_value.parameters.get_parameter_uniform_host_value(4);
            Slice<v4f> tmp_uniform_host = Slice_cast<v4f>(&tmp_uniform_host_value);
            assert_true(*Slice_get(&tmp_uniform_host, 0) == v4f{1294.0f, 1295.0f, 1296.0f, 1297.0f});
        }
        l_material_ressource_compiled.free();
    }
    {
        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("material_asset_test.json")));
        assert_true(l_metadata.type.slice.Slice_compare(AssetType_Const::MATERIAL_NAME));
        assert_true(l_metadata.path.slice.Slice_compare(Slice_int8_build_rawstr("material_asset_test.json")));
        l_metadata.free();
    }
    {
        Span<int8> l_material_dependencies_compiled = l_ctx.asset_database.get_asset_dependencies_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("material_asset_test.json")));
        MaterialRessource::AssetDependencies::Value l_material_dependencies =
            MaterialRessource::AssetDependencies::Value::build_from_asset(MaterialRessource::AssetDependencies{l_material_dependencies_compiled});

        assert_true(l_material_dependencies.shader == HashSlice(Slice_int8_build_rawstr("shader_asset_test.json")));
        assert_true(l_material_dependencies.textures.Size == 1);
        assert_true(*Slice_get(&l_material_dependencies.textures, 0) == HashSlice(Slice_int8_build_rawstr("link_to_png.png")));

        l_material_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void mesh_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           Slice_int8_build_rawstr("cube.obj"));
    {
        Span<int8> l_mesh_ressource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("cube.obj")));
        MeshRessource::Asset::Value l_mesh_value = MeshRessource::Asset::Value::build_from_asset(MeshRessource::Asset::build_from_binary(l_mesh_ressource_compiled));

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

        assert_true(l_mesh_value.initial_vertices.Slice_compare(slice_from_slicen(&l_vertices)));
        assert_true(l_mesh_value.initial_indices.Slice_compare(slice_from_slicen(&l_indices)));

        l_mesh_ressource_compiled.free();
    }
    {
        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("cube.obj")));
        assert_true(l_metadata.type.slice.Slice_compare(AssetType_Const::MESH_NAME));
        assert_true(l_metadata.path.slice.Slice_compare(Slice_int8_build_rawstr("cube.obj")));
        l_metadata.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
};

inline void texture_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_and_metadata_test_initialize(Slice_int8_build_rawstr("asset.db"));
    AssetCompilerTestCtx l_ctx = AssetCompilerTestCtx::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_ctx.connection, l_ctx.asset_database, l_ctx.assetmetadata_database, l_asset_root_path.slice,
                                                           Slice_int8_build_rawstr("texture.png"));
    {
        Span<int8> l_texture_ressource_compiled = l_ctx.asset_database.get_asset_blob(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("texture.png")));

        TextureRessource::Asset::Value l_texture_value = TextureRessource::Asset::Value::build_from_asset(TextureRessource::Asset{l_texture_ressource_compiled});
        assert_true(l_texture_value.size == v3ui{4, 4, 1});
        assert_true(l_texture_value.channel_nb == 4);
        Slice<color> l_pixels = Slice_cast<color>(&l_texture_value.pixels);
        assert_true(l_pixels.Size == 16);
        SliceN<color, 16> l_awaited_pixels_arr{color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                               color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                               color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX},
                                               color{UINT8_MAX, 0, 0, UINT8_MAX}, color{0, UINT8_MAX, 0, UINT8_MAX}, color{0, 0, UINT8_MAX, UINT8_MAX}, color{0, 0, 0, UINT8_MAX}};

        assert_true(l_pixels.Slice_compare(slice_from_slicen(&l_awaited_pixels_arr)));
        l_texture_ressource_compiled.free();
    }
    {
        AssetMetadataDatabase::AssetMetadata l_metadata = l_ctx.assetmetadata_database.get_from_id(l_ctx.connection, HashSlice(Slice_int8_build_rawstr("texture.png")));
        assert_true(l_metadata.type.slice.Slice_compare(AssetType_Const::TEXTURE_NAME));
        assert_true(l_metadata.path.slice.Slice_compare(Slice_int8_build_rawstr("texture.png")));
        l_metadata.free();
    }

    l_asset_root_path.free();
    l_ctx.free();
    l_asset_database_path.free();
}

inline void compilation_pass(ShaderCompiler& p_shader_compiler)
{
    String l_db_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_db_path.append(Slice_int8_build_rawstr("asset.db"));
    String l_db_2_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_db_2_path.append(Slice_int8_build_rawstr("asset_2.db"));
    {
        File l_db = File::create_or_open(l_db_path.to_slice());
        l_db.erase();
    }

    AssetCompilerPassHeap l_heap = AssetCompilerPassHeap::allocate();
    AssetCompilerConfigurationJSON l_asset_compiler_configuration{};

    AssetCompilationPassStatic l_static_pass = AssetCompilationPassStatic::allocate_default();
    l_static_pass.root_path.append(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_static_pass.assets_to_compile.push_back_element(String::allocate_elements(Slice_int8_build_rawstr("cube.obj")));
    l_asset_compiler_configuration.static_passes.push_back_element(l_static_pass);

    AssetCompilationPass l_pass_1 = AssetCompilationPass::allocate_default();
    l_pass_1.root_path.append(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_pass_1.database_path.append(l_db_path.to_slice());
    l_pass_1.assets_to_compile.push_back_element(String::allocate_elements(Slice_int8_build_rawstr("material_asset_test.json")));

    AssetCompilationPass l_pass_2 = AssetCompilationPass::allocate_default();
    l_pass_2.root_path.append(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_pass_2.database_path.append(l_db_2_path.to_slice());
    l_pass_2.assets_to_compile.push_back_element(String::allocate_elements(Slice_int8_build_rawstr("material_asset_test.json")));
    l_pass_2.assets_to_compile.push_back_element(String::allocate_elements(Slice_int8_build_rawstr("texture.png")));

    l_asset_compiler_configuration.local_passes.push_back_element(l_pass_1);
    l_asset_compiler_configuration.local_passes.push_back_element(l_pass_2);

    Vector<Token(AssetCompilationPass)> l_passes = Vector<Token(AssetCompilationPass)>::allocate(0);
    AssetCompilerPassComposition::allocate_passes_from_inline(l_heap, l_asset_compiler_configuration, Slice_int8_build_rawstr(ASSET_FOLDER_PATH), &l_passes);

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
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashSlice(Slice_int8_build_rawstr("cube.obj"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashSlice(Slice_int8_build_rawstr("material_asset_test.json"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashSlice(Slice_int8_build_rawstr("texture.png"))) == 0);
        l_asset_db.free(l_db_connection);
        l_db_connection.free();
    }
    {
        DatabaseConnection l_db_connection = DatabaseConnection::allocate(l_db_2_path.to_slice());
        AssetDatabase l_asset_db = AssetDatabase::allocate(l_db_connection);
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashSlice(Slice_int8_build_rawstr("cube.obj"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashSlice(Slice_int8_build_rawstr("material_asset_test.json"))));
        assert_true(l_asset_db.does_asset_exists(l_db_connection, HashSlice(Slice_int8_build_rawstr("texture.png"))));
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

inline void compilation_pass_configuration_serialization(){
    // TODO -> write test
    // const int8* l_configuration_json = MULTILINE();
    // AssetCompilerConfigurationJSON l_configuration = AssetCompilerConfigurationJSON::allocate_from_json();
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