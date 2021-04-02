
#include "AssetRessource/asset_ressource.hpp"
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_database_test_utils.hpp"

inline void render_asset_binary_serialization_deserialization_test()
{
    Slice<int8> l_slice_int8 = Slice_int8_build_rawstr("this is a test slice");
    {
        ShaderModuleRessource::Asset l_shader_modules = ShaderModuleRessource::Asset::allocate_from_values(ShaderModuleRessource::Asset::Value{l_slice_int8});
        ShaderModuleRessource::Asset::Value l_shader_module_value = ShaderModuleRessource::Asset::Value::build_from_asset(l_shader_modules);
        assert_true(Slice_compare(&l_shader_module_value.compiled_shader, &l_slice_int8));
        l_shader_modules.free();
    }
    {
        Declare_sized_slice(ShaderLayoutParameterType, 2, l_value_layout_arr, l_value_layout_slice, ShaderLayoutParameterType::TEXTURE_FRAGMENT, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX);
        ShaderRessource::Asset::Value l_value = ShaderRessource::Asset::Value{l_value_layout_slice, 2, ShaderConfiguration{0, ShaderConfiguration::CompareOp::Always}};
        ShaderRessource::Asset l_shader = ShaderRessource::Asset::allocate_from_values(l_value);
        ShaderRessource::Asset::Value l_deserialized_asset = ShaderRessource::Asset::Value::build_from_asset(l_shader);
        assert_true(l_deserialized_asset.execution_order == l_value.execution_order);
        Slice<ShaderConfiguration> l_asset_shader_configuration_slice = Slice_build_memory_elementnb<ShaderConfiguration>(&l_deserialized_asset.shader_configuration, 1);
        Slice<ShaderConfiguration> l_value_shader_configuration_slice = Slice_build_memory_elementnb<ShaderConfiguration>(&l_value.shader_configuration, 1);
        assert_true(Slice_compare(&l_asset_shader_configuration_slice, &l_value_shader_configuration_slice));
        assert_true(Slice_compare(&l_deserialized_asset.specific_parameters, &l_value.specific_parameters));
        l_shader.free();
    }
    Declare_sized_slice(Vertex, 2, l_initial_vertices_arr, l_initial_vertices, Vertex{v3f{1.0f, 2.0f, 3.0f}, v2f{1.0f, 1.0f}});
    Declare_sized_slice(uint32, 2, l_initial_indices_arr, l_initial_indices, 1, 2);
    {
        MeshRessource::Asset::Value l_value = MeshRessource::Asset::Value{l_initial_vertices, l_initial_indices};
        MeshRessource::Asset l_mesh = MeshRessource::Asset::allocate_from_values(l_value);
        MeshRessource::Asset::Value l_deserialized_value = MeshRessource::Asset::Value::build_from_asset(l_mesh);
        assert_true(Slice_compare(&l_deserialized_value.initial_vertices, &l_initial_vertices));
        assert_true(Slice_compare(&l_deserialized_value.initial_indices, &l_initial_indices));
        l_mesh.free();
    }
    {
        TextureRessource::Asset::Value l_value = TextureRessource::Asset::Value{v3ui{8, 8, 1}, 4, l_slice_int8};
        TextureRessource::Asset l_texture = TextureRessource::Asset::allocate_from_values(l_value);
        TextureRessource::Asset::Value l_deserialized_value = TextureRessource::Asset::Value::build_from_asset(l_texture);
        assert_true(l_deserialized_value.size == l_value.size);
        assert_true(l_deserialized_value.channel_nb == 4);
        assert_true(Slice_compare(&l_deserialized_value.pixels, &l_value.pixels));
        l_texture.free();
    }

    Declare_sized_slice(ShaderParameter::Type, 2, l_material_parameters_memory_arr, l_material_parameters_memory_slice, ShaderParameter::Type::TEXTURE_GPU, ShaderParameter::Type::UNIFORM_HOST);
    Slice<int8> l_material_parameters_memory = Slice_build_asint8(&l_material_parameters_memory_slice);
    Declare_sized_slice(SliceIndex, 2, l_chunks_arr, l_chunkds, SliceIndex_build(0, sizeof(ShaderParameter::Type)), SliceIndex_build(sizeof(ShaderParameter::Type), sizeof(ShaderParameter::Type)));
    VaryingSlice l_material_parameters_varying = VaryingSlice::build(l_material_parameters_memory, l_chunkds);
    {
        MaterialRessource::Asset::Value l_value = MaterialRessource::Asset::Value{l_material_parameters_varying};
        MaterialRessource::Asset l_material = MaterialRessource::Asset::allocate_from_values(l_value);
        MaterialRessource::Asset::Value l_deserialized_value = MaterialRessource::Asset::Value::build_from_asset(l_material);
        assert_true(Slice_compare(&l_deserialized_value.parameters.parameters.memory, &l_material_parameters_varying.memory));
        assert_true(Slice_compare(&l_deserialized_value.parameters.parameters.chunks, &l_material_parameters_varying.chunks));
        l_material.free();
    }
};

struct AssetRessourceTestContext
{
    GPUContext gpu_ctx;
    D3Renderer renderer;
    RenderRessourceAllocator2 render_ressource_allocator;
    DatabaseConnection database_connection;
    AssetDatabase asset_database;

    inline static AssetRessourceTestContext allocate()
    {
        GPUContext l_gpu_ctx = GPUContext::allocate(Slice_build_default<GPUExtension>());
        D3Renderer l_renderer = D3Renderer::allocate(l_gpu_ctx, ColorStep::AllocateInfo{v3ui{8, 8, 1}, 0});
        String l_asset_database_path = asset_database_test_initialize(Slice_int8_build_rawstr("asset.db"));
        DatabaseConnection l_database_connection = DatabaseConnection::allocate(l_asset_database_path.to_slice());
        AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_connection);
        RenderRessourceAllocator2 l_render_ressource_allocator = RenderRessourceAllocator2::allocate();
        l_asset_database_path.free();
        return AssetRessourceTestContext{l_gpu_ctx, l_renderer, l_render_ressource_allocator, l_database_connection, l_asset_database};
    };

    inline void free()
    {
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        this->render_ressource_allocator.free(this->renderer, this->gpu_ctx);
        this->renderer.free(this->gpu_ctx);
        this->gpu_ctx.free();
    };

    inline void reset_database()
    {
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        String l_asset_database_path = asset_database_test_initialize(Slice_int8_build_rawstr("asset.db"));
        this->database_connection = DatabaseConnection::allocate(l_asset_database_path.to_slice());
        this->asset_database = AssetDatabase::allocate(this->database_connection);
        l_asset_database_path.free();
    };
};

struct CachedCompiledShaders
{
    Span<int8> vertex_dummy_shader;
    Span<int8> fragment_dummy_shader;

    inline static CachedCompiledShaders allocate()
    {
        ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

        const int8* p_vertex_litteral =
            MULTILINE(\
                #version 450 \n

                layout(location = 0) in vec3 pos; \n
                layout(location = 1) in vec2 uv; \n

                struct Camera \n
            { \n
                mat4 view; \n
                mat4 projection; \n
            }; \n

                layout(set = 0, binding = 0) uniform camera { Camera cam; }; \n
                layout(set = 1, binding = 0) uniform model { mat4 mod; }; \n

                void main()\n
            { \n
                gl_Position = cam.projection * (cam.view * (mod * vec4(pos.xyz, 1.0f)));\n
            }\n
            );

        const int8* p_fragment_litteral =
            MULTILINE(\
                #version 450\n

                layout(location = 0) out vec4 outColor;\n

                void main()\n
            { \n
                outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n
            }\n
            );

        ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
        ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));
        Slice<int8> l_vertex_shader_compiled_binary = l_vertex_shader_compiled.get_compiled_binary();
        Slice<int8> l_fragment_shader_compiled_binary = l_fragment_shader_compiled.get_compiled_binary();

        CachedCompiledShaders l_return;

        l_return.vertex_dummy_shader = Span_allocate_slice(&l_vertex_shader_compiled_binary);
        l_return.fragment_dummy_shader = Span_allocate_slice(&l_fragment_shader_compiled_binary);

        l_vertex_shader_compiled.free();
        l_fragment_shader_compiled.free();

        l_shader_compiler.free();

        return l_return;
    };

    inline void free()
    {
        Span_free(&this->vertex_dummy_shader);
        Span_free(&this->fragment_dummy_shader);
    }
};

struct AssetRessource_TestAssertion
{
    struct AssertRessource
    {
        hash_t id;
        uimax counter;
        int8 allocated;
    };

    inline static void assert_material_allocation(AssetRessourceTestContext p_ctx, const Token(MaterialRessource) p_material_ressource, const AssertRessource& p_material_assert,
                                                  const Slice<AssertRessource>& p_material_textures, const AssertRessource& p_shader_assert, const AssertRessource& p_vertex_shader_assert,
                                                  const AssertRessource& p_fragment_shader_assert)
    {
        MaterialRessource& l_material = p_ctx.render_ressource_allocator.material_unit.materials.pool.get(p_material_ressource);
        assert_ressource(p_ctx.render_ressource_allocator.material_unit.materials, l_material, p_material_assert);

        if (p_material_assert.counter > 0)
        {
            Slice<MaterialRessource::DynamicDependency> l_material_parameter_dependencies =
                p_ctx.render_ressource_allocator.material_unit.material_dynamic_dependencies.get_vector(l_material.dependencies.dynamic_dependencies);
            assert_true(l_material_parameter_dependencies.Size == p_material_textures.Size);
            for (loop(i, 0, l_material_parameter_dependencies.Size))
            {
                TextureRessource& l_texture_ressource = p_ctx.render_ressource_allocator.texture_unit.textures.pool.get(Slice_get(&l_material_parameter_dependencies, i)->dependency);
                assert_ressource(p_ctx.render_ressource_allocator.texture_unit.textures, l_texture_ressource, *Slice_get(&p_material_textures, i));
            }
        }
        else
        {
            assert_true(p_ctx.render_ressource_allocator.material_unit.material_dynamic_dependencies.is_token_free(l_material.dependencies.dynamic_dependencies));
        }

        ShaderRessource& l_shader = p_ctx.render_ressource_allocator.shader_unit.shaders.pool.get(l_material.dependencies.shader);
        assert_ressource(p_ctx.render_ressource_allocator.shader_unit.shaders, l_shader, p_shader_assert);

        ShaderModuleRessource& l_vertex_shader_ressource = p_ctx.render_ressource_allocator.shader_module_unit.shader_modules.pool.get(l_shader.dependencies.vertex_shader);
        assert_ressource(p_ctx.render_ressource_allocator.shader_module_unit.shader_modules, l_vertex_shader_ressource, p_vertex_shader_assert);

        ShaderModuleRessource& l_fragment_shader_ressource = p_ctx.render_ressource_allocator.shader_module_unit.shader_modules.pool.get(l_shader.dependencies.fragment_shader);
        assert_ressource(p_ctx.render_ressource_allocator.shader_module_unit.shader_modules, l_fragment_shader_ressource, p_fragment_shader_assert);
    };

    inline static void assert_mesh_allocation(AssetRessourceTestContext p_ctx, const Token(MeshRessource) p_mesh_ressource, const AssertRessource& p_mesh_assert)
    {
        MeshRessource& l_mesh_ressource = p_ctx.render_ressource_allocator.mesh_unit.meshes.pool.get(p_mesh_ressource);
        assert_ressource(p_ctx.render_ressource_allocator.mesh_unit.meshes, l_mesh_ressource, p_mesh_assert);
    };

    inline static void assert_texture_allocation(AssetRessourceTestContext p_ctx, const Token(TextureRessource) p_texture_ressource, const AssertRessource& p_texture_assert)
    {
        TextureRessource& l_texture_ressource = p_ctx.render_ressource_allocator.texture_unit.textures.pool.get(p_texture_ressource);
        assert_ressource(p_ctx.render_ressource_allocator.texture_unit.textures, l_texture_ressource, p_texture_assert);
    };

  private:
    template <class t_RessourceType> inline static void assert_ressource(PoolHashedCounted<hash_t, t_RessourceType>& p_map, t_RessourceType& p_ressource, const AssertRessource& p_assert_ressource)
    {
        assert_true(p_ressource.header.id == p_assert_ressource.id);
        assert_true(p_ressource.header.allocated == p_assert_ressource.allocated);
        if (p_assert_ressource.counter == 0)
        {
            assert_true(!p_map.CountMap.has_key_nothashed(p_ressource.header.id));
        }
        else
        {
            assert_true(p_map.CountMap.get_value_nothashed(p_ressource.header.id)->counter == p_assert_ressource.counter);
        }
    };
};

inline void render_middleware_inline_allocation(CachedCompiledShaders& p_cached_compiled_shader)
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();
    {
        Declare_sized_slice(ShaderLayoutParameterType, 2, l_shader_parameter_layout_arr, l_shader_parameter_layout, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX,
                            ShaderLayoutParameterType::TEXTURE_FRAGMENT);
        v3f l_positions[8] = {v3f{-1.0f, -1.0f, 1.0f}, v3f{-1.0f, 1.0f, 1.0f}, v3f{-1.0f, -1.0f, -1.0f}, v3f{-1.0f, 1.0f, -1.0f},
                              v3f{1.0f, -1.0f, 1.0f},  v3f{1.0f, 1.0f, 1.0f},  v3f{1.0f, -1.0f, -1.0f},  v3f{1.0f, 1.0f, -1.0f}};

        v2f l_uvs[14] = {v2f{0.625f, 0.0f},  v2f{0.375f, 0.25f}, v2f{0.375f, 0.0f},  v2f{0.625f, 0.25f}, v2f{0.375f, 0.5f},  v2f{0.625f, 0.5f},  v2f{0.375f, 0.75f},
                         v2f{0.625f, 0.75f}, v2f{0.375f, 1.00f}, v2f{0.125f, 0.75f}, v2f{0.125f, 0.50f}, v2f{0.875f, 0.50f}, v2f{0.625f, 1.00f}, v2f{0.875f, 0.75f}};

        Vertex l_vertices[14] = {Vertex{l_positions[1], l_uvs[0]},  Vertex{l_positions[2], l_uvs[1]}, Vertex{l_positions[0], l_uvs[2]},  Vertex{l_positions[3], l_uvs[3]},
                                 Vertex{l_positions[6], l_uvs[4]},  Vertex{l_positions[7], l_uvs[5]}, Vertex{l_positions[4], l_uvs[6]},  Vertex{l_positions[5], l_uvs[7]},
                                 Vertex{l_positions[0], l_uvs[8]},  Vertex{l_positions[0], l_uvs[9]}, Vertex{l_positions[2], l_uvs[10]}, Vertex{l_positions[3], l_uvs[11]},
                                 Vertex{l_positions[1], l_uvs[12]}, Vertex{l_positions[1], l_uvs[13]}};
        uint32 l_indices[14 * 3] = {0, 1, 2, 3, 4, 1, 5, 6, 4, 7, 8, 6, 4, 9, 10, 11, 7, 5, 0, 3, 1, 3, 5, 4, 5, 7, 6, 7, 12, 8, 4, 6, 9, 11, 13, 7};

        Slice<Vertex> l_vertices_span = Slice_build_memory_elementnb<Vertex>(l_vertices, 14);
        Slice<uint32> l_indices_span = Slice_build_memory_elementnb<uint32>(l_indices, 14 * 3);

        hash_t l_vertex_shader_id = 12;
        ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(Span_allocate_slice(&p_cached_compiled_shader.vertex_dummy_shader.slice));

        hash_t l_fragment_shader_id = 14;
        ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(Span_allocate_slice(&p_cached_compiled_shader.fragment_dummy_shader.slice));

        hash_t l_shader_asset_id = 1482658;
        ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        hash_t l_mesh_id = 1486;
        MeshRessource::Asset l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});

        hash_t l_material_texture_id = 14874879;
        Span<int8> l_material_texture_span = Span_allocate<int8>(8 * 8 * 4);
        TextureRessource::Asset l_material_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        Span_free(&l_material_texture_span);

        MaterialRessource::Asset l_material_asset_1;
        {
            Span<int8> l_material_parameter_temp = Span_allocate<int8>(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;

            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8_memory_singleelement<hash_t>(&l_material_texture_id));
            l_material_asset_1 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            Span_free(&l_material_parameter_temp);
        }

        Token(MeshRessource) l_mesh_ressource =
            MeshRessourceComposition::allocate_or_increment_inline(l_ctx.render_ressource_allocator.mesh_unit, MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});

        Declare_sized_slice(TextureRessource::InlineAllocationInput, 1, l_material_ressource_texture_input_arr, l_material_ressource_texture_input_slice, l_material_texture_id,
                            l_material_texture_asset);
        Token(MaterialRessource) l_material_ressource = MaterialRessourceComposition::allocate_or_increment_inline(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, MaterialRessource::InlineAllocationInput{0, l_material_asset_1, l_material_ressource_texture_input_slice},
            ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset}, ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        Slice<MaterialRessource::DynamicDependency> l_material_dynamic_dependencies = l_ctx.render_ressource_allocator.material_unit.material_dynamic_dependencies.get_vector(
            l_ctx.render_ressource_allocator.material_unit.materials.pool.get(l_material_ressource).dependencies.dynamic_dependencies);
        Token(TextureRessource) l_material_texture_ressource = Slice_get(&l_material_dynamic_dependencies, 0)->dependency;

        MaterialRessource::Asset l_material_asset_2;
        {
            Span<int8> l_material_parameter_temp = Span_allocate<int8>(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8_memory_singleelement<hash_t>(&l_material_texture_id));
            l_material_asset_2 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            Span_free(&l_material_parameter_temp);
        }

        Token(MeshRessource) l_mesh_ressource_2 =
            MeshRessourceComposition::allocate_or_increment_inline(l_ctx.render_ressource_allocator.mesh_unit, MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Declare_sized_slice(TextureRessource::InlineAllocationInput, 1, l_material_ressource_2_arr, l_material_ressource_2_slice,
                            TextureRessource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset});
        Token(MaterialRessource) l_material_ressource_2 = MaterialRessourceComposition::allocate_or_increment_inline(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, MaterialRessource::InlineAllocationInput{1, l_material_asset_2, l_material_ressource_2_slice},
            ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset}, ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        /*
            We created mesh_1, mesh_2, material_1 and material_2
            They use the same Shader but materials are different
            They use the same Mesh.
            They are allocated and a step is executing -> render ressources will be allocated.
        */
        {
            {
                Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, material_assert_ressource_arr, material_assert_ressource_slice,
                                    AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 2, 1});
                AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource, AssetRessource_TestAssertion::AssertRessource{0, 1, 1}, material_assert_ressource_slice,
                                                                         AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 2, 1},
                                                                         AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                         AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
                AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 2, 1});
            }
            {
                Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, material_assert_ressource_arr, material_assert_ressource_slice,
                                    AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 2, 1});
                AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource_2, AssetRessource_TestAssertion::AssertRessource{1, 1, 1}, material_assert_ressource_slice,
                                                                         AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 2, 1},
                                                                         AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                         AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
                AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource_2, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 2, 1});
            }
        }

        Token(MeshRessource) l_mesh_ressource_3 =
            MeshRessourceComposition::allocate_or_increment_inline(l_ctx.render_ressource_allocator.mesh_unit, MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Token(MaterialRessource) l_material_ressource_3 = MaterialRessourceComposition::allocate_or_increment_inline(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, MaterialRessource::InlineAllocationInput{0}, ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset},
            ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader}, ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, material_assert_ressource_arr, material_assert_ressource_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 2, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource_3, AssetRessource_TestAssertion::AssertRessource{0, 2, 1}, material_assert_ressource_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 2, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource_3, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 3, 1});
        }

        l_ctx.render_ressource_allocator.mesh_unit.release_ressource(l_mesh_ressource_3);
        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material_ressource_3);

        /*
           We have created a mesh and material on the same frame.
           No render module allocation must occurs.
           The MeshRenderer has been removed from the RenderRessourceAllocator.
       */
        {
            assert_true(l_ctx.render_ressource_allocator.material_unit.materials_allocation_events.Size == 0);
            assert_true(l_ctx.render_ressource_allocator.mesh_unit.meshes_allocation_events.Size == 0);
            assert_true(l_ctx.render_ressource_allocator.material_unit.materials.CountMap.get_value_nothashed(0)->counter == 1);
            assert_true(l_ctx.render_ressource_allocator.mesh_unit.meshes.CountMap.get_value_nothashed(l_mesh_id)->counter == 2);
        }
        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, material_assert_ressource_arr, material_assert_ressource_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 2, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource_3, AssetRessource_TestAssertion::AssertRessource{0, 1, 1}, material_assert_ressource_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 2, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource_3, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 2, 1});
        }

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        l_ctx.render_ressource_allocator.mesh_unit.release_ressource(l_mesh_ressource_2);
        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material_ressource_2);

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, material_assert_ressource_arr, material_assert_ressource_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 1, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource_2, AssetRessource_TestAssertion::AssertRessource{1, 0, 1}, material_assert_ressource_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource_2, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 1, 1});
            AssetRessource_TestAssertion::assert_texture_allocation(l_ctx, l_material_texture_ressource, AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 1, 1});
        }

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        // We removed the l_node_2. The l_node_1 is still here and common ressources still allocated
        {
            assert_true(l_ctx.render_ressource_allocator.material_unit.materials.pool.is_element_free(l_material_ressource_2));
            assert_true(l_ctx.render_ressource_allocator.mesh_unit.meshes.CountMap.get_value_nothashed(l_mesh_id)->counter == 1);
        }

        l_ctx.render_ressource_allocator.mesh_unit.release_ressource(l_mesh_ressource);
        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material_ressource);

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, material_assert_ressource_arr, material_assert_ressource_slice, l_material_texture_id, 0, 1);
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource, AssetRessource_TestAssertion::AssertRessource{0, 0, 1}, material_assert_ressource_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 0, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 0, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 0, 1});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 0, 1});
            AssetRessource_TestAssertion::assert_texture_allocation(l_ctx, l_material_texture_ressource, AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 0, 1});
        }
    }

    // l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

    l_ctx.free();
};

inline void render_middleware_inline_alloc_dealloc_same_frame(CachedCompiledShaders p_cached_compiled_shaders)
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();
    {
        Declare_sized_slice(ShaderLayoutParameterType, 2, l_shader_parameter_layout_arr, l_shader_parameter_layout, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX,
                            ShaderLayoutParameterType::TEXTURE_FRAGMENT);

        v3f l_positions[1] = {v3f{-1.0f, -1.0f, 1.0f}};

        v2f l_uvs[1] = {v2f{0.625f, 0.0f}};

        Vertex l_vertices[1] = {Vertex{l_positions[0], l_uvs[0]}};
        uint32 l_indices[3] = {
            0,
            1,
            2,
        };

        Slice<Vertex> l_vertices_span = Slice_build_memory_elementnb<Vertex>(l_vertices, 1);
        Slice<uint32> l_indices_span = Slice_build_memory_elementnb<uint32>(l_indices, 3);

        hash_t l_vertex_shader_id = 12;
        ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(Span_allocate_slice(&p_cached_compiled_shaders.vertex_dummy_shader.slice));

        hash_t l_fragment_shader_id = 14;
        ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(Span_allocate_slice(&p_cached_compiled_shaders.fragment_dummy_shader.slice));

        hash_t l_shader_asset_id = 1482658;
        ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        hash_t l_mesh_id = 1486;
        MeshRessource::Asset l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});

        hash_t l_material_texture_id = 14874879;
        Span<int8> l_material_texture_span = Span_allocate<int8>(8 * 8 * 4);
        TextureRessource::Asset l_material_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        Span_free(&l_material_texture_span);

        MaterialRessource::Asset l_material_asset_1;
        {
            Span<int8> l_material_parameter_temp = Span_allocate<int8>(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8_memory_singleelement<hash_t>(&l_material_texture_id));
            l_material_asset_1 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            Span_free(&l_material_parameter_temp);
        }

        Token(MeshRessource) l_mesh_ressource =
            MeshRessourceComposition::allocate_or_increment_inline(l_ctx.render_ressource_allocator.mesh_unit, MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Declare_sized_slice(TextureRessource::InlineAllocationInput, 1, l_material_ressource_textureinput_arr, l_material_ressource_textureinput_slice,
                            TextureRessource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset});
        Token(MaterialRessource) l_material_ressource = MaterialRessourceComposition::allocate_or_increment_inline(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, MaterialRessource::InlineAllocationInput{0, l_material_asset_1, l_material_ressource_textureinput_slice},
            ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset}, ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        Slice<MaterialRessource::DynamicDependency> l_material_ressource_dynamic_dependencies = l_ctx.render_ressource_allocator.material_unit.material_dynamic_dependencies.get_vector(
            l_ctx.render_ressource_allocator.material_unit.materials.pool.get(l_material_ressource).dependencies.dynamic_dependencies);
        Token(TextureRessource) l_material_texture_ressource = Slice_get(&l_material_ressource_dynamic_dependencies, 0)->dependency;

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_assert_material_texure, l_assert_material_texure_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 1, 0});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource, AssetRessource_TestAssertion::AssertRessource{0, 1, 0}, l_assert_material_texure_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_asset_id, 1, 0},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 0},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 0});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 1, 0});
            AssetRessource_TestAssertion::assert_texture_allocation(l_ctx, l_material_texture_ressource, AssetRessource_TestAssertion::AssertRessource{l_material_texture_id, 1, 0});
        }

        l_ctx.render_ressource_allocator.mesh_unit.release_ressource(l_mesh_ressource);
        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material_ressource);

        {
            assert_true(l_ctx.render_ressource_allocator.mesh_unit.meshes.empty());
            assert_true(l_ctx.render_ressource_allocator.shader_unit.shaders.empty());
            assert_true(l_ctx.render_ressource_allocator.shader_module_unit.shader_modules.empty());
            assert_true(l_ctx.render_ressource_allocator.texture_unit.textures.empty());
            assert_true(l_ctx.render_ressource_allocator.material_unit.materials.empty());
            assert_true(!l_ctx.render_ressource_allocator.material_unit.material_dynamic_dependencies.has_allocated_elements());
        }

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);
    }
    l_ctx.free();
}

inline void render_middleware_database_allocation(CachedCompiledShaders p_cached_compiled_shaders)
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = Slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = Slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = Slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = Slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = Slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = Slice_int8_build_rawstr("mesh");

    const hash_t l_vertex_shader_id = HashSlice(l_vertex_shader_path);
    const hash_t l_fragment_shader_id = HashSlice(l_fragment_shader_path);
    const hash_t l_shader_id = HashSlice(l_shader_path);
    const hash_t l_texture_id = HashSlice(l_texture_path);
    const hash_t l_material_id = HashSlice(l_material_path);
    const hash_t l_mesh_id = HashSlice(l_mesh_path);

    {
        Declare_sized_slice(ShaderLayoutParameterType, 2, l_shader_asset_layout_arr, l_shader_asset_layout_slice, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX,
                            ShaderLayoutParameterType::TEXTURE_FRAGMENT);
        ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_asset_layout_slice, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span_allocate<int8>(8 * 8 * 4);
        TextureRessource::Asset l_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        Span_free(&l_material_texture_span);

        MaterialRessource::Asset l_material_asset;
        {
            Declare_sized_slice(hash_t, 1, tmp_texture_ids, tmp_texture_ids_slice, l_texture_id);
            Span<int8> l_material_parameter_temp = Span_allocate<int8>(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8(&tmp_texture_ids_slice));
            l_material_asset = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            Span_free(&l_material_parameter_temp);
        }

        MeshRessource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice_build_memory_elementnb<Vertex>(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice_build_memory_elementnb<uint32>(l_indices, 14 * 3);

            l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});
        }

        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_vertex_shader_path, p_cached_compiled_shaders.vertex_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_fragment_shader_path, p_cached_compiled_shaders.fragment_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_shader_path, l_shader_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_texture_path, l_texture_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_material_path, l_material_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_mesh_path, l_mesh_asset.allocated_binary.slice);

        l_shader_asset.free();
        l_texture_asset.free();
        l_material_asset.free();
        l_mesh_asset.free();
    }
    {

        Token(MeshRessource) l_mesh_ressource = MeshRessourceComposition::allocate_or_increment_database(l_ctx.render_ressource_allocator.mesh_unit, MeshRessource::DatabaseAllocationInput{l_mesh_id});
        Declare_sized_slice(TextureRessource::DatabaseAllocationInput, 1, l_material_ressource_textures_arr, l_material_ressource_textures_slice,
                            TextureRessource::DatabaseAllocationInput{l_texture_id});
        Token(MaterialRessource) l_material_ressource = MaterialRessourceComposition::allocate_or_increment_database(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, MaterialRessource::DatabaseAllocationInput{l_material_id, l_material_ressource_textures_slice},
            ShaderRessource::DatabaseAllocationInput{l_shader_id}, ShaderModuleRessource::DatabaseAllocationInput{l_vertex_shader_id},
            ShaderModuleRessource::DatabaseAllocationInput{l_fragment_shader_id});

        assert_true(l_ctx.render_ressource_allocator.mesh_unit.meshes_allocation_events.Size == 1);
        assert_true(l_ctx.render_ressource_allocator.material_unit.materials_allocation_events.Size == 1);

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_material_assert_textures, l_material_assert_textures_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_texture_id, 1, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource, AssetRessource_TestAssertion::AssertRessource{l_material_id, 1, 1}, l_material_assert_textures_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 1, 1});
        }

        l_ctx.render_ressource_allocator.mesh_unit.release_ressource(l_mesh_ressource);
        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material_ressource);

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_material_assert_textures, l_material_assert_textures_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_texture_id, 0, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material_ressource, AssetRessource_TestAssertion::AssertRessource{l_material_id, 0, 1}, l_material_assert_textures_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_id, 0, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 0, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 0, 1});
            AssetRessource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_ressource, AssetRessource_TestAssertion::AssertRessource{l_mesh_id, 0, 1});
        }
    }
    l_ctx.free();
};

inline void render_middleware_get_dependencies_from_database(CachedCompiledShaders& p_cached_compiled_shaders)
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = Slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = Slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = Slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = Slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = Slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = Slice_int8_build_rawstr("mesh");

    const hash_t l_vertex_shader_id = HashSlice(l_vertex_shader_path);
    const hash_t l_fragment_shader_id = HashSlice(l_fragment_shader_path);
    const hash_t l_shader_id = HashSlice(l_shader_path);
    const hash_t l_texture_id = HashSlice(l_texture_path);
    const hash_t l_material_id = HashSlice(l_material_path);
    const hash_t l_mesh_id = HashSlice(l_mesh_path);

    {
        Declare_sized_slice(ShaderLayoutParameterType, 2, l_shader_layout_arr, l_shader_layout_slice, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT);
        ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_layout_slice, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span_allocate<int8>(8 * 8 * 4);
        TextureRessource::Asset l_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        Span_free(&l_material_texture_span);

        MaterialRessource::Asset l_material_asset;
        {
            Declare_sized_slice(hash_t, 1, tmp_material_textures_arr, tmp_material_textures_arr_slice, HashSlice(l_texture_path));
            Span<int8> l_material_parameter_temp = Span_allocate<int8>(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8(&tmp_material_textures_arr_slice));
            l_material_asset = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            Span_free(&l_material_parameter_temp);
        }

        MeshRessource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice_build_memory_elementnb<Vertex>(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice_build_memory_elementnb<uint32>(l_indices, 14 * 3);

            l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});
        }

        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_vertex_shader_path, p_cached_compiled_shaders.vertex_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_fragment_shader_path, p_cached_compiled_shaders.fragment_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_shader_path, l_shader_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_texture_path, l_texture_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_material_path, l_material_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_mesh_path, l_mesh_asset.allocated_binary.slice);

        Declare_sized_slice(hash_t, 1, tmp_material_textures_arr, tmp_material_textures_slice, HashSlice(l_texture_path));
        MaterialRessource::AssetDependencies l_material_asset_dependencies = MaterialRessource::AssetDependencies::allocate_from_values(MaterialRessource::AssetDependencies::Value{
            HashSlice(l_shader_path), ShaderRessource::AssetDependencies::Value{HashSlice(l_vertex_shader_path), HashSlice(l_fragment_shader_path)}, tmp_material_textures_slice});

        l_ctx.asset_database.insert_asset_dependencies_blob(l_ctx.database_connection, l_material_path, l_material_asset_dependencies.allocated_binary.slice);

        l_material_asset_dependencies.free();

        l_shader_asset.free();
        l_texture_asset.free();
        l_material_asset.free();
        l_mesh_asset.free();
    }

    {
        Token(MaterialRessource) l_material = MaterialRessourceComposition::allocate_or_increment_database_and_load_dependecies(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, l_ctx.database_connection, l_ctx.asset_database, HashSlice(l_material_path));

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_material_assert_textures_arr, l_material_assert_textures_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_texture_id, 1, 0});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetRessource_TestAssertion::AssertRessource{l_material_id, 1, 0}, l_material_assert_textures_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_id, 1, 0},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 0},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 0});
        }

        assert_true(l_ctx.render_ressource_allocator.material_unit.materials_allocation_events.Size == 1);
        assert_true(l_ctx.render_ressource_allocator.texture_unit.textures_allocation_events.Size == 1);
        assert_true(l_ctx.render_ressource_allocator.shader_unit.shaders_allocation_events.Size == 1);
        assert_true(l_ctx.render_ressource_allocator.shader_module_unit.shader_modules_allocation_events.Size == 2);

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material);
    }
    l_ctx.free();
}

// When we try to allocate multiple time the same ressource, no database request is performed
inline void render_middleware_multiple_database_allocation(CachedCompiledShaders& p_cached_compile_shaders)
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = Slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = Slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = Slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = Slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = Slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = Slice_int8_build_rawstr("mesh");

    const hash_t l_vertex_shader_id = HashSlice(l_vertex_shader_path);
    const hash_t l_fragment_shader_id = HashSlice(l_fragment_shader_path);
    const hash_t l_shader_id = HashSlice(l_shader_path);
    const hash_t l_texture_id = HashSlice(l_texture_path);
    const hash_t l_material_id = HashSlice(l_material_path);
    const hash_t l_mesh_id = HashSlice(l_mesh_path);

    {
        Declare_sized_slice(ShaderLayoutParameterType, 2, l_shader_asset_layout_arr, l_shader_asset_layout_slice, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX,
                            ShaderLayoutParameterType::TEXTURE_FRAGMENT) ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_asset_layout_slice, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span_allocate<int8>(8 * 8 * 4);
        TextureRessource::Asset l_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        Span_free(&l_material_texture_span);

        MaterialRessource::Asset l_material_asset;
        {
            Declare_sized_slice(hash_t, 1, tmp_material_texture_arr, tmp_material_texture_slice, HashSlice(l_texture_path));
            Span<int8> l_material_parameter_temp = Span_allocate<int8>(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8(&tmp_material_texture_slice));
            l_material_asset = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            Span_free(&l_material_parameter_temp);
        }

        MeshRessource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice_build_memory_elementnb<Vertex>(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice_build_memory_elementnb<uint32>(l_indices, 14 * 3);

            l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});
        }

        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_vertex_shader_path, p_cached_compile_shaders.vertex_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_fragment_shader_path, p_cached_compile_shaders.fragment_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_shader_path, l_shader_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_texture_path, l_texture_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_material_path, l_material_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_mesh_path, l_mesh_asset.allocated_binary.slice);

        Declare_sized_slice(hash_t, 1, tmp_material_textures, tmp_material_textures_slice, HashSlice(l_texture_path));
        MaterialRessource::AssetDependencies l_material_asset_dependencies = MaterialRessource::AssetDependencies::allocate_from_values(MaterialRessource::AssetDependencies::Value{
            HashSlice(l_shader_path), ShaderRessource::AssetDependencies::Value{HashSlice(l_vertex_shader_path), HashSlice(l_fragment_shader_path)}, tmp_material_textures_slice});

        l_ctx.asset_database.insert_asset_dependencies_blob(l_ctx.database_connection, l_material_path, l_material_asset_dependencies.allocated_binary.slice);

        l_material_asset_dependencies.free();

        l_shader_asset.free();
        l_texture_asset.free();
        l_material_asset.free();
        l_mesh_asset.free();
    }

    Token(MaterialRessource) l_material;
    {
        l_material = MaterialRessourceComposition::allocate_or_increment_database_and_load_dependecies(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, l_ctx.database_connection, l_ctx.asset_database, HashSlice(l_material_path));
        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);
    }

    {
        Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_material_assert_textures, l_material_assert_textures_slice,
                            AssetRessource_TestAssertion::AssertRessource{l_texture_id, 1, 1});
        AssetRessource_TestAssertion::assert_material_allocation(
            l_ctx, l_material, AssetRessource_TestAssertion::AssertRessource{l_material_id, 1, 1}, l_material_assert_textures_slice, AssetRessource_TestAssertion::AssertRessource{l_shader_id, 1, 1},
            AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1}, AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
    }

    // We reset the database to be sure that data cannot be requested
    l_ctx.reset_database();

    {
        Token(MaterialRessource) l_material_2 = MaterialRessourceComposition::allocate_or_increment_database_and_load_dependecies(
            l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit, l_ctx.render_ressource_allocator.shader_module_unit,
            l_ctx.render_ressource_allocator.texture_unit, l_ctx.database_connection, l_ctx.asset_database, HashSlice(l_material_path));

        assert_true(token_equals(l_material_2, l_material));

        assert_true(l_ctx.render_ressource_allocator.material_unit.materials_allocation_events.Size == 0);
        assert_true(l_ctx.render_ressource_allocator.shader_unit.shaders_allocation_events.Size == 0);
        assert_true(l_ctx.render_ressource_allocator.shader_module_unit.shader_modules_allocation_events.Size == 0);

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_material_assert_textures, l_material_assert_textures_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_texture_id, 1, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetRessource_TestAssertion::AssertRessource{l_material_id, 2, 1}, l_material_assert_textures_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
        }
    }
    {
        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material);

        {
            Declare_sized_slice(AssetRessource_TestAssertion::AssertRessource, 1, l_material_assert_textures, l_material_assert_textures_slice,
                                AssetRessource_TestAssertion::AssertRessource{l_texture_id, 1, 1});
            AssetRessource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetRessource_TestAssertion::AssertRessource{l_material_id, 1, 1}, l_material_assert_textures_slice,
                                                                     AssetRessource_TestAssertion::AssertRessource{l_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_vertex_shader_id, 1, 1},
                                                                     AssetRessource_TestAssertion::AssertRessource{l_fragment_shader_id, 1, 1});
        }

        MaterialRessourceComposition::decrement_or_release(l_ctx.render_ressource_allocator.material_unit, l_ctx.render_ressource_allocator.shader_unit,
                                                           l_ctx.render_ressource_allocator.shader_module_unit, l_ctx.render_ressource_allocator.texture_unit, l_material);
        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);
    }

    l_ctx.free();
};

int main()
{
    render_asset_binary_serialization_deserialization_test();

    CachedCompiledShaders l_cached_compiled_shaders = CachedCompiledShaders::allocate();

    render_middleware_inline_allocation(l_cached_compiled_shaders);
    render_middleware_inline_alloc_dealloc_same_frame(l_cached_compiled_shaders);
    render_middleware_database_allocation(l_cached_compiled_shaders);
    render_middleware_get_dependencies_from_database(l_cached_compiled_shaders);
    render_middleware_multiple_database_allocation(l_cached_compiled_shaders);

    l_cached_compiled_shaders.free();

    memleak_ckeck();
}