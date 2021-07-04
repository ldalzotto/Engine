#pragma once

struct AssetCompilationPassStatic
{
    String root_path;
    Vector<String> assets_to_compile;

    inline static AssetCompilationPassStatic build(const String& p_root_path, const Vector<String>& p_asset_to_compile)
    {
        return AssetCompilationPassStatic{p_root_path, p_asset_to_compile};
    };

    inline static AssetCompilationPassStatic allocate_default()
    {
        return AssetCompilationPassStatic::build(String::allocate(0), Vector<String>::allocate(0));
    };

    inline void free()
    {
        this->root_path.free();
        for (loop(i, 0, this->assets_to_compile.Size))
        {
            this->assets_to_compile.get(i).free();
        }
        this->assets_to_compile.free();
    };
};

struct AssetCompilationPass
{
    String database_path;
    String root_path;
    Vector<String> assets_to_compile;

    using sToken = Pool<AssetCompilationPass>::sToken;
    using sTokenValue = Pool<AssetCompilationPass>::sTokenValue;

    inline static AssetCompilationPass build(const String& p_database_path, const String& p_root_path, const Vector<String>& p_asset_to_compile)
    {
        return AssetCompilationPass{p_database_path, p_root_path, p_asset_to_compile};
    };

    inline static AssetCompilationPass allocate_default()
    {
        return AssetCompilationPass::build(String::allocate(0), String::allocate(0), Vector<String>::allocate(0));
    };

    inline static AssetCompilationPass allocate_from_static(const String& p_database_path, const AssetCompilationPassStatic& p_static)
    {
        Vector<String> l_assets_to_compile = Vector<String>::allocate_elements(p_static.assets_to_compile.to_slice());
        for (loop(i, 0, l_assets_to_compile.Size))
        {
            l_assets_to_compile.get(i) = String::allocate_elements(p_static.assets_to_compile.get(i).to_slice());
        }
        return AssetCompilationPass::build(p_database_path, String::allocate_elements(p_static.root_path.to_slice()), l_assets_to_compile);
    };

    inline void free()
    {
        this->database_path.free();
        this->root_path.free();
        for (loop(i, 0, this->assets_to_compile.Size))
        {
            this->assets_to_compile.get(i).free();
        }
        this->assets_to_compile.free();
    };
};

// TODO -> to reduce system call done by allocating a large number of String (Vector<int8>) we can create a VectorOfVector and string allocations will be token of this VectorOfVector
struct AssetCompilerPassHeap
{
    Pool<AssetCompilationPass> asset_compilation_passes;

    inline static AssetCompilerPassHeap allocate()
    {
        return AssetCompilerPassHeap{Pool<AssetCompilationPass>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->asset_compilation_passes.has_allocated_elements());
#endif
        this->asset_compilation_passes.free();
    };

    inline void free_asset_compiler_pass(const AssetCompilationPass::sToken p_asset_compiler_pass_token)
    {
        this->asset_compilation_passes.get(p_asset_compiler_pass_token).free();
        this->asset_compilation_passes.release_element(p_asset_compiler_pass_token);
    };
};

struct AssetCompilerConfigurationJSON
{
    Vector<AssetCompilationPassStatic> static_passes;
    Vector<AssetCompilationPass> local_passes;

    inline static AssetCompilerConfigurationJSON allocate_default()
    {
        return AssetCompilerConfigurationJSON{Vector<AssetCompilationPassStatic>::allocate(0), Vector<AssetCompilationPass>::allocate(0)};
    };

    inline static AssetCompilerConfigurationJSON allocate_from_json(JSONDeserializer& p_deserializer, const Slice<int8>& p_pathes_absolute_prefix)
    {

        AssetCompilerConfigurationJSON l_return = AssetCompilerConfigurationJSON::allocate_default();
        {
            JSONDeserializer l_common_array = JSONDeserializer::allocate_default();
            JSONDeserializer l_common_item = JSONDeserializer::allocate_default();
            p_deserializer.next_array("common", &l_common_array);
            while (l_common_array.next_array_object(&l_common_item))
            {
                AssetCompilationPassStatic l_static_pass = AssetCompilationPassStatic::allocate_default();

                l_common_item.next_field("root_folder");
                l_static_pass.root_path.append(p_pathes_absolute_prefix);
                l_static_pass.root_path.append(l_common_item.get_currentfield().value);

                JSONDeserializer l_assets_array = JSONDeserializer::allocate_default();
                Slice<int8> l_assets_array_item;
                l_common_item.next_array("assets", &l_assets_array);
                while (l_assets_array.next_array_string_value(&l_assets_array_item))
                {
                    l_static_pass.assets_to_compile.push_back_element(String::allocate_elements(l_assets_array_item));
                }
                l_assets_array.free();

                l_return.static_passes.push_back_element(l_static_pass);
            }
            l_common_item.free();
            l_common_array.free();
        }

        {
            JSONDeserializer l_local_array = JSONDeserializer::allocate_default();
            JSONDeserializer l_local_array_item = JSONDeserializer::allocate_default();
            p_deserializer.next_array("config", &l_local_array);
            while (l_local_array.next_array_object(&l_local_array_item))
            {
                AssetCompilationPass l_local_config = AssetCompilationPass::allocate_default();
                l_local_array_item.next_field("root_folder");
                l_local_config.root_path.append(p_pathes_absolute_prefix);
                l_local_config.root_path.append(l_local_array_item.get_currentfield().value);
                l_local_array_item.next_field("database");
                l_local_config.database_path.append(l_local_config.root_path.to_slice());
                l_local_config.database_path.append(l_local_array_item.get_currentfield().value);

                JSONDeserializer l_assets_array = JSONDeserializer::allocate_default();
                Slice<int8> l_assets_array_item;
                l_local_array_item.next_array("assets", &l_assets_array);
                while (l_assets_array.next_array_string_value(&l_assets_array_item))
                {
                    l_local_config.assets_to_compile.push_back_element(String::allocate_elements(l_assets_array_item));
                }
                l_assets_array.free();

                l_return.local_passes.push_back_element(l_local_config);
            }
            l_local_array.free();
            l_local_array_item.free();
        }

        return l_return;
    };

    inline static AssetCompilerConfigurationJSON allocate_from_json_file(const Slice<int8>& p_json_file_path, const Slice<int8>& p_pathes_absolute_prefix)
    {
        File l_file = File::open(p_json_file_path);
        Span<int8> l_file_content = l_file.read_file_allocate();
        l_file.free();

        Vector<int8> l_file_content_vector = Vector<int8>{l_file_content.Capacity, l_file_content};
        JSONDeserializer l_deserializer = JSONDeserializer::sanitize_and_start(iVector_v2<Vector<int8>>{l_file_content_vector});

        AssetCompilerConfigurationJSON l_return = allocate_from_json(l_deserializer, p_pathes_absolute_prefix);

        l_deserializer.free();
        l_file_content_vector.free();

        return l_return;
    };

    inline Vector<AssetCompilationPass> consume_and_merge_to_passes()
    {
        Vector<AssetCompilationPass> l_return = Vector<AssetCompilationPass>::allocate(0);
        for (loop(i, 0, this->local_passes.Size))
        {
            AssetCompilationPass& l_local_pass = this->local_passes.get(i);
            for (loop(j, 0, this->static_passes.Size))
            {
                l_return.push_back_element(AssetCompilationPass::allocate_from_static(String::allocate_elements(l_local_pass.database_path.to_slice()), this->static_passes.get(j)));
            }
            l_return.push_back_element(l_local_pass);
        }

        this->free();

        return l_return;
    };

  private:
    inline void free()
    {
        for (loop(i, 0, this->static_passes.Size))
        {
            this->static_passes.get(i).free();
        }
        this->static_passes.free();
        this->local_passes.free();
    };
};

struct AssetCompilerPassComposition
{
    inline static void allocate_passes_from_json_configuration(AssetCompilerPassHeap& p_asset_compiler_pass_heap, const Slice<int8>& p_configuration_file_path, const Slice<int8>& p_asset_folder_root,
                                                               Vector<AssetCompilationPass::sToken>* in_out_asset_compilation_pass)
    {
        AssetCompilerConfigurationJSON l_asset_compiler_configuration = AssetCompilerConfigurationJSON::allocate_from_json_file(p_configuration_file_path, p_asset_folder_root);
        allocate_passes_from_inline(p_asset_compiler_pass_heap, l_asset_compiler_configuration, p_asset_folder_root, in_out_asset_compilation_pass);
    };

    inline static void allocate_passes_from_json_configuration_with_assetroot_as_configuration_file_path(AssetCompilerPassHeap& p_asset_compiler_pass_heap,
                                                                                                         const Slice<int8>& p_configuration_file_path,
                                                                                                         Vector<AssetCompilationPass::sToken>* in_out_asset_compilation_pass)
    {
        String l_configuration_file_path_str = String::allocate_elements(p_configuration_file_path);
        Path::move_up(&l_configuration_file_path_str);
        AssetCompilerPassComposition::allocate_passes_from_json_configuration(p_asset_compiler_pass_heap, p_configuration_file_path, l_configuration_file_path_str.to_slice(),
                                                                              in_out_asset_compilation_pass);
        l_configuration_file_path_str.free();
    };

    inline static void allocate_passes_from_inline(AssetCompilerPassHeap& p_asset_compiler_pass_heap, AssetCompilerConfigurationJSON& p_configuration_file, const Slice<int8>& p_asset_folder_root,
                                                   Vector<AssetCompilationPass::sToken>* in_out_asset_compilation_pass)
    {
        Vector<AssetCompilationPass> l_passes = p_configuration_file.consume_and_merge_to_passes();
        for (loop(i, 0, l_passes.Size))
        {
            in_out_asset_compilation_pass->push_back_element(p_asset_compiler_pass_heap.asset_compilation_passes.alloc_element(l_passes.get(i)));
        }
        l_passes.free();
    };

    struct ExecutionState
    {
        ShaderCompiler& shader_compiler;
        const AssetCompilationPass& asset_compilation_pass;
        DatabaseConnection database_connection;
        AssetDatabase asset_database;
        AssetMetadataDatabase asset_metadata_database;

        inline static ExecutionState allocate(ShaderCompiler& p_shader_compiler, const AssetCompilationPass& p_asset_compilation_pass)
        {
            DatabaseConnection l_database_connection = DatabaseConnection::allocate(p_asset_compilation_pass.database_path.to_slice());

            AssetDatabase::initialize_database(l_database_connection);
            AssetMetadataDatabase::initialize_database(l_database_connection);

            AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_connection);
            AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);

            return ExecutionState{p_shader_compiler, p_asset_compilation_pass, l_database_connection, l_asset_database, l_asset_metadata_database};
        };

        inline uimax get_size()
        {
            return this->asset_compilation_pass.assets_to_compile.Size;
        };

        inline int8 compile_single(const uimax p_index)
        {
            return AssetCompiler_compile_and_push_to_database_single_file(this->shader_compiler, this->database_connection, this->asset_database, this->asset_metadata_database,
                                                                          this->asset_compilation_pass.root_path.to_slice(), this->asset_compilation_pass.assets_to_compile.get(p_index).to_slice());
        };

        inline void compile_all()
        {
            for (loop(i, 0, this->asset_compilation_pass.assets_to_compile.Size))
            {
                int8 l_return = this->compile_single(i);
#if __DEBUG
                assert_true(l_return);
#endif
            }
        };

        inline void free()
        {
            this->asset_database.free(this->database_connection);
            this->asset_metadata_database.free(this->database_connection);
            this->database_connection.free();
        };
    };
};