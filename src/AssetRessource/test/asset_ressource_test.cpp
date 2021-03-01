
#include "AssetRessource/asset_ressource.hpp"
#include "shader_compiler.hpp"
#include "asset_database_test_utils.hpp"

namespace v2
{
inline void ressource_composition_test()
{
    struct RessourceTest
    {
        RessourceIdentifiedHeader header;

        struct Asset
        {
            uint8 free_called;

            inline static Asset build_default()
            {
                return Asset{0};
            };

            inline void free()
            {
                this->free_called = 1;
            };
        };

        struct AllocateEvent
        {
            Token(RessourceTest) allocated_ressource;
            Asset asset;
        };

        struct FreeEvent
        {
        };
    };

    PoolHashedCounted<hash_t, RessourceTest> hashed_counted_ressources = PoolHashedCounted<hash_t, RessourceTest>::allocate_default();
    Vector<RessourceTest::AllocateEvent> ressource_allocation_events = Vector<RessourceTest::AllocateEvent>::allocate(0);
    Vector<RessourceTest::FreeEvent> ressource_free_events = Vector<RessourceTest::FreeEvent>::allocate(0);

    hash_t l_ressource_id = 10;

    {
        RessourceComposition::allocate_ressource_composition_explicit(
            hashed_counted_ressources, ressource_allocation_events, l_ressource_id, [](const hash_t p_key) { return RessourceTest{RessourceIdentifiedHeader::build_inline_with_id(p_key)}; },
            [](const Token(RessourceTest) p_allocated_ressource_token) {
                return RessourceTest::AllocateEvent{p_allocated_ressource_token, RessourceTest::Asset::build_default()};
            });

        assert_true(ressource_allocation_events.Size == 1);
        assert_true(tk_eq(ressource_allocation_events.get(0).allocated_ressource, tk_b(RessourceTest, 0)));
        assert_true(hashed_counted_ressources.has_key_nothashed(l_ressource_id));
        assert_true(hashed_counted_ressources.CountMap.get_value_nothashed(10)->counter == 1);
    }

    auto l_allocate_event_builder = [](const Token(RessourceTest) p_allocated_ressource_token) {
        return RessourceTest::AllocateEvent{p_allocated_ressource_token, RessourceTest::Asset::build_default()};
    };
    auto l_free_event_builder = [](const Token(RessourceTest) p_allocated_ressource_token) { return RessourceTest::FreeEvent{}; };

    Token(RessourceTest) l_allocated_ressource;
    {
        l_allocated_ressource = RessourceComposition::allocate_ressource_composition_explicit(
            hashed_counted_ressources, ressource_allocation_events, l_ressource_id, [](const hash_t p_key) { return RessourceTest{}; }, l_allocate_event_builder);

        assert_true(ressource_allocation_events.Size == 1);
        assert_true(tk_eq(ressource_allocation_events.get(0).allocated_ressource, l_allocated_ressource));
        assert_true(hashed_counted_ressources.has_key_nothashed(l_ressource_id));
        assert_true(hashed_counted_ressources.CountMap.get_value_nothashed(10)->counter == 2);
    }

    {
        RessourceTest& l_ressource = hashed_counted_ressources.pool.get(l_allocated_ressource);
        RessourceComposition::free_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, ressource_free_events, l_ressource.header, l_free_event_builder,
                                                                  RessourceComposition::AllocationEventFoundSlot::FreeAsset{});

        assert_true(ressource_free_events.Size == 0);
        assert_true(hashed_counted_ressources.CountMap.get_value_nothashed(10)->counter == 1);
    }

    // If the ressource has not been already allocated, then the allocation event is removed, but no free event is generated
    {
        RessourceTest& l_ressource = hashed_counted_ressources.pool.get(l_allocated_ressource);
        RessourceComposition::free_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, ressource_free_events, l_ressource.header, l_free_event_builder,
                                                                  RessourceComposition::AllocationEventFoundSlot::FreeAsset{});

        assert_true(ressource_allocation_events.Size == 0);
        assert_true(ressource_free_events.Size == 0);
        assert_true(!hashed_counted_ressources.has_key_nothashed(10));
    }

    // If the ressource has already been allocated, then a free vent is generated
    {
        assert_true(ressource_free_events.Size == 0);

        l_allocated_ressource = RessourceComposition::allocate_ressource_composition_explicit(
            hashed_counted_ressources, ressource_allocation_events, l_ressource_id, [](const hash_t p_key) { return RessourceTest{RessourceIdentifiedHeader::build_inline_with_id(p_key)}; },
            l_allocate_event_builder);

        assert_true(ressource_allocation_events.Size == 1);

        RessourceTest& l_ressource = hashed_counted_ressources.pool.get(l_allocated_ressource);
        l_ressource.header.allocated = 1;

        ressource_allocation_events.clear();

        RessourceComposition::free_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, ressource_free_events, l_ressource.header, l_free_event_builder,
                                                                  RessourceComposition::AllocationEventFoundSlot::FreeAsset{});

        assert_true(ressource_free_events.Size == 1);
    }

    ressource_free_events.free();
    ressource_allocation_events.free();
    hashed_counted_ressources.free();
};

inline void render_asset_binary_serialization_deserialization_test()
{
    Slice<int8> l_slice_int8 = slice_int8_build_rawstr("this is a test slice");
    {
        ShaderModuleRessource::Asset l_shader_modules = ShaderModuleRessource::Asset::allocate_from_values(ShaderModuleRessource::Asset::Value{l_slice_int8});
        ShaderModuleRessource::Asset::Value l_shader_module_value = ShaderModuleRessource::Asset::Value::build_from_asset(l_shader_modules);
        assert_true(l_shader_module_value.compiled_shader.compare(l_slice_int8));
        l_shader_modules.free();
    }
    {
        ShaderRessource::Asset::Value l_value =
            ShaderRessource::Asset::Value{SliceN<ShaderLayoutParameterType, 2>{ShaderLayoutParameterType::TEXTURE_FRAGMENT, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX}.to_slice(), 2,
                                          ShaderConfiguration{0, ShaderConfiguration::CompareOp::Always}};
        ShaderRessource::Asset l_shader = ShaderRessource::Asset::allocate_from_values(l_value);
        ShaderRessource::Asset::Value l_deserialized_asset = ShaderRessource::Asset::Value::build_from_asset(l_shader);
        assert_true(l_deserialized_asset.execution_order == l_value.execution_order);
        assert_true(Slice<ShaderConfiguration>::build_memory_elementnb(&l_deserialized_asset.shader_configuration, 1)
                        .compare(Slice<ShaderConfiguration>::build_memory_elementnb(&l_value.shader_configuration, 1)));
        assert_true(l_deserialized_asset.specific_parameters.compare(l_value.specific_parameters));
        l_shader.free();
    }
    Slice<Vertex> l_initial_vertices = SliceN<Vertex, 2>{Vertex{v3f{1.0f, 2.0f, 3.0f}, v2f{1.0f, 1.0f}}}.to_slice();
    Slice<uint32> l_initial_indices = SliceN<uint32, 2>{1, 2}.to_slice();
    {
        MeshRessource::Asset::Value l_value = MeshRessource::Asset::Value{l_initial_vertices, l_initial_indices};
        MeshRessource::Asset l_mesh = MeshRessource::Asset::allocate_from_values(l_value);
        MeshRessource::Asset::Value l_deserialized_value = MeshRessource::Asset::Value::build_from_asset(l_mesh);
        assert_true(l_deserialized_value.initial_vertices.compare(l_initial_vertices));
        assert_true(l_deserialized_value.initial_indices.compare(l_initial_indices));
        l_mesh.free();
    }
    {
        TextureRessource::Asset::Value l_value = TextureRessource::Asset::Value{v3ui{8, 8, 1}, l_slice_int8};
        TextureRessource::Asset l_texture = TextureRessource::Asset::allocate_from_values(l_value);
        TextureRessource::Asset::Value l_deserialized_value = TextureRessource::Asset::Value::build_from_asset(l_texture);
        assert_true(l_deserialized_value.size == l_value.size);
        assert_true(l_deserialized_value.pixels.compare(l_value.pixels));
        l_texture.free();
    }

    Slice<int8> l_material_parameters_memory = SliceN<ShaderParameter::Type, 2>{ShaderParameter::Type::TEXTURE_GPU, ShaderParameter::Type::UNIFORM_HOST}.to_slice().build_asint8();
    Slice<SliceIndex> l_chunkds =
        SliceN<SliceIndex, 2>{SliceIndex::build(0, sizeof(ShaderParameter::Type)), SliceIndex::build(sizeof(ShaderParameter::Type), sizeof(ShaderParameter::Type))}.to_slice();
    VaryingSlice l_material_parameters_varying = VaryingSlice::build(l_material_parameters_memory, l_chunkds);
    {
        MaterialRessource::Asset::Value l_value = MaterialRessource::Asset::Value{l_material_parameters_varying};
        MaterialRessource::Asset l_material = MaterialRessource::Asset::allocate_from_values(l_value);
        MaterialRessource::Asset::Value l_deserialized_value = MaterialRessource::Asset::Value::build_from_asset(l_material);
        assert_true(l_deserialized_value.parameters.memory.compare(l_material_parameters_varying.memory));
        assert_true(l_deserialized_value.parameters.chunks.compare(l_material_parameters_varying.chunks));
        l_material.free();
    }
};

struct AssetRessourceTestContext
{
    GPUContext gpu_ctx;
    D3Renderer renderer;
    RenderRessourceAllocator2 render_ressource_allocator;
    AssetDatabase asset_database;

    inline static AssetRessourceTestContext allocate()
    {
        GPUContext l_gpu_ctx = GPUContext::allocate();
        D3Renderer l_renderer = D3Renderer::allocate(l_gpu_ctx, ColorStep::AllocateInfo{v3ui{8, 8, 1}, 0});
        String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
        AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());
        RenderRessourceAllocator2 l_render_ressource_allocator = RenderRessourceAllocator2::allocate();
        l_asset_database_path.free();
        return AssetRessourceTestContext{l_gpu_ctx, l_renderer, l_render_ressource_allocator, l_asset_database};
    };

    inline void free()
    {
        this->asset_database.free();
        this->render_ressource_allocator.free(this->renderer, this->gpu_ctx);
        this->renderer.free(this->gpu_ctx);
        this->gpu_ctx.free();
    };
};

inline void render_middleware_inline_allocation()
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();
    {
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

        ShaderCompiled l_vertex_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
        ShaderCompiled l_fragment_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

        Span<int8> l_compiled_vertex = Span<int8>::allocate_slice(l_vertex_shader_compiled.get_compiled_binary());
        Span<int8> l_compiled_fragment = Span<int8>::allocate_slice(l_fragment_shader_compiled.get_compiled_binary());

        l_vertex_shader_compiled.free();
        l_fragment_shader_compiled.free();

        Slice<ShaderLayoutParameterType> l_shader_parameter_layout =
            SliceN<ShaderLayoutParameterType, 2>{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice();

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
        ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_vertex);

        hash_t l_fragment_shader_id = 14;
        ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_fragment);

        hash_t l_shader_asset_id = 1482658;
        ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        hash_t l_mesh_id = 1486;
        MeshRessource::Asset l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});

        hash_t l_material_texture_id = 14874879;
        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureRessource::Asset l_material_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialRessource::Asset l_material_asset_1;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;

            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
            l_material_asset_1 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        Token(MeshRessource) l_mesh_ressource = l_ctx.render_ressource_allocator.allocate_mesh_inline(MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Token(MaterialRessource) l_material_ressource = RenderRessourceAllocator2Composition::allocate_material_inline_with_dependencies(
            l_ctx.render_ressource_allocator,
            MaterialRessource::InlineAllocationInput{
                0, l_material_asset_1, SliceN<TextureRessource::InlineRessourceInput, 1>{TextureRessource::InlineRessourceInput{l_material_texture_id, l_material_texture_asset}}.to_slice()},
            ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader}, ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader},
            ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset});

        MaterialRessource::Asset l_material_asset_2;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
            l_material_asset_2 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        Token(MeshRessource) l_mesh_ressource_2 = l_ctx.render_ressource_allocator.allocate_mesh_inline(MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Token(MaterialRessource) l_material_ressource_2 = RenderRessourceAllocator2Composition::allocate_material_inline_with_dependencies(
            l_ctx.render_ressource_allocator,
            MaterialRessource::InlineAllocationInput{
                1, l_material_asset_2, SliceN<TextureRessource::InlineRessourceInput, 1>{TextureRessource::InlineRessourceInput{l_material_texture_id, l_material_texture_asset}}.to_slice()},
            ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader}, ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader},
            ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset});

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

        /*
            We created mesh_1, mesh_2, material_1 and material_2
            They use the same Shader but materials are different
            They use the same Mesh.
            They are allocated and a step is executing -> render ressources will be allocated.
        */
        {
            {
                MaterialRessource& l_material = l_ctx.render_ressource_allocator.heap.materials.pool.get(l_material_ressource);
                assert_true(l_material.header.id == 0);
                assert_true(l_material.header.allocated == 1);
                Slice<MaterialRessource::DynamicDependency> l_material_parameter_dependencies =
                    l_ctx.render_ressource_allocator.heap.material_dynamic_dependencies.get_vector(l_material.dependencies.dynamic_dependencies);
                assert_true(l_material_parameter_dependencies.Size == 1);
                assert_true(l_ctx.render_ressource_allocator.heap.textures.pool.get(l_material_parameter_dependencies.get(0).dependency).header.id == l_material_texture_id);
                ShaderRessource& l_shader = l_ctx.render_ressource_allocator.heap.shaders_v3.pool.get(l_material.dependencies.shader);
                assert_true(l_shader.header.id == l_shader_asset_id);
                assert_true(l_shader.header.allocated == 1);
                MeshRessource& l_mesh = l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource);
                assert_true(l_mesh.header.id == l_mesh_id);
                assert_true(l_mesh.header.allocated == 1);
            }
            {
                MaterialRessource& l_material = l_ctx.render_ressource_allocator.heap.materials.pool.get(l_material_ressource_2);
                assert_true(l_material.header.id == 1);
                assert_true(l_material.header.allocated == 1);
                Slice<MaterialRessource::DynamicDependency> l_material_parameter_dependencies =
                    l_ctx.render_ressource_allocator.heap.material_dynamic_dependencies.get_vector(l_material.dependencies.dynamic_dependencies);
                assert_true(l_material_parameter_dependencies.Size == 1);
                assert_true(l_ctx.render_ressource_allocator.heap.textures.pool.get(l_material_parameter_dependencies.get(0).dependency).header.id == l_material_texture_id);
                ShaderRessource& l_shader = l_ctx.render_ressource_allocator.heap.shaders_v3.pool.get(l_material.dependencies.shader);
                assert_true(l_shader.header.id == l_shader_asset_id);
                assert_true(l_shader.header.allocated == 1);
                MeshRessource& l_mesh = l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource_2);
                assert_true(l_mesh.header.id == l_mesh_id);
                assert_true(l_mesh.header.allocated == 1);
            }
        }

        Token(MeshRessource) l_mesh_ressource_3 = l_ctx.render_ressource_allocator.allocate_mesh_inline(MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Token(MaterialRessource) l_material_ressource_3 = RenderRessourceAllocator2Composition::allocate_material_inline_with_dependencies(
            l_ctx.render_ressource_allocator, MaterialRessource::InlineAllocationInput{0}, ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader}, ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset});

        l_ctx.render_ressource_allocator.free_mesh(l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource_3));
        RenderRessourceAllocator2Composition::free_material_with_dependencies(l_ctx.render_ressource_allocator, l_material_ressource_3);

        /*
           We have created a mesh and material on the same frame.
           No render module allocation must occurs.
           The MeshRenderer has been removed from the RenderRessourceAllocator.
       */
        {
            assert_true(l_ctx.render_ressource_allocator.material_allocation_events.Size == 0);
            assert_true(l_ctx.render_ressource_allocator.mesh_allocation_events.Size == 0);
            assert_true(l_ctx.render_ressource_allocator.heap.materials.CountMap.get_value_nothashed(0)->counter == 1);
            assert_true(l_ctx.render_ressource_allocator.heap.mesh_v2.CountMap.get_value_nothashed(l_mesh_id)->counter == 2);
        }

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

        l_ctx.render_ressource_allocator.free_mesh(l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource_2));
        RenderRessourceAllocator2Composition::free_material_with_dependencies(l_ctx.render_ressource_allocator, l_material_ressource_2);

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

        // We removed the l_node_2. The l_node_1 is still here and common ressources still allocated
        {
            assert_true(l_ctx.render_ressource_allocator.heap.materials.pool.is_element_free(l_material_ressource_2));
            assert_true(l_ctx.render_ressource_allocator.heap.mesh_v2.CountMap.get_value_nothashed(l_mesh_id)->counter == 1);

            MaterialRessource& l_material = l_ctx.render_ressource_allocator.heap.materials.pool.get(l_material_ressource);
            assert_true(l_material.header.id == 0);
            assert_true(l_material.header.allocated == 1);
            ShaderRessource& l_shader = l_ctx.render_ressource_allocator.heap.shaders_v3.pool.get(l_material.dependencies.shader);
            assert_true(l_shader.header.id == l_shader_asset_id);
            assert_true(l_shader.header.allocated == 1);
            MeshRessource& l_mesh = l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource);
            assert_true(l_mesh.header.id == l_mesh_id);
            assert_true(l_mesh.header.allocated == 1);
        }

        l_ctx.render_ressource_allocator.free_mesh(l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource));
        RenderRessourceAllocator2Composition::free_material_with_dependencies(l_ctx.render_ressource_allocator, l_material_ressource);
    }

    // l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

    l_ctx.free();
};

inline void render_middleware_inline_alloc_dealloc_same_frame()
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();
    {
        const int8* p_vertex_litteral = "a";
        const int8* p_fragment_litteral = "b";

        Span<int8> l_compiled_vertex = Span<int8>::allocate_slice(Slice<int8>::build_memory_elementnb((int8*)p_vertex_litteral, 1));
        Span<int8> l_compiled_fragment = Span<int8>::allocate_slice(Slice<int8>::build_memory_elementnb((int8*)p_fragment_litteral, 1));

        Slice<ShaderLayoutParameterType> l_shader_parameter_layout =
            SliceN<ShaderLayoutParameterType, 2>{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice();

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
        ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_vertex);

        hash_t l_fragment_shader_id = 14;
        ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_fragment);

        hash_t l_shader_asset_id = 1482658;
        ShaderRessource::Asset l_shader_asset =
            ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        hash_t l_mesh_id = 1486;
        MeshRessource::Asset l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});

        hash_t l_material_texture_id = 14874879;
        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureRessource::Asset l_material_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialRessource::Asset l_material_asset_1;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
            l_material_asset_1 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        Token(MeshRessource) l_mesh_ressource = l_ctx.render_ressource_allocator.allocate_mesh_inline(MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset});
        Token(MaterialRessource) l_material_ressource = RenderRessourceAllocator2Composition::allocate_material_inline_with_dependencies(
            l_ctx.render_ressource_allocator,
            MaterialRessource::InlineAllocationInput{
                0, l_material_asset_1, SliceN<TextureRessource::InlineRessourceInput, 1>{TextureRessource::InlineRessourceInput{l_material_texture_id, l_material_texture_asset}}.to_slice()},
            ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader}, ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader},
            ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset});


        l_ctx.render_ressource_allocator.free_mesh(l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource));
        RenderRessourceAllocator2Composition::free_material_with_dependencies(l_ctx.render_ressource_allocator, l_material_ressource);

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

        {
            assert_true(l_ctx.render_ressource_allocator.heap.mesh_v2.empty());
            assert_true(l_ctx.render_ressource_allocator.heap.materials.empty());
            assert_true(!l_ctx.render_ressource_allocator.heap.material_dynamic_dependencies.has_allocated_elements());
        }
    }
    l_ctx.free();
}

inline void render_middleware_database_allocation()
{
    AssetRessourceTestContext l_ctx = AssetRessourceTestContext::allocate();

    const Slice<int8> l_vertex_shader_path = slice_int8_build_rawstr("shader/v.vert");
    const Slice<int8> l_fragment_shader_path = slice_int8_build_rawstr("shader/f.frag");
    const Slice<int8> l_shader_path = slice_int8_build_rawstr("shader");
    const Slice<int8> l_texture_path = slice_int8_build_rawstr("texture");
    const Slice<int8> l_material_path = slice_int8_build_rawstr("material");
    const Slice<int8> l_mesh_path = slice_int8_build_rawstr("mesh");

    {
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

        ShaderCompiled l_vertex_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
        ShaderCompiled l_fragment_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

        ShaderRessource::Asset l_shader_asset = ShaderRessource::Asset::allocate_from_values(
            ShaderRessource::Asset::Value{SliceN<ShaderLayoutParameterType, 2>{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice(), 0,
                                          ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

        Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
        TextureRessource::Asset l_texture_asset = TextureRessource::Asset::allocate_from_values(TextureRessource::Asset::Value{v3ui{8, 8, 1}, l_material_texture_span.slice});
        l_material_texture_span.free();

        MaterialRessource::Asset l_material_asset;
        {
            Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
            auto l_obj = ShaderParameter::Type::UNIFORM_HOST;
            auto l_tex = ShaderParameter::Type::TEXTURE_GPU;
            VaryingVector l_varying_vector = VaryingVector::allocate_default();
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
            l_varying_vector.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_tex), SliceN<hash_t, 1>{HashSlice(l_texture_path)}.to_slice().build_asint8());
            l_material_asset = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
            l_varying_vector.free();
            l_material_parameter_temp.free();
        }

        MeshRessource::Asset l_mesh_asset;
        {
            Vertex l_vertices[14] = {};
            uint32 l_indices[14 * 3] = {};

            Slice<Vertex> l_vertices_span = Slice<Vertex>::build_memory_elementnb(l_vertices, 14);
            Slice<uint32> l_indices_span = Slice<uint32>::build_memory_elementnb(l_indices, 14 * 3);

            l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});
        }

        l_ctx.asset_database.insert_asset_blob(l_vertex_shader_path, l_vertex_shader_compiled.get_compiled_binary());
        l_ctx.asset_database.insert_asset_blob(l_fragment_shader_path, l_fragment_shader_compiled.get_compiled_binary());
        l_ctx.asset_database.insert_asset_blob(l_shader_path, l_shader_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_texture_path, l_texture_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_material_path, l_material_asset.allocated_binary.slice);
        l_ctx.asset_database.insert_asset_blob(l_mesh_path, l_mesh_asset.allocated_binary.slice);

        l_vertex_shader_compiled.free();
        l_fragment_shader_compiled.free();
        l_shader_asset.free();
        l_texture_asset.free();
        l_material_asset.free();
        l_mesh_asset.free();
    }
    {

        Token(MeshRessource) l_mesh_ressource = l_ctx.render_ressource_allocator.allocate_mesh_database(MeshRessource::DatabaseAllocationInput{HashSlice(l_mesh_path)});
        Token(MaterialRessource) l_material_ressource = RenderRessourceAllocator2Composition::allocate_material_database_with_dependencies(
            l_ctx.render_ressource_allocator,
            MaterialRessource::DatabaseAllocationInput{HashSlice(l_material_path),
                                                       SliceN<TextureRessource::DatabaseRessourceInput, 1>{TextureRessource::DatabaseRessourceInput{HashSlice(l_texture_path)}}.to_slice()},
            ShaderModuleRessource::DatabaseAllocationInput{HashSlice(l_vertex_shader_path)},
            ShaderModuleRessource::DatabaseAllocationInput{HashSlice(l_fragment_shader_path)}, ShaderRessource::DatabaseAllocationInput{HashSlice(l_shader_path)});


        assert_true(l_ctx.render_ressource_allocator.mesh_allocation_events.Size == 1);
        assert_true(l_ctx.render_ressource_allocator.material_allocation_events.Size == 1);

        l_ctx.render_ressource_allocator.deallocation_step(l_ctx.renderer, l_ctx.gpu_ctx);
        l_ctx.render_ressource_allocator.allocation_step(l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.asset_database);

        l_ctx.render_ressource_allocator.free_mesh(l_ctx.render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_ressource));
        RenderRessourceAllocator2Composition::free_material_with_dependencies(l_ctx.render_ressource_allocator, l_material_ressource);

    }
    l_ctx.free();
};

} // namespace v2

int main()
{
    v2::ressource_composition_test();
    v2::render_asset_binary_serialization_deserialization_test();
    v2::render_middleware_inline_allocation();
    v2::render_middleware_inline_alloc_dealloc_same_frame();
    v2::render_middleware_database_allocation();

    memleak_ckeck();
}