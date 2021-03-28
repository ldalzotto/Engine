#pragma once


struct CameraComponent
{
    static constexpr component_t Type = HashRaw_constexpr(STR(CameraComponent));

    struct Asset
    {
        float32 Near;
        float32 Far;
        float32 Fov;

        inline static Asset build_from_binary(const Slice<int8>& p_allocated_binary)
        {
            BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_allocated_binary);
            Asset l_asset;
            l_asset.Near = *l_deserializer.type<float32>();
            l_asset.Far = *l_deserializer.type<float32>();
            l_asset.Fov = *l_deserializer.type<float32>();
            return l_asset;
        };
    };

    int8 allocated;
    int8 force_update;
    Token(Node) scene_node;
    Asset asset;

    inline static CameraComponent build_default()
    {
        return CameraComponent{0, 0, token_build_default(Node)};
    };
};

struct MeshRendererComponent
{
    struct Dependencies
    {
        Token(MaterialRessource) material;
        Token(MeshRessource) mesh;
    };

    static constexpr component_t Type = HashRaw_constexpr(STR(MeshRendererComponent));
    int8 allocated;
    int8 force_update;
    Token(Node) scene_node;
    Token(RenderableObject) renderable_object;
    Dependencies dependencies;

    inline static MeshRendererComponent build(const Token(Node) p_scene_node, const Dependencies& p_dependencies)
    {
        return MeshRendererComponent{0, 1, p_scene_node, token_build_default(RenderableObject), p_dependencies};
    };

    struct AssetDependencies
    {
        hash_t material;
        hash_t mesh;
    };

    struct AllocationEvent
    {
        Token(MeshRendererComponent) allocated_ressource;
    };

    struct FreeEvent
    {
        Token(MeshRendererComponent) component;
    };
};

struct RenderMiddleWare
{
    Vector<MeshRendererComponent::AllocationEvent> mesh_renderer_allocation_events;
    Vector<MeshRendererComponent::FreeEvent> meshrenderer_free_events;

    PoolIndexed<MeshRendererComponent> mesh_renderers;
    CameraComponent camera_component;

    inline static RenderMiddleWare allocate()
    {
        return RenderMiddleWare{Vector<MeshRendererComponent::AllocationEvent>::allocate(0), Vector<MeshRendererComponent::FreeEvent>::allocate(0),
                                PoolIndexed<MeshRendererComponent>::allocate_default(), CameraComponent::build_default()};
    };

    inline void free(D3Renderer& p_renderer, GPUContext& p_gpu_context, AssetDatabase& p_asset_database, RenderRessourceAllocator2& p_render_ressource_allocator, Scene* p_scene)
    {

#if __DEBUG
        assert_true(this->mesh_renderer_allocation_events.empty());
#endif

        this->deallocation_step(p_renderer, p_gpu_context, p_render_ressource_allocator);
        this->step(p_renderer, p_gpu_context, p_scene);

#if __DEBUG
        assert_true(this->mesh_renderer_allocation_events.empty());
        assert_true(this->meshrenderer_free_events.empty());
        assert_true(!this->mesh_renderers.has_allocated_elements());
        assert_true(!this->camera_component.allocated);
#endif

        this->mesh_renderer_allocation_events.free();
        this->meshrenderer_free_events.free();
        this->mesh_renderers.free();
    };

    inline void deallocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, RenderRessourceAllocator2& p_render_ressource_allocator)
    {
        for (loop_reverse(i, 0, this->meshrenderer_free_events.Size))
        {
            auto& l_event = this->meshrenderer_free_events.get(i);
            MeshRendererComponent& l_mesh_renderer = this->mesh_renderers.get(l_event.component);
            MaterialRessource& l_linked_material = p_render_ressource_allocator.material_unit.materials.pool.get(l_mesh_renderer.dependencies.material);
            p_renderer.allocator.heap.unlink_material_with_renderable_object(l_linked_material.material, l_mesh_renderer.renderable_object);
            D3RendererAllocatorComposition::free_renderable_object_with_buffers(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_mesh_renderer.renderable_object);
            this->mesh_renderers.release_element(l_event.component);
            this->meshrenderer_free_events.pop_back();
        }
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, RenderRessourceAllocator2& p_render_ressource_allocator, AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->mesh_renderer_allocation_events.Size))
        {
            auto& l_event = this->mesh_renderer_allocation_events.get(i);
            MeshRendererComponent& l_mesh_renderer = this->mesh_renderers.get(l_event.allocated_ressource);

            l_mesh_renderer.renderable_object = D3RendererAllocatorComposition::allocate_renderable_object_with_buffers(
                p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, p_render_ressource_allocator.mesh_unit.meshes.pool.get(l_mesh_renderer.dependencies.mesh).mesh);
            p_renderer.allocator.heap.link_material_with_renderable_object(p_render_ressource_allocator.material_unit.materials.pool.get(l_mesh_renderer.dependencies.material).material,
                                                                           l_mesh_renderer.renderable_object);
            l_mesh_renderer.allocated = 1;

            this->mesh_renderer_allocation_events.pop_back();
        }
    };

    inline Token(MeshRendererComponent) allocate_meshrenderer(const MeshRendererComponent::Dependencies& p_dependencies, const Token(Node) p_scene_node)
    {
        Token(MeshRendererComponent) l_mesh_renderer = this->mesh_renderers.alloc_element(MeshRendererComponent::build(p_scene_node, p_dependencies));
        this->mesh_renderer_allocation_events.push_back_element(MeshRendererComponent::AllocationEvent{l_mesh_renderer});
        return l_mesh_renderer;
    };

    inline void allocate_camera_inline(const CameraComponent::Asset& p_camera_component_asset, const Token(Node) p_scene_node)
    {
        this->camera_component.force_update = 1;
        this->camera_component.allocated = 1;
        this->camera_component.scene_node = p_scene_node;
        this->camera_component.asset = p_camera_component_asset;
    };

    inline void free_camera()
    {
        this->camera_component.allocated = 0;
    };

    inline void step(D3Renderer& p_renderer, GPUContext& p_gpu_context, Scene* p_scene)
    {
        for (loop(i, 0, this->mesh_renderers.Indices.Size))
        {
            MeshRendererComponent& l_mesh_renderer = this->mesh_renderers.get(this->mesh_renderers.Indices.get(i));
            NodeEntry l_node = p_scene->get_node(l_mesh_renderer.scene_node);
            if (l_mesh_renderer.force_update || l_node.Element->state.haschanged_thisframe)
            {
                p_renderer.heap().push_modelupdateevent(D3RendererHeap::RenderableObject_ModelUpdateEvent{l_mesh_renderer.renderable_object, p_scene->tree.get_localtoworld(l_node)});

                l_mesh_renderer.force_update = 0;
            }
        }

        if (this->camera_component.allocated)
        {
            NodeEntry l_camera_node = p_scene->get_node(this->camera_component.scene_node);
            if (this->camera_component.force_update)
            {
                p_renderer.color_step.set_camera_projection(p_gpu_context, this->camera_component.asset.Near, this->camera_component.asset.Far, this->camera_component.asset.Fov);

                m44f l_local_to_world = p_scene->tree.get_localtoworld(l_camera_node);
                p_renderer.color_step.set_camera_view(p_gpu_context, p_scene->tree.get_worldposition(l_camera_node), l_local_to_world.Forward.Vec3, l_local_to_world.Up.Vec3);
                this->camera_component.force_update = 0;
            }
            else if (l_camera_node.Element->state.haschanged_thisframe)
            {
                m44f l_local_to_world = p_scene->tree.get_localtoworld(l_camera_node);
                p_renderer.color_step.set_camera_view(p_gpu_context, p_scene->tree.get_worldposition(l_camera_node), l_local_to_world.Forward.Vec3, l_local_to_world.Up.Vec3);
            }
        }
    };
};

struct RenderMiddleWare_AllocationComposition
{
    inline static Token(MeshRendererComponent)
        allocate_meshrenderer_inline_with_dependencies(RenderMiddleWare& p_render_middleware, RenderRessourceAllocator2& p_render_ressource_allocator,
                                                       const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader, const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader,
                                                       const ShaderRessource::InlineAllocationInput& p_shader, const MaterialRessource::InlineAllocationInput& p_material,
                                                       const MeshRessource::InlineAllocationInput& p_mesh, const Token(Node) p_scene_node)
    {
        Token(MaterialRessource) l_material_ressource = MaterialRessourceComposition::allocate_or_increment_inline(
            p_render_ressource_allocator.material_unit, p_render_ressource_allocator.shader_unit, p_render_ressource_allocator.shader_module_unit, p_render_ressource_allocator.texture_unit,
            p_material, p_shader, p_vertex_shader, p_fragment_shader);
        Token(MeshRessource) l_mesh_ressource = MeshRessourceComposition::allocate_or_increment_inline(p_render_ressource_allocator.mesh_unit, p_mesh);
        return p_render_middleware.allocate_meshrenderer(MeshRendererComponent::Dependencies{l_material_ressource, l_mesh_ressource}, p_scene_node);
    };

    inline static Token(MeshRendererComponent)
        allocate_meshrenderer_database_with_dependencies(RenderMiddleWare& p_render_middleware, RenderRessourceAllocator2& p_render_ressource_allocator,
                                                         const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader, const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader,
                                                         const ShaderRessource::DatabaseAllocationInput& p_shader, const MaterialRessource::DatabaseAllocationInput& p_material,
                                                         const MeshRessource::DatabaseAllocationInput& p_mesh, const Token(Node) p_scene_node)
    {
        Token(MaterialRessource) l_material_ressource = MaterialRessourceComposition::allocate_or_increment_database(
            p_render_ressource_allocator.material_unit, p_render_ressource_allocator.shader_unit, p_render_ressource_allocator.shader_module_unit, p_render_ressource_allocator.texture_unit,
            p_material, p_shader, p_vertex_shader, p_fragment_shader);
        Token(MeshRessource) l_mesh_ressource = MeshRessourceComposition::allocate_or_increment_database(p_render_ressource_allocator.mesh_unit, p_mesh);
        return p_render_middleware.allocate_meshrenderer(MeshRendererComponent::Dependencies{l_material_ressource, l_mesh_ressource}, p_scene_node);
    };

    inline static Token(MeshRendererComponent)
        allocate_meshrenderer_database_and_load_dependecies(RenderMiddleWare& p_render_middleware, RenderRessourceAllocator2& p_render_ressource_allocator, DatabaseConnection& p_database_connection,
                                                            AssetDatabase& p_assrt_database, const MeshRendererComponent::AssetDependencies& p_meshrenderer_asset_dependencied,
                                                            const Token(Node) p_scene_node)
    {
        Token(MaterialRessource) l_material_ressource = MaterialRessourceComposition::allocate_or_increment_database_and_load_dependecies(
            p_render_ressource_allocator.material_unit, p_render_ressource_allocator.shader_unit, p_render_ressource_allocator.shader_module_unit, p_render_ressource_allocator.texture_unit,
            p_database_connection, p_assrt_database, p_meshrenderer_asset_dependencied.material);
        Token(MeshRessource) l_mesh_ressource =
            MeshRessourceComposition::allocate_or_increment_database(p_render_ressource_allocator.mesh_unit, MeshRessource::DatabaseAllocationInput{p_meshrenderer_asset_dependencied.mesh});
        return p_render_middleware.allocate_meshrenderer(MeshRendererComponent::Dependencies{l_material_ressource, l_mesh_ressource}, p_scene_node);
    };

    inline static void free_meshrenderer_with_dependencies(RenderMiddleWare& p_render_middleware, RenderRessourceAllocator2& p_render_ressource_allocator,
                                                           const Token(MeshRendererComponent) p_mesh_renderer)
    {
        MeshRendererComponent& l_mesh_renderer = p_render_middleware.mesh_renderers.get(p_mesh_renderer);

        if (l_mesh_renderer.allocated)
        {
            p_render_middleware.meshrenderer_free_events.push_back_element(MeshRendererComponent::FreeEvent{p_mesh_renderer});
        }
        else
        {
            for (loop(i, 0, p_render_middleware.mesh_renderer_allocation_events.Size))
            {
                auto& l_event = p_render_middleware.mesh_renderer_allocation_events.get(i);
                if (token_equals(l_event.allocated_ressource, p_mesh_renderer))
                {
                    p_render_middleware.mesh_renderer_allocation_events.erase_element_at_always(i);
                    break;
                }
            }
            p_render_middleware.mesh_renderers.release_element(p_mesh_renderer);
        }

        p_render_ressource_allocator.mesh_unit.release_ressource(l_mesh_renderer.dependencies.mesh);
        MaterialRessourceComposition::decrement_or_release(p_render_ressource_allocator.material_unit, p_render_ressource_allocator.shader_unit, p_render_ressource_allocator.shader_module_unit,
                                                           p_render_ressource_allocator.texture_unit, l_mesh_renderer.dependencies.material);
    };
};