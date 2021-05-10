
#include "AssetResource/asset_ressource.hpp"
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_database_test_utils.hpp"

inline void render_asset_binary_serialization_deserialization_test()
{
    Slice<int8> l_slice_int8 = slice_int8_build_rawstr("this is a test slice");
    {
        ShaderModuleResource::Asset l_shader_modules = ShaderModuleResource::Asset::allocate_from_values(ShaderModuleResource::Asset::Value{l_slice_int8});
        ShaderModuleResource::Asset::Value l_shader_module_value = ShaderModuleResource::Asset::Value::build_from_asset(l_shader_modules);
        assert_true(l_shader_module_value.compiled_shader.compare(l_slice_int8));
        l_shader_modules.free();
    }
    {
        SliceN<ShaderLayoutParameterType, 2> l_value_layout_arr{ShaderLayoutParameterType::TEXTURE_FRAGMENT, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX};
        ShaderResource::Asset::Value l_value = ShaderResource::Asset::Value{slice_from_slicen(&l_value_layout_arr), 2, ShaderConfiguration{0, ShaderConfiguration::CompareOp::Always}};
        ShaderResource::Asset l_shader = ShaderResource::Asset::allocate_from_values(l_value);
        ShaderResource::Asset::Value l_deserialized_asset = ShaderResource::Asset::Value::build_from_asset(l_shader);
        assert_true(l_deserialized_asset.execution_order == l_value.execution_order);
        assert_true(Slice<ShaderConfiguration>::build_memory_elementnb(&l_deserialized_asset.shader_configuration, 1)
                        .compare(Slice<ShaderConfiguration>::build_memory_elementnb(&l_value.shader_configuration, 1)));
        assert_true(l_deserialized_asset.specific_parameters.compare(l_value.specific_parameters));
        l_shader.free();
    }
    SliceN<Vertex, 2> l_initial_vertices_arr{Vertex{v3f{1.0f, 2.0f, 3.0f}, v2f{1.0f, 1.0f}}};
    Slice<Vertex> l_initial_vertices = slice_from_slicen(&l_initial_vertices_arr);
    SliceN<uint32, 2> l_initial_indices_arr{1, 2};
    Slice<uint32> l_initial_indices = slice_from_slicen(&l_initial_indices_arr);
    {
        MeshResource::Asset::Value l_value = MeshResource::Asset::Value{l_initial_vertices, l_initial_indices};
        MeshResource::Asset l_mesh = MeshResource::Asset::allocate_from_values(l_value);
        MeshResource::Asset::Value l_deserialized_value = MeshResource::Asset::Value::build_from_asset(l_mesh);
        assert_true(l_deserialized_value.initial_vertices.compare(l_initial_vertices));
        assert_true(l_deserialized_value.initial_indices.compare(l_initial_indices));
        l_mesh.free();
    }
    {
        TextureResource::Asset::Value l_value = TextureResource::Asset::Value{v3ui{8, 8, 1}, 4, l_slice_int8};
        TextureResource::Asset l_texture = TextureResource::Asset::allocate_from_values(l_value);
        TextureResource::Asset::Value l_deserialized_value = TextureResource::Asset::Value::build_from_asset(l_texture);
        assert_true(l_deserialized_value.size == l_value.size);
        assert_true(l_deserialized_value.channel_nb == 4);
        assert_true(l_deserialized_value.pixels.compare(l_value.pixels));
        l_texture.free();
    }

    SliceN<ShaderParameter::Type, 2> l_material_parameters_memory_arr{ShaderParameter::Type::TEXTURE_GPU, ShaderParameter::Type::UNIFORM_HOST};
    Slice<int8> l_material_parameters_memory = slice_from_slicen(&l_material_parameters_memory_arr).build_asint8();
    SliceN<SliceIndex, 2> l_chunks_arr{SliceIndex::build(0, sizeof(ShaderParameter::Type)), SliceIndex::build(sizeof(ShaderParameter::Type), sizeof(ShaderParameter::Type))};
    Slice<SliceIndex> l_chunkds = slice_from_slicen(&l_chunks_arr);
    VaryingSlice l_material_parameters_varying = VaryingSlice::build(l_material_parameters_memory, l_chunkds);
    {
        MaterialResource::Asset::Value l_value = MaterialResource::Asset::Value{l_material_parameters_varying};
        MaterialResource::Asset l_material = MaterialResource::Asset::allocate_from_values(l_value);
        MaterialResource::Asset::Value l_deserialized_value = MaterialResource::Asset::Value::build_from_asset(l_material);
        assert_true(l_deserialized_value.parameters.parameters.memory.compare(l_material_parameters_varying.memory));
        assert_true(l_deserialized_value.parameters.parameters.chunks.compare(l_material_parameters_varying.chunks));
        l_material.free();
    }
};

struct AssetResourceTestContext
{
    GPUContext gpu_ctx;
    RenderTargetInternal_Color_Depth render_target;
    D3Renderer renderer;
    RenderResourceAllocator2 render_resource_allocator;
    DatabaseConnection database_connection;
    AssetDatabase asset_database;

    inline static AssetResourceTestContext allocate()
    {
        GPUContext l_gpu_ctx = GPUContext::allocate(Slice<GPUExtension>::build_default());
        RenderTargetInternal_Color_Depth l_render_target = RenderTargetInternal_Color_Depth::allocate(l_gpu_ctx, RenderTargetInternal_Color_Depth::AllocateInfo{v3ui{8, 8, 1}, 0});
        D3Renderer l_renderer = D3Renderer::allocate(l_gpu_ctx, l_render_target);
        String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
        DatabaseConnection l_database_connection = DatabaseConnection::allocate(l_asset_database_path.to_slice());
        AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_connection);
        RenderResourceAllocator2 l_render_resource_allocator = RenderResourceAllocator2::allocate();
        l_asset_database_path.free();
        return AssetResourceTestContext{l_gpu_ctx, l_render_target, l_renderer, l_render_resource_allocator, l_database_connection, l_asset_database};
    };

    inline void free()
    {
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        this->render_resource_allocator.free(this->renderer, this->gpu_ctx);
        this->renderer.free(this->gpu_ctx);
        this->render_target.free(this->gpu_ctx);
        this->gpu_ctx.free();
    };

    inline void reset_database()
    {
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
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

        ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
        ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

        CachedCompiledShaders l_return;

        l_return.vertex_dummy_shader = Span<int8>::allocate_slice(l_vertex_shader_compiled.get_compiled_binary());
        l_return.fragment_dummy_shader = Span<int8>::allocate_slice(l_fragment_shader_compiled.get_compiled_binary());

        l_vertex_shader_compiled.free();
        l_fragment_shader_compiled.free();

        l_shader_compiler.free();

        return l_return;
    };

    inline void free()
    {
        this->vertex_dummy_shader.free();
        this->fragment_dummy_shader.free();
    }
};

struct AssetResource_TestAssertion
{
    struct AssertResource
    {
        hash_t id;
        uimax counter;
        int8 allocated;
    };

    inline static void assert_material_allocation(AssetResourceTestContext p_ctx, const Token<MaterialResource> p_material_resource, const AssertResource& p_material_assert,
                                                  const Slice<AssertResource>& p_material_textures, const AssertResource& p_shader_assert, const AssertResource& p_vertex_shader_assert,
                                                  const AssertResource& p_fragment_shader_assert)
    {
        MaterialResource& l_material = p_ctx.render_resource_allocator.material_unit.materials.pool.get(p_material_resource);
        assert_resource(p_ctx.render_resource_allocator.material_unit.materials, l_material, p_material_assert);

        if (p_material_assert.counter > 0)
        {
            Slice<MaterialResource::DynamicDependency> l_material_parameter_dependencies =
                p_ctx.render_resource_allocator.material_unit.material_dynamic_dependencies.get_vector(l_material.dependencies.dynamic_dependencies);
            assert_true(l_material_parameter_dependencies.Size == p_material_textures.Size);
            for (loop(i, 0, l_material_parameter_dependencies.Size))
            {
                TextureResource& l_texture_resource = p_ctx.render_resource_allocator.texture_unit.textures.pool.get(l_material_parameter_dependencies.get(i).dependency);
                assert_resource(p_ctx.render_resource_allocator.texture_unit.textures, l_texture_resource, p_material_textures.get(i));
            }
        }
        else
        {
            assert_true(p_ctx.render_resource_allocator.material_unit.material_dynamic_dependencies.is_element_free(l_material.dependencies.dynamic_dependencies));
        }

        ShaderResource& l_shader = p_ctx.render_resource_allocator.shader_unit.shaders.pool.get(l_material.dependencies.shader);
        assert_resource(p_ctx.render_resource_allocator.shader_unit.shaders, l_shader, p_shader_assert);

        ShaderModuleResource& l_vertex_shader_resource = p_ctx.render_resource_allocator.shader_module_unit.shader_modules.pool.get(l_shader.dependencies.vertex_shader);
        assert_resource(p_ctx.render_resource_allocator.shader_module_unit.shader_modules, l_vertex_shader_resource, p_vertex_shader_assert);

        ShaderModuleResource& l_fragment_shader_resource = p_ctx.render_resource_allocator.shader_module_unit.shader_modules.pool.get(l_shader.dependencies.fragment_shader);
        assert_resource(p_ctx.render_resource_allocator.shader_module_unit.shader_modules, l_fragment_shader_resource, p_fragment_shader_assert);
    };

    inline static void assert_mesh_allocation(AssetResourceTestContext p_ctx, const Token<MeshResource> p_mesh_resource, const AssertResource& p_mesh_assert)
    {
        MeshResource& l_mesh_resource = p_ctx.render_resource_allocator.mesh_unit.meshes.pool.get(p_mesh_resource);
        assert_resource(p_ctx.render_resource_allocator.mesh_unit.meshes, l_mesh_resource, p_mesh_assert);
    };

    inline static void assert_texture_allocation(AssetResourceTestContext p_ctx, const Token<TextureResource> p_texture_resource, const AssertResource& p_texture_assert)
    {
        TextureResource& l_texture_resource = p_ctx.render_resource_allocator.texture_unit.textures.pool.get(p_texture_resource);
        assert_resource(p_ctx.render_resource_allocator.texture_unit.textures, l_texture_resource, p_texture_assert);
    };

  private:
    template <class t_ResourceType> inline static void assert_resource(PoolHashedCounted<hash_t, t_ResourceType>& p_map, t_ResourceType& p_resource, const AssertResource& p_assert_resource)
    {
        assert_true(p_resource.header.id == p_assert_resource.id);
        assert_true(p_resource.header.allocated == p_assert_resource.allocated);
        if (p_assert_resource.counter == 0)
        {
            assert_true(!p_map.CountMap.has_key_nothashed(p_resource.header.id));
        }
        else
        {
            assert_true(p_map.CountMap.get_value_nothashed(p_resource.header.id)->counter == p_assert_resource.counter);
        }
    };
};

inline void render_middleware_inline_allocation(CachedCompiledShaders& p_cached_compiled_shader)
{
    AssetResourceTestContext l_ctx = AssetResourceTestContext::allocate();
    {
        SliceN<ShaderLayoutParameterType, 2> l_shader_parameter_layout_arr{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT};
        Slice<ShaderLayoutParameterType> l_shader_parameter_layout = slice_from_slicen(&l_shader_parameter_layout_arr);

        v3f l_positions[8] = {v3f{-1.0f, -1.0f, 1.0f}, v3f{-1.0f, 1.0f, 1.0f}, v3f{-1.0f, -1.0f, -1.0f}, v3f{-1.0f, 1.0f, -1.0f},
                              v3f{1.0f, -1.0f, 1.0f},  v3f{1.0f, 1.0f, 1.0f},  v3f{1.0f, -1.0f, -1.0f},  v3f{1.0f, 1.0f, -1.0f}};

        v2f l_uvs[14] = {v2f{0.625f, 0.0f},  v2f{0.375f, 0.25f}, v2f{0.375f, 0.0f},  v2f{0.625f, 0.25f}, v2f{0.375f, 0.5f},  v2f{0.625f, 0.5f},  v2f{0.375f, 0.75f},
                         v2f{0.625f, 0.75f}, v2f{0.375f, 1.00f}, v2f{0.125f, 0.75f}, v2f{0.125f, 0.50f}, v2f{0.875f, 0.50f}, v2f{0.625f, 1.00f}, v2f{0.875f, 0.75f}};

        Vertex l_vertices[14] = {Vertex{l_positions[1], l_uvs[0]},  Vertex{l_positions[2], l_uvs[1]}, Vertex{l_positions[0], l_uvs[2]},  Vertex{l_positions[3], l_uvs[3]},
                                 Vertex{l_positions[6], l_uvs[4]},  Vertex{l_positions[7], l_uvs[5]}, Vertex{l_positions[4], l_uvs[6]},  Vertex{l_positions[5], l_uvs[7]},
                                 Vertex{l_positions[0], l_uvs[8]},  Vertex{l_positions[0], l_uvs[9]}, Vertex{l_positions[2], l_uvs[10]}, Vertex{l_positions[3], l_uvs[11]},
                                 Vertex{l_positions[1], l_uvs[12]}, Vertex{l_positions[1], l_uvs[13]}};
        uint32 l_indices[14 * 3] = {0, 1, 2, 3, 4, 1, 5, 6, 4, 7, 8, 6, 4, 9, 10, 11, 7, 5, 0, 3, 1, 3, 5, 4, 5, 7, 6, 7, 12, 8, 4, 6, 9, 11, 13, 7};

        Slice<Vertex> l_vertices_span = Slice<Vertex>::build_memory_elementnb(l_vertices, 14);
        Slice<uint32> l_indices_span = Slice<uint32>::build_memory_elementnb(l_indices, 14 * 3);

        hash_t l_vertex_shader_id = 12;
        ShaderModuleResource::Asset l_vertex_shader = ShaderModuleResource::Asset::build_from_binary(Span<int8>::allocate_slice(p_cached_compiled_shader.vertex_dummy_shader.slice));

        hash_t l_fragment_shader_id = 14;
        ShaderModuleResource::Asset l_fragment_shader = ShaderModuleResource::Asset::build_from_binary(Span<int8>::allocate_slice(p_cached_compiled_shader.fragment_dummy_shader.slice));

        hash_t l_shader_asset_id = 1482658;
        ShaderResource::Asset l_shader_asset =
            ShaderResource::Asset::allocate_from_values(ShaderResource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        hash_t l_mesh_id = 1486;
        MeshResource::Asset l_mesh_asset = MeshResource::Asset::allocate_from_values(MeshResource::Asset::Value{l_vertices_span, l_indices_span});

        hash_t l_material_texture_id = 14874879;
        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureResource::Asset l_material_texture_asset = TextureResource::Asset::allocate_from_values(TextureResource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialResource::Asset l_material_asset_1;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;

            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
            l_material_asset_1 = MaterialResource::Asset::allocate_from_values(MaterialResource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        Token<MeshResource> l_mesh_resource =
            MeshResourceComposition::allocate_or_increment_inline(l_ctx.render_resource_allocator.mesh_unit, MeshResource::InlineAllocationInput{l_mesh_id, l_mesh_asset});

        SliceN<TextureResource::InlineAllocationInput, 1> l_material_resource_texture_input_arr{TextureResource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset}};
        Token<MaterialResource> l_material_resource = MaterialResourceComposition::allocate_or_increment_inline(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, MaterialResource::InlineAllocationInput{0, l_material_asset_1, slice_from_slicen(&l_material_resource_texture_input_arr)},
            ShaderResource::InlineAllocationInput{l_shader_asset_id, l_shader_asset}, ShaderModuleResource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleResource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        Token<TextureResource> l_material_texture_resource = l_ctx.render_resource_allocator.material_unit.material_dynamic_dependencies
                                                                 .get_vector(l_ctx.render_resource_allocator.material_unit.materials.pool.get(l_material_resource).dependencies.dynamic_dependencies)
                                                                 .get(0)
                                                                 .dependency;

        MaterialResource::Asset l_material_asset_2;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
            l_material_asset_2 = MaterialResource::Asset::allocate_from_values(MaterialResource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        Token<MeshResource> l_mesh_resource_2 =
            MeshResourceComposition::allocate_or_increment_inline(l_ctx.render_resource_allocator.mesh_unit, MeshResource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        SliceN<TextureResource::InlineAllocationInput, 1> l_material_resource_2_arr{TextureResource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset}};
        Token<MaterialResource> l_material_resource_2 = MaterialResourceComposition::allocate_or_increment_inline(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, MaterialResource::InlineAllocationInput{1, l_material_asset_2, slice_from_slicen(&l_material_resource_2_arr)},
            ShaderResource::InlineAllocationInput{l_shader_asset_id, l_shader_asset}, ShaderModuleResource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleResource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        /*
            We created mesh_1, mesh_2, material_1 and material_2
            They use the same Shader but materials are different
            They use the same Mesh.
            They are allocated and a step is executing -> render resources will be allocated.
        */
        {
            {
                SliceN<AssetResource_TestAssertion::AssertResource, 1> material_assert_resource_arr{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 2, 1}};
                AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource, AssetResource_TestAssertion::AssertResource{0, 1, 1},
                                                                        slice_from_slicen(&material_assert_resource_arr), AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 2, 1},
                                                                        AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                        AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
                AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource, AssetResource_TestAssertion::AssertResource{l_mesh_id, 2, 1});
            }
            {
                SliceN<AssetResource_TestAssertion::AssertResource, 1> material_assert_resource_arr{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 2, 1}};
                AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource_2, AssetResource_TestAssertion::AssertResource{1, 1, 1},
                                                                        slice_from_slicen(&material_assert_resource_arr), AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 2, 1},
                                                                        AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                        AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
                AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource_2, AssetResource_TestAssertion::AssertResource{l_mesh_id, 2, 1});
            }
        }

        Token<MeshResource> l_mesh_resource_3 =
            MeshResourceComposition::allocate_or_increment_inline(l_ctx.render_resource_allocator.mesh_unit, MeshResource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Token<MaterialResource> l_material_resource_3 = MaterialResourceComposition::allocate_or_increment_inline(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, MaterialResource::InlineAllocationInput{0}, ShaderResource::InlineAllocationInput{l_shader_asset_id, l_shader_asset},
            ShaderModuleResource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader}, ShaderModuleResource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> material_assert_resource_arr{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 2, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource_3, AssetResource_TestAssertion::AssertResource{0, 2, 1},
                                                                    slice_from_slicen(&material_assert_resource_arr), AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 2, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource_3, AssetResource_TestAssertion::AssertResource{l_mesh_id, 3, 1});
        }

        MeshResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.mesh_unit, l_mesh_resource_3);
        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material_resource_3);

        /*
           We have created a mesh and material on the same frame.
           No render module allocation must occurs.
           The MeshRenderer has been removed from the RenderResourceAllocator.
       */
        {
            assert_true(l_ctx.render_resource_allocator.material_unit.materials_inline_allocation_events.Size == 0);
            assert_true(l_ctx.render_resource_allocator.mesh_unit.meshes_allocation_events.Size == 0);
            assert_true(l_ctx.render_resource_allocator.material_unit.materials.CountMap.get_value_nothashed(0)->counter == 1);
            assert_true(l_ctx.render_resource_allocator.mesh_unit.meshes.CountMap.get_value_nothashed(l_mesh_id)->counter == 2);
        }
        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> material_assert_resource_arr{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 2, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource_3, AssetResource_TestAssertion::AssertResource{0, 1, 1},
                                                                    slice_from_slicen(&material_assert_resource_arr), AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 2, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource_3, AssetResource_TestAssertion::AssertResource{l_mesh_id, 2, 1});
        }

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        MeshResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.mesh_unit, l_mesh_resource_2);
        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material_resource_2);

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> material_assert_resource_arr{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 1, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource_2, AssetResource_TestAssertion::AssertResource{1, 0, 1},
                                                                    slice_from_slicen(&material_assert_resource_arr), AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource_2, AssetResource_TestAssertion::AssertResource{l_mesh_id, 1, 1});
            AssetResource_TestAssertion::assert_texture_allocation(l_ctx, l_material_texture_resource, AssetResource_TestAssertion::AssertResource{l_material_texture_id, 1, 1});
        }

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        // We removed the l_node_2. The l_node_1 is still here and common resources still allocated
        {
            assert_true(l_ctx.render_resource_allocator.material_unit.materials.pool.is_element_free(l_material_resource_2));
            assert_true(l_ctx.render_resource_allocator.mesh_unit.meshes.CountMap.get_value_nothashed(l_mesh_id)->counter == 1);
        }

        MeshResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.mesh_unit, l_mesh_resource);
        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material_resource);

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> material_assert_resource_arr{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 0, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource, AssetResource_TestAssertion::AssertResource{0, 0, 1}, slice_from_slicen(&material_assert_resource_arr),
                                                                    AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 0, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 0, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 0, 1});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource, AssetResource_TestAssertion::AssertResource{l_mesh_id, 0, 1});
            AssetResource_TestAssertion::assert_texture_allocation(l_ctx, l_material_texture_resource, AssetResource_TestAssertion::AssertResource{l_material_texture_id, 0, 1});
        }
    }

    // l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

    l_ctx.free();
};

inline void render_middleware_inline_alloc_dealloc_same_frame(CachedCompiledShaders p_cached_compiled_shaders)
{
    AssetResourceTestContext l_ctx = AssetResourceTestContext::allocate();
    {
        SliceN<ShaderLayoutParameterType, 2> l_shader_parameter_layout_arr{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT};
        Slice<ShaderLayoutParameterType> l_shader_parameter_layout = slice_from_slicen(&l_shader_parameter_layout_arr);

        v3f l_positions[1] = {v3f{-1.0f, -1.0f, 1.0f}};

        v2f l_uvs[1] = {v2f{0.625f, 0.0f}};

        Vertex l_vertices[1] = {Vertex{l_positions[0], l_uvs[0]}};
        uint32 l_indices[3] = {
            0,
            1,
            2,
        };

        Slice<Vertex> l_vertices_span = Slice<Vertex>::build_memory_elementnb(l_vertices, 1);
        Slice<uint32> l_indices_span = Slice<uint32>::build_memory_elementnb(l_indices, 3);

        hash_t l_vertex_shader_id = 12;
        ShaderModuleResource::Asset l_vertex_shader = ShaderModuleResource::Asset::build_from_binary(Span<int8>::allocate_slice(p_cached_compiled_shaders.vertex_dummy_shader.slice));

        hash_t l_fragment_shader_id = 14;
        ShaderModuleResource::Asset l_fragment_shader = ShaderModuleResource::Asset::build_from_binary(Span<int8>::allocate_slice(p_cached_compiled_shaders.fragment_dummy_shader.slice));

        hash_t l_shader_asset_id = 1482658;
        ShaderResource::Asset l_shader_asset =
            ShaderResource::Asset::allocate_from_values(ShaderResource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        hash_t l_mesh_id = 1486;
        MeshResource::Asset l_mesh_asset = MeshResource::Asset::allocate_from_values(MeshResource::Asset::Value{l_vertices_span, l_indices_span});

        hash_t l_material_texture_id = 14874879;
        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureResource::Asset l_material_texture_asset = TextureResource::Asset::allocate_from_values(TextureResource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialResource::Asset l_material_asset_1;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
            l_material_asset_1 = MaterialResource::Asset::allocate_from_values(MaterialResource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        Token<MeshResource> l_mesh_resource =
            MeshResourceComposition::allocate_or_increment_inline(l_ctx.render_resource_allocator.mesh_unit, MeshResource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        SliceN<TextureResource::InlineAllocationInput, 1> l_material_resource_textureinput_arr{TextureResource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset}};
        Token<MaterialResource> l_material_resource = MaterialResourceComposition::allocate_or_increment_inline(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, MaterialResource::InlineAllocationInput{0, l_material_asset_1, slice_from_slicen(&l_material_resource_textureinput_arr)},
            ShaderResource::InlineAllocationInput{l_shader_asset_id, l_shader_asset}, ShaderModuleResource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleResource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader});

        Token<TextureResource> l_material_texture_resource = l_ctx.render_resource_allocator.material_unit.material_dynamic_dependencies
                                                                 .get_vector(l_ctx.render_resource_allocator.material_unit.materials.pool.get(l_material_resource).dependencies.dynamic_dependencies)
                                                                 .get(0)
                                                                 .dependency;

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> l_assert_material_texure{AssetResource_TestAssertion::AssertResource{l_material_texture_id, 1, 0}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource, AssetResource_TestAssertion::AssertResource{0, 1, 0}, slice_from_slicen(&l_assert_material_texure),
                                                                    AssetResource_TestAssertion::AssertResource{l_shader_asset_id, 1, 0},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 0},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 0});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource, AssetResource_TestAssertion::AssertResource{l_mesh_id, 1, 0});
            AssetResource_TestAssertion::assert_texture_allocation(l_ctx, l_material_texture_resource, AssetResource_TestAssertion::AssertResource{l_material_texture_id, 1, 0});
        }

        MeshResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.mesh_unit, l_mesh_resource);
        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material_resource);

        {
            assert_true(l_ctx.render_resource_allocator.mesh_unit.meshes.empty());
            assert_true(l_ctx.render_resource_allocator.shader_unit.shaders.empty());
            assert_true(l_ctx.render_resource_allocator.shader_module_unit.shader_modules.empty());
            assert_true(l_ctx.render_resource_allocator.texture_unit.textures.empty());
            assert_true(l_ctx.render_resource_allocator.material_unit.materials.empty());
            assert_true(!l_ctx.render_resource_allocator.material_unit.material_dynamic_dependencies.has_allocated_elements());
        }

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);
    }
    l_ctx.free();
}

inline void render_middleware_database_allocation(CachedCompiledShaders p_cached_compiled_shaders)
{
    AssetResourceTestContext l_ctx = AssetResourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = slice_int8_build_rawstr("mesh");

    const hash_t l_vertex_shader_id = HashSlice(l_vertex_shader_path);
    const hash_t l_fragment_shader_id = HashSlice(l_fragment_shader_path);
    const hash_t l_shader_id = HashSlice(l_shader_path);
    const hash_t l_texture_id = HashSlice(l_texture_path);
    const hash_t l_material_id = HashSlice(l_material_path);
    const hash_t l_mesh_id = HashSlice(l_mesh_path);

    {
        SliceN<ShaderLayoutParameterType, 2> l_shader_asset_layout_arr{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT};
        ShaderResource::Asset l_shader_asset = ShaderResource::Asset::allocate_from_values(
            ShaderResource::Asset::Value{slice_from_slicen(&l_shader_asset_layout_arr), 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureResource::Asset l_texture_asset = TextureResource::Asset::allocate_from_values(TextureResource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialResource::Asset l_material_asset;
        {
            SliceN<hash_t, 1> tmp_texture_ids{l_texture_id};
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), slice_from_slicen(&tmp_texture_ids).build_asint8());
            l_material_asset = MaterialResource::Asset::allocate_from_values(MaterialResource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        MeshResource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice<Vertex>::build_memory_elementnb(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice<uint32>::build_memory_elementnb(l_indices, 14 * 3);

            l_mesh_asset = MeshResource::Asset::allocate_from_values(MeshResource::Asset::Value{l_vertices_span, l_indices_span});
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

        Token<MeshResource> l_mesh_resource = MeshResourceComposition::allocate_or_increment_database(l_ctx.render_resource_allocator.mesh_unit, MeshResource::DatabaseAllocationInput{l_mesh_id});
        SliceN<TextureResource::DatabaseAllocationInput, 1> l_material_resource_textures_arr{TextureResource::DatabaseAllocationInput{l_texture_id}};
        Token<MaterialResource> l_material_resource = MaterialResourceComposition::allocate_or_increment_database(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, MaterialResource::DatabaseAllocationInput{l_material_id, slice_from_slicen(&l_material_resource_textures_arr)},
            ShaderResource::DatabaseAllocationInput{l_shader_id}, ShaderModuleResource::DatabaseAllocationInput{l_vertex_shader_id},
            ShaderModuleResource::DatabaseAllocationInput{l_fragment_shader_id});

        assert_true(l_ctx.render_resource_allocator.mesh_unit.meshes_database_allocation_events.Size == 1);
        assert_true(l_ctx.render_resource_allocator.material_unit.materials_database_allocation_events.Size == 1);

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> l_material_assert_textures{AssetResource_TestAssertion::AssertResource{l_texture_id, 1, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource, AssetResource_TestAssertion::AssertResource{l_material_id, 1, 1},
                                                                    slice_from_slicen(&l_material_assert_textures), AssetResource_TestAssertion::AssertResource{l_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource, AssetResource_TestAssertion::AssertResource{l_mesh_id, 1, 1});
        }

        MeshResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.mesh_unit, l_mesh_resource);
        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material_resource);

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> l_material_assert_textures{AssetResource_TestAssertion::AssertResource{l_texture_id, 0, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material_resource, AssetResource_TestAssertion::AssertResource{l_material_id, 0, 1},
                                                                    slice_from_slicen(&l_material_assert_textures), AssetResource_TestAssertion::AssertResource{l_shader_id, 0, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 0, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 0, 1});
            AssetResource_TestAssertion::assert_mesh_allocation(l_ctx, l_mesh_resource, AssetResource_TestAssertion::AssertResource{l_mesh_id, 0, 1});
        }
    }
    l_ctx.free();
};

inline void render_middleware_get_dependencies_from_database(CachedCompiledShaders& p_cached_compiled_shaders)
{
    AssetResourceTestContext l_ctx = AssetResourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = slice_int8_build_rawstr("mesh");

    const hash_t l_vertex_shader_id = HashSlice(l_vertex_shader_path);
    const hash_t l_fragment_shader_id = HashSlice(l_fragment_shader_path);
    const hash_t l_shader_id = HashSlice(l_shader_path);
    const hash_t l_texture_id = HashSlice(l_texture_path);
    const hash_t l_material_id = HashSlice(l_material_path);
    const hash_t l_mesh_id = HashSlice(l_mesh_path);

    {
        SliceN<ShaderLayoutParameterType, 2> l_shader_layout{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT};
        ShaderResource::Asset l_shader_asset =
            ShaderResource::Asset::allocate_from_values(ShaderResource::Asset::Value{slice_from_slicen(&l_shader_layout), 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureResource::Asset l_texture_asset = TextureResource::Asset::allocate_from_values(TextureResource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialResource::Asset l_material_asset;
        {
            SliceN<hash_t, 1> tmp_material_textures_arr{HashSlice(l_texture_path)};
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), slice_from_slicen(&tmp_material_textures_arr).build_asint8());
            l_material_asset = MaterialResource::Asset::allocate_from_values(MaterialResource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        MeshResource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice<Vertex>::build_memory_elementnb(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice<uint32>::build_memory_elementnb(l_indices, 14 * 3);

            l_mesh_asset = MeshResource::Asset::allocate_from_values(MeshResource::Asset::Value{l_vertices_span, l_indices_span});
        }

        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_vertex_shader_path, p_cached_compiled_shaders.vertex_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_fragment_shader_path, p_cached_compiled_shaders.fragment_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_shader_path, l_shader_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_texture_path, l_texture_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_material_path, l_material_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_mesh_path, l_mesh_asset.allocated_binary.slice);

        SliceN<hash_t, 1> tmp_material_textures_arr{HashSlice(l_texture_path)};
        MaterialResource::AssetDependencies l_material_asset_dependencies = MaterialResource::AssetDependencies::allocate_from_values(MaterialResource::AssetDependencies::Value{
            HashSlice(l_shader_path), ShaderResource::AssetDependencies::Value{HashSlice(l_vertex_shader_path), HashSlice(l_fragment_shader_path)}, slice_from_slicen(&tmp_material_textures_arr)});

        l_ctx.asset_database.insert_asset_dependencies_blob(l_ctx.database_connection, l_material_path, l_material_asset_dependencies.allocated_binary.slice);

        l_material_asset_dependencies.free();

        l_shader_asset.free();
        l_texture_asset.free();
        l_material_asset.free();
        l_mesh_asset.free();
    }

    {
        Token<MaterialResource> l_material = MaterialResourceComposition::allocate_or_increment_database_and_load_dependecies(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, l_ctx.database_connection, l_ctx.asset_database, HashSlice(l_material_path));

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> l_material_assert_textures{AssetResource_TestAssertion::AssertResource{l_texture_id, 1, 0}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetResource_TestAssertion::AssertResource{l_material_id, 1, 0}, slice_from_slicen(&l_material_assert_textures),
                                                                    AssetResource_TestAssertion::AssertResource{l_shader_id, 1, 0},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 0},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 0});
        }

        assert_true(l_ctx.render_resource_allocator.material_unit.materials_database_allocation_events.Size == 1);
        assert_true(l_ctx.render_resource_allocator.texture_unit.texture_database_allocation_events.Size == 1);
        assert_true(l_ctx.render_resource_allocator.shader_unit.shaders_database_allocation_events.Size == 1);
        assert_true(l_ctx.render_resource_allocator.shader_module_unit.shader_module_database_allocation_events.Size == 2);

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material);
    }
    l_ctx.free();
}

// When we try to allocate multiple time the same resource, no database request is performed
inline void render_middleware_multiple_database_allocation(CachedCompiledShaders& p_cached_compile_shaders)
{
    AssetResourceTestContext l_ctx = AssetResourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = slice_int8_build_rawstr("mesh");

    const hash_t l_vertex_shader_id = HashSlice(l_vertex_shader_path);
    const hash_t l_fragment_shader_id = HashSlice(l_fragment_shader_path);
    const hash_t l_shader_id = HashSlice(l_shader_path);
    const hash_t l_texture_id = HashSlice(l_texture_path);
    const hash_t l_material_id = HashSlice(l_material_path);
    const hash_t l_mesh_id = HashSlice(l_mesh_path);

    {
        SliceN<ShaderLayoutParameterType, 2> l_shader_asset_layout_arr{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT};
        ShaderResource::Asset l_shader_asset = ShaderResource::Asset::allocate_from_values(
            ShaderResource::Asset::Value{slice_from_slicen(&l_shader_asset_layout_arr), 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureResource::Asset l_texture_asset = TextureResource::Asset::allocate_from_values(TextureResource::Asset::Value{v3ui{8, 8, 1}, 4, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialResource::Asset l_material_asset;
        {
            SliceN<hash_t, 1> tmp_material_texture_arr{HashSlice(l_texture_path)};
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), slice_from_slicen(&tmp_material_texture_arr).build_asint8());
            l_material_asset = MaterialResource::Asset::allocate_from_values(MaterialResource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        MeshResource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice<Vertex>::build_memory_elementnb(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice<uint32>::build_memory_elementnb(l_indices, 14 * 3);

            l_mesh_asset = MeshResource::Asset::allocate_from_values(MeshResource::Asset::Value{l_vertices_span, l_indices_span});
        }

        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_vertex_shader_path, p_cached_compile_shaders.vertex_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_fragment_shader_path, p_cached_compile_shaders.fragment_dummy_shader.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_shader_path, l_shader_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_texture_path, l_texture_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_material_path, l_material_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_ctx.database_connection, l_mesh_path, l_mesh_asset.allocated_binary.slice);

        SliceN<hash_t, 1> tmp_material_textures{HashSlice(l_texture_path)};
        MaterialResource::AssetDependencies l_material_asset_dependencies = MaterialResource::AssetDependencies::allocate_from_values(MaterialResource::AssetDependencies::Value{
            HashSlice(l_shader_path), ShaderResource::AssetDependencies::Value{HashSlice(l_vertex_shader_path), HashSlice(l_fragment_shader_path)}, slice_from_slicen(&tmp_material_textures)});

        l_ctx.asset_database.insert_asset_dependencies_blob(l_ctx.database_connection, l_material_path, l_material_asset_dependencies.allocated_binary.slice);

        l_material_asset_dependencies.free();

        l_shader_asset.free();
        l_texture_asset.free();
        l_material_asset.free();
        l_mesh_asset.free();
    }

    Token<MaterialResource> l_material;
    {
        l_material = MaterialResourceComposition::allocate_or_increment_database_and_load_dependecies(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                                                                      l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit,
                                                                                                      l_ctx.database_connection, l_ctx.asset_database, HashSlice(l_material_path));
        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);
    }

    {
        SliceN<AssetResource_TestAssertion::AssertResource, 1> l_material_assert_textures{AssetResource_TestAssertion::AssertResource{l_texture_id, 1, 1}};
        AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetResource_TestAssertion::AssertResource{l_material_id, 1, 1}, slice_from_slicen(&l_material_assert_textures),
                                                                AssetResource_TestAssertion::AssertResource{l_shader_id, 1, 1}, AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
    }

    // We reset the database to be sure that data cannot be requested
    l_ctx.reset_database();

    {
        Token<MaterialResource> l_material_2 = MaterialResourceComposition::allocate_or_increment_database_and_load_dependecies(
            l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit, l_ctx.render_resource_allocator.shader_module_unit,
            l_ctx.render_resource_allocator.texture_unit, l_ctx.database_connection, l_ctx.asset_database, HashSlice(l_material_path));

        assert_true(token_equals(l_material_2, l_material));

        assert_true(l_ctx.render_resource_allocator.material_unit.materials_database_allocation_events.Size == 0);
        assert_true(l_ctx.render_resource_allocator.shader_unit.shaders_database_allocation_events.Size == 0);
        assert_true(l_ctx.render_resource_allocator.shader_module_unit.shader_module_database_allocation_events.Size == 0);

        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> l_material_assert_textures{AssetResource_TestAssertion::AssertResource{l_texture_id, 1, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetResource_TestAssertion::AssertResource{l_material_id, 2, 1}, slice_from_slicen(&l_material_assert_textures),
                                                                    AssetResource_TestAssertion::AssertResource{l_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
        }
    }
    {
        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material);

        {
            SliceN<AssetResource_TestAssertion::AssertResource, 1> l_material_assert_textures{AssetResource_TestAssertion::AssertResource{l_texture_id, 1, 1}};
            AssetResource_TestAssertion::assert_material_allocation(l_ctx, l_material, AssetResource_TestAssertion::AssertResource{l_material_id, 1, 1}, slice_from_slicen(&l_material_assert_textures),
                                                                    AssetResource_TestAssertion::AssertResource{l_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_vertex_shader_id, 1, 1},
                                                                    AssetResource_TestAssertion::AssertResource{l_fragment_shader_id, 1, 1});
        }

        MaterialResourceComposition::decrement_or_release(l_ctx.render_resource_allocator.material_unit, l_ctx.render_resource_allocator.shader_unit,
                                                          l_ctx.render_resource_allocator.shader_module_unit, l_ctx.render_resource_allocator.texture_unit, l_material);
        l_ctx.render_resource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_resource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.database_connection, l_ctx.asset_database);
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