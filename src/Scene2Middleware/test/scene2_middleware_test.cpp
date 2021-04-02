#include "Scene2Middleware/scene2_middleware.hpp"
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_database_test_utils.hpp"

struct ComponentReleaser2
{
    Collision2& collision;
    D3Renderer& renderer;
    GPUContext& gpu_ctx;
    RenderRessourceAllocator2& render_ressource_allocator;
    SceneMiddleware* scene_middleware;

    inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component)
    {
        g_on_node_component_removed(this->scene_middleware, this->collision, this->renderer, this->gpu_ctx, this->render_ressource_allocator, p_component);
    };
};

struct Scene2MiddlewareContext
{
    Scene scene;
    Collision2 collision;
    GPUContext gpu_ctx;
    D3Renderer renderer;
    DatabaseConnection database_connection;
    AssetDatabase asset_database;
    RenderRessourceAllocator2 render_ressource_allocator;
    SceneMiddleware scene_middleware;

    inline static Scene2MiddlewareContext allocate()
    {
        Scene l_scene = Scene::allocate_default();
        Collision2 l_collision = Collision2::allocate();
        GPUContext l_gpu_ctx = GPUContext::allocate(Slice_build_default<GPUExtension>());
        D3Renderer l_renderer = D3Renderer::allocate(l_gpu_ctx, ColorStep::AllocateInfo{v3ui{8, 8, 1}, 1});
        String l_asset_database_path = asset_database_test_initialize(Slice_int8_build_rawstr("asset.db"));
        DatabaseConnection l_database_connection = DatabaseConnection::allocate(l_asset_database_path.to_slice());
        AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_connection);
        l_asset_database_path.free();
        SceneMiddleware l_scene_middleware = SceneMiddleware::allocate_default();
        RenderRessourceAllocator2 l_render_ressource_allocator = RenderRessourceAllocator2::allocate();
        return Scene2MiddlewareContext{l_scene, l_collision, l_gpu_ctx, l_renderer, l_database_connection, l_asset_database, l_render_ressource_allocator, l_scene_middleware};
    };

    inline void free(ComponentReleaser2& p_component_releaser)
    {
        this->scene.consume_component_events_stateful(p_component_releaser);
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        this->collision.free();
        this->render_ressource_allocator.free(this->renderer, this->gpu_ctx);
        this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_ctx, this->render_ressource_allocator, this->asset_database);
        this->renderer.free(this->gpu_ctx);
        this->gpu_ctx.free();
        this->scene.free();
    };

    inline void step(ComponentReleaser2& p_component_releaser)
    {
        this->scene.consume_component_events_stateful<ComponentReleaser2>(p_component_releaser);
        this->scene_middleware.deallocation_step(this->renderer, this->gpu_ctx, this->render_ressource_allocator);
        this->render_ressource_allocator.deallocation_step(this->renderer, this->gpu_ctx);
        this->render_ressource_allocator.allocation_step(this->renderer, this->gpu_ctx, this->database_connection, this->asset_database);
        this->scene_middleware.allocation_step(this->renderer, this->gpu_ctx, this->render_ressource_allocator, this->asset_database);
        this->scene_middleware.step(&this->scene, this->collision, this->renderer, this->gpu_ctx);
    };
};

namespace MeshRederer_AllocationTestUtil
{

inline static Token(MeshRendererComponent)
    allocate_mesh_renderer(Scene2MiddlewareContext& p_ctx, ShaderCompiler& p_shader_compiler, const Token(Node) p_scene_node, const hash_t p_vertex_shader_id, const hash_t p_fragment_shader_id,
                           const hash_t p_shader_asset_id, const hash_t p_mesh_id, const hash_t p_material_texture_id, const hash_t p_material_id)
{
    const int8* p_vertex_litteral = MULTILINE(#version 450 \n layout(location = 0) in vec3 pos; \n layout(location = 1) in vec2 uv; \n void main()\n {
        \n gl_Position = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        \n
    }\n);

    const int8* p_fragment_litteral = MULTILINE(#version 450\n layout(location = 0) out vec4 outColor;\n void main()\n {
        \n outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        \n
    }\n);

    ShaderCompiled l_vertex_shader_compiled = p_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
    ShaderCompiled l_fragment_shader_compiled = p_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));
    Slice<int8> l_vertex_shader_compiled_binary = l_vertex_shader_compiled.get_compiled_binary();
    Slice<int8> l_fragment_shader_compiled_binary = l_fragment_shader_compiled.get_compiled_binary();
    Span<int8> l_compiled_vertex = Span_allocate_slice(&l_vertex_shader_compiled_binary);
    Span<int8> l_compiled_fragment = Span_allocate_slice(&l_fragment_shader_compiled_binary);

    l_vertex_shader_compiled.free();
    l_fragment_shader_compiled.free();

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

    ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_vertex);
    ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_fragment);

    ShaderRessource::Asset l_shader_asset =
        ShaderRessource::Asset::allocate_from_values(ShaderRessource::Asset::Value{l_shader_parameter_layout, 0, ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual}});

    MeshRessource::Asset l_mesh_asset = MeshRessource::Asset::allocate_from_values(MeshRessource::Asset::Value{l_vertices_span, l_indices_span});

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
        l_varying_vector.push_back_2(Slice_build_asint8_memory_singleelement<ShaderParameter::Type>(&l_tex), Slice_build_asint8_memory_singleelement<hash_t>(&p_material_texture_id));
        l_material_asset_1 = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_varying_vector.to_varying_slice()});
        l_varying_vector.free();
        Span_free(&l_material_parameter_temp);
    }

    Declare_sized_slice(TextureRessource::InlineAllocationInput, 1, tmp_material_texture_input, tmp_material_texture_input_slice,
                        TextureRessource::InlineAllocationInput{p_material_texture_id, l_material_texture_asset});
    return RenderMiddleWare_AllocationComposition::allocate_meshrenderer_inline_with_dependencies(
        p_ctx.scene_middleware.render_middleware, p_ctx.render_ressource_allocator, ShaderModuleRessource::InlineAllocationInput{p_vertex_shader_id, l_vertex_shader},
        ShaderModuleRessource::InlineAllocationInput{p_fragment_shader_id, l_fragment_shader}, ShaderRessource::InlineAllocationInput{p_shader_asset_id, l_shader_asset},
        MaterialRessource::InlineAllocationInput{p_material_id, l_material_asset_1, tmp_material_texture_input_slice}, MeshRessource::InlineAllocationInput{p_mesh_id, l_mesh_asset}, p_scene_node);
};

}; // namespace MeshRederer_AllocationTestUtil

inline void collision_middleware_component_allocation()
{
    Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
    ComponentReleaser2 l_component_releaser = ComponentReleaser2{l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.render_ressource_allocator, &l_ctx.scene_middleware};
    ;
    /*
        We allocate the BoxCollider component with the "deferred" path.
          - The BoxColliderComponentAsset is descontructible even if the CollisionMiddleware step is not called.
        We step the scene_middleware.
          - The BoxColliderComponentAsset is still descontructible.
          - But there is no more pending BoxCollider ressource allocation in the collision_middleware.
        The component associated to the BoxCollider component is removed.
        We consume the scene deletion exent.
    */
    {
        v3f l_half_extend = {1.0f, 2.0f, 3.0f};
        Token(Node) l_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        Token(BoxColliderComponent) l_box_collider_component =
            l_ctx.scene_middleware.collision_middleware.allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{l_half_extend});

        NodeComponent l_box_collider_node_component = BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component);
        l_ctx.scene.add_node_component_by_value(l_node, l_box_collider_node_component);
        l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);

        {
            BoxColliderComponentAsset l_box_collider_component_asset =
                BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_ctx.scene_middleware, l_ctx.collision, l_box_collider_node_component);
            assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
            assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 1);
        }
        {
            l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);
            BoxColliderComponentAsset l_box_collider_component_asset =
                BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_ctx.scene_middleware, l_ctx.collision, l_box_collider_node_component);
            assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
            assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 0);
        }

        l_ctx.scene.remove_node_component_typed<BoxColliderComponent>(l_node);
        l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);
    }

    {
        v3f l_half_extend = {1.0f, 2.0f, 3.0f};
        Token(Node) l_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        Token(BoxColliderComponent) l_box_collider_component =
            l_ctx.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(l_ctx.collision, l_node, BoxColliderComponentAsset{l_half_extend});

        NodeComponent l_box_collider_node_component = BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component);
        l_ctx.scene.add_node_component_by_value(l_node, l_box_collider_node_component);
        l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);

        {
            BoxColliderComponentAsset l_box_collider_component_asset =
                BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_ctx.scene_middleware, l_ctx.collision, l_box_collider_node_component);
            assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
            assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 0);
        }

        l_ctx.scene.remove_node_component_typed<BoxColliderComponent>(l_node);
        l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);
    }

    l_ctx.free(l_component_releaser);
};

inline void collision_middleware_queuing_for_calculation()
{
    Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
    ComponentReleaser2 component_releaser = ComponentReleaser2{l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.render_ressource_allocator, &l_ctx.scene_middleware};

    {
        v3f l_half_extend = {1.0f, 2.0f, 3.0f};
        Token(Node) l_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        Token(BoxColliderComponent) l_box_collider_component =
            l_ctx.scene_middleware.collision_middleware.allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{l_half_extend});
        l_ctx.scene.add_node_component_by_value(l_node, BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component));

        assert_true(!l_ctx.scene_middleware.collision_middleware.allocator.box_collider_is_queued_for_detection(l_ctx.collision, l_box_collider_component));

        l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);

        assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_collider_is_queued_for_detection(l_ctx.collision, l_box_collider_component));

        l_ctx.scene.remove_node_component_typed<BoxColliderComponent>(l_node);
    }

    l_ctx.free(component_releaser);
};

inline void render_middleware_inline_allocation()
{
    Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
    ComponentReleaser2 component_releaser = ComponentReleaser2{l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.render_ressource_allocator, &l_ctx.scene_middleware};
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();
    {
        Token(Node) l_node_1 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        Token(Node) l_node_2 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        Token(Node) l_node_3 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);

        const int8* p_vertex_litteral = MULTILINE(#version 450 \n layout(location = 0) in vec3 pos; \n layout(location = 1) in vec2 uv; \n void main()\n {
            \n gl_Position = vec4(0.0f, 0.0f, 0.0f, 0.0f);
            \n
        }\n);

        const int8* p_fragment_litteral = MULTILINE(#version 450\n layout(location = 0) out vec4 outColor;\n void main()\n {
            \n outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
            \n
        }\n);

        ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
        ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));
        Slice<int8> l_vertex_shader_compiled_binary = l_vertex_shader_compiled.get_compiled_binary();
        Slice<int8> l_fragment_shader_compiled_binary = l_fragment_shader_compiled.get_compiled_binary();

        Span<int8> l_compiled_vertex = Span_allocate_slice(&l_vertex_shader_compiled_binary);
        Span<int8> l_compiled_fragment = Span_allocate_slice(&l_fragment_shader_compiled_binary);

        l_vertex_shader_compiled.free();
        l_fragment_shader_compiled.free();

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
        ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_vertex);

        hash_t l_fragment_shader_id = 14;
        ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_fragment);

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

        Declare_sized_slice(TextureRessource::InlineAllocationInput, 1, l_material_texture_input, l_material_texture_input_slice,
                            TextureRessource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset});
        Token(MeshRendererComponent) l_mesh_renderer = RenderMiddleWare_AllocationComposition::allocate_meshrenderer_inline_with_dependencies(
            l_ctx.scene_middleware.render_middleware, l_ctx.render_ressource_allocator, ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader}, ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset},
            MaterialRessource::InlineAllocationInput{0, l_material_asset_1, l_material_texture_input_slice}, MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset}, l_node_1);
        l_ctx.scene.add_node_component_by_value(l_node_1, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer));

        Token(Node) l_camera_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        l_ctx.scene_middleware.render_middleware.allocate_camera_inline(CameraComponent::Asset{10.0f, 2.0f, 100.0f}, l_camera_node);
        l_ctx.scene.add_node_component_by_value(l_camera_node, CameraComponentAsset_SceneCommunication::construct_nodecomponent());

        l_ctx.step(component_releaser);

        /*
            We create a node and attach a MeshRenderer component.
            Render ressources are allocated
        */
        {
            MeshRendererComponent& l_mesh_renderer_ressource = l_ctx.scene_middleware.render_middleware.mesh_renderers.get(l_mesh_renderer);
            assert_true(l_mesh_renderer_ressource.allocated);
            assert_true(token_not_equals(l_mesh_renderer_ressource.renderable_object, token_build_default(RenderableObject)));
            assert_true(token_equals(l_mesh_renderer_ressource.scene_node, l_node_1));
        }

        l_ctx.scene.remove_node(l_ctx.scene.get_node(l_node_1));
        l_ctx.scene.remove_node(l_ctx.scene.get_node(l_camera_node));

        l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);
        l_ctx.step(component_releaser);
        assert_true(l_ctx.scene_middleware.render_middleware.mesh_renderers.Memory.is_element_free(l_mesh_renderer));
    }

    l_ctx.free(component_releaser);
    l_shader_compiler.free();
};

/*
    We add a MeshComponent and remove it the same frame.
    -> No allocations is performed
*/
inline void render_middleware_inline_alloc_dealloc_same_frame()
{
    Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
    ComponentReleaser2 component_releaser = ComponentReleaser2{l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.render_ressource_allocator, &l_ctx.scene_middleware};
    {

        Token(Node) l_node_1 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);

        const int8* p_vertex_litteral = "a";
        const int8* p_fragment_litteral = "b";
        Slice<int8> p_vertex_litteral_slice = Slice_build_memory_elementnb<int8>((int8*)p_vertex_litteral, 1);
        Slice<int8> p_fragment_litteral_slice = Slice_build_memory_elementnb<int8>((int8*)p_fragment_litteral, 1);

        Span<int8> l_compiled_vertex = Span_allocate_slice(&p_vertex_litteral_slice);
        Span<int8> l_compiled_fragment = Span_allocate_slice(&p_fragment_litteral_slice);

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
        ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_vertex);

        hash_t l_fragment_shader_id = 14;
        ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset::build_from_binary(l_compiled_fragment);

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

        Declare_sized_slice(TextureRessource::InlineAllocationInput, 1, tmp_material_input, tmp_material_input_slice,
                            TextureRessource::InlineAllocationInput{l_material_texture_id, l_material_texture_asset});
        Token(MeshRendererComponent) l_mesh_renderer = RenderMiddleWare_AllocationComposition::allocate_meshrenderer_inline_with_dependencies(
            l_ctx.scene_middleware.render_middleware, l_ctx.render_ressource_allocator, ShaderModuleRessource::InlineAllocationInput{l_vertex_shader_id, l_vertex_shader},
            ShaderModuleRessource::InlineAllocationInput{l_fragment_shader_id, l_fragment_shader}, ShaderRessource::InlineAllocationInput{l_shader_asset_id, l_shader_asset},
            MaterialRessource::InlineAllocationInput{0, l_material_asset_1, tmp_material_input_slice}, MeshRessource::InlineAllocationInput{l_mesh_id, l_mesh_asset}, l_node_1);
        l_ctx.scene.add_node_component_by_value(l_node_1, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer));
        l_ctx.scene.remove_node_component(l_node_1, MeshRendererComponent::Type);

        l_ctx.step(component_releaser);

        {
            assert_true(!l_ctx.scene_middleware.render_middleware.mesh_renderers.has_allocated_elements());
            assert_true(l_ctx.render_ressource_allocator.material_unit.materials.empty());
            assert_true(!l_ctx.render_ressource_allocator.material_unit.material_dynamic_dependencies.has_allocated_elements());
        }
    }
    l_ctx.free(component_releaser);
}

inline void scene_object_movement()
{
    Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
    ComponentReleaser2 component_releaser = ComponentReleaser2{l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, l_ctx.render_ressource_allocator, &l_ctx.scene_middleware};
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();
    {
        Token(Node) l_node_1 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
        Token(MeshRendererComponent) l_mesh_renderer = MeshRederer_AllocationTestUtil::allocate_mesh_renderer(l_ctx, l_shader_compiler, l_node_1, 0, 1, 2, 3, 4, 5);
        l_ctx.scene.add_node_component_by_value(l_node_1, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer));

        l_ctx.step(component_releaser);
        assert_true(l_ctx.renderer.events.model_update_events.get_size() == 1);

        l_ctx.scene.tree.set_localposition(l_ctx.scene.get_node(l_node_1), v3f{5.0f, 3.0f, 9.0f});

        l_ctx.step(component_releaser);
        assert_true(l_ctx.renderer.events.model_update_events.get_size() == 2);

        l_ctx.scene.remove_node(l_ctx.scene.get_node(l_node_1));
        l_ctx.step(component_releaser);
    }
    l_ctx.free(component_releaser);
    l_shader_compiler.free();
};

int main()
{
    collision_middleware_component_allocation();
    collision_middleware_queuing_for_calculation();
    render_middleware_inline_allocation();
    render_middleware_inline_alloc_dealloc_same_frame();
    scene_object_movement();
    memleak_ckeck();
}