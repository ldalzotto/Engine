#pragma once

/*
    Some resource depedencies are stored inside the resource itself. Wouldn't it be better to store dependencies in a separated Pool ?
    Not necessarily, because we never iterate over resources, we request them by key.
*/

struct ShaderModuleResourceUnit
{
    PoolHashedCounted<hash_t, ShaderModuleResource> shader_modules;
    Vector<ShaderModuleResource::InlineAllocationEvent> shader_modules_allocation_events;
    Vector<ShaderModuleResource::DatabaseAllocationEvent> shader_module_database_allocation_events;
    Vector<ShaderModuleResource::FreeEvent> shader_modules_free_events;

    inline static ShaderModuleResourceUnit allocate()
    {
        return ShaderModuleResourceUnit{PoolHashedCounted<hash_t, ShaderModuleResource>::allocate_default(), Vector<ShaderModuleResource::InlineAllocationEvent>::allocate(0),
                                         Vector<ShaderModuleResource::DatabaseAllocationEvent>::allocate(0), Vector<ShaderModuleResource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->shader_modules_allocation_events.empty());
        assert_true(this->shader_modules_free_events.empty());
        assert_true(this->shader_module_database_allocation_events.empty());
        assert_true(this->shader_modules.empty());
#endif
        this->shader_modules_free_events.free();
        this->shader_modules_allocation_events.free();
        this->shader_module_database_allocation_events.free();
        this->shader_modules.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->shader_modules_allocation_events.empty());
        assert_true(this->shader_module_database_allocation_events.empty());
#endif
    };

    inline void deallocation_step(GPUContext& p_gpu_context)
    {
        ResourceAlgorithm::deallocation_step(this->shader_modules, this->shader_modules_free_events, [&](const ShaderModuleResource& p_resource) {
            p_gpu_context.graphics_allocator.free_shader_module(p_resource.shader_module);
        });
    };

    inline void allocation_step(GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        ResourceAlgorithm::allocation_step(this->shader_modules, this->shader_module_database_allocation_events, this->shader_modules_allocation_events, p_database_connection, p_asset_database,
                                            [&](ShaderModuleResource& p_resource, const ShaderModuleResource::Asset::Value& p_value) {
                                               p_resource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(p_value.compiled_shader);
                                            });
    };
};

struct ShaderModuleResourceComposition
{
    inline static Token<ShaderModuleResource> allocate_or_increment_inline(ShaderModuleResourceUnit& p_unit, const ShaderModuleResource::InlineAllocationInput& p_inline_input)
    {
        return ResourceAlgorithm::allocate_or_increment_inline_v2(p_unit.shader_modules, p_inline_input.id, [&](const ResourceIdentifiedHeader& p_header) {
            ShaderModuleResource l_resource = ShaderModuleResource{p_header, token_build_default<ShaderModule>()};
            return ResourceAlgorithm::push_resource_to_be_allocated_inline_v2(p_unit.shader_modules, l_resource, p_unit.shader_modules_allocation_events, p_header.id, p_inline_input.asset);
        });
    };

    inline static Token<ShaderModuleResource> allocate_or_increment_database(ShaderModuleResourceUnit& p_unit, const ShaderModuleResource::DatabaseAllocationInput& p_inline_input)
    {
        return ResourceAlgorithm::allocate_or_increment_database_v2(p_unit.shader_modules, p_inline_input.id, [&](const ResourceIdentifiedHeader& p_header) {
            ShaderModuleResource l_resource = ShaderModuleResource{p_header, token_build_default<ShaderModule>()};
            return ResourceAlgorithm::push_resource_to_be_allocated_database_v2(p_unit.shader_modules, l_resource, p_unit.shader_module_database_allocation_events, p_header.id);
        });
    };

    inline static void decrement_or_release(ShaderModuleResourceUnit& p_unit, const Token<ShaderModuleResource> p_resource)
    {
        ResourceAlgorithm::decrement_or_release_resource_by_token_v3(p_unit.shader_modules, p_unit.shader_modules_free_events, p_unit.shader_modules_allocation_events,
                                                                     p_unit.shader_module_database_allocation_events, p_resource, [](auto) {
                                                                     });
    };
};

struct TextureResourceUnit
{
    PoolHashedCounted<hash_t, TextureResource> textures;
    Vector<TextureResource::InlineAllocationEvent> textures_allocation_events;
    Vector<TextureResource::DatabaseAllocationEvent> texture_database_allocation_events;
    Vector<TextureResource::FreeEvent> textures_free_events;

    inline static TextureResourceUnit allocate()
    {
        return TextureResourceUnit{PoolHashedCounted<hash_t, TextureResource>::allocate_default(), Vector<TextureResource::InlineAllocationEvent>::allocate(0),
                                    Vector<TextureResource::DatabaseAllocationEvent>::allocate(0), Vector<TextureResource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->textures_allocation_events.empty());
        assert_true(this->textures_free_events.empty());
        assert_true(this->texture_database_allocation_events.empty());
        assert_true(this->textures.empty());
#endif
        this->textures_free_events.free();
        this->textures_allocation_events.free();
        this->texture_database_allocation_events.free();
        this->textures.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->textures_allocation_events.empty());
        assert_true(this->texture_database_allocation_events.empty());
#endif
    };

    inline void deallocation_step(GPUContext& p_gpu_context)
    {
        ResourceAlgorithm::deallocation_step(this->textures, this->textures_free_events, [&](const TextureResource& p_resource) {
            GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_resource.texture);
        });
    };

    inline void allocation_step(GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        ResourceAlgorithm::allocation_step(this->textures, this->texture_database_allocation_events, this->textures_allocation_events, p_database_connection, p_asset_database,
                                            TextureResourceAllocation{p_gpu_context});
    };

    struct TextureResourceAllocation
    {
        GPUContext& gpu_context;

        inline void operator()(TextureResource& p_resource, const TextureResource::Asset::Value& p_value) const
        {
            p_resource.texture = ShaderParameterBufferAllocationFunctions::allocate_texture_gpu_for_shaderparameter(gpu_context.graphics_allocator, gpu_context.buffer_memory,
                                                                                                                     ImageFormat::build_color_2d(p_value.size, ImageUsageFlag::UNDEFINED));
            TextureGPU& l_texture_gpu = gpu_context.graphics_allocator.heap.textures_gpu.get(p_resource.texture);
            BufferReadWrite::write_to_imagegpu(gpu_context.buffer_memory.allocator, gpu_context.buffer_memory.events, l_texture_gpu.Image,
                                               gpu_context.buffer_memory.allocator.gpu_images.get(l_texture_gpu.Image), p_value.pixels);
        }
    };
};

struct TextureResourceComposition
{
    inline static Token<TextureResource> allocate_or_increment_inline(TextureResourceUnit& p_unit, const TextureResource::InlineAllocationInput& p_inline_input)
    {
        return ResourceAlgorithm::allocate_or_increment_inline_v2(p_unit.textures, p_inline_input.id, [&](const ResourceIdentifiedHeader& p_header) {
            TextureResource l_resource = TextureResource{p_header, token_build_default<TextureGPU>()};
            return ResourceAlgorithm::push_resource_to_be_allocated_inline_v2(p_unit.textures, l_resource, p_unit.textures_allocation_events, p_header.id, p_inline_input.asset);
        });
    };

    inline static Token<TextureResource> allocate_or_increment_database(TextureResourceUnit& p_unit, const TextureResource::DatabaseAllocationInput& p_inline_input)
    {
        return ResourceAlgorithm::allocate_or_increment_database_v2(p_unit.textures, p_inline_input.id, [&](const ResourceIdentifiedHeader& p_header) {
            TextureResource l_resource = TextureResource{p_header, token_build_default<TextureGPU>()};
            return ResourceAlgorithm::push_resource_to_be_allocated_database_v2(p_unit.textures, l_resource, p_unit.texture_database_allocation_events, p_header.id);
        });
    };

    inline static void decrement_or_release(TextureResourceUnit& p_unit, const Token<TextureResource> p_resource)
    {
        ResourceAlgorithm::decrement_or_release_resource_by_token_v3(p_unit.textures, p_unit.textures_free_events, p_unit.textures_allocation_events, p_unit.texture_database_allocation_events,
                                                                     p_resource, [](auto) {
                                                                     });
    };
};

struct MeshResourceUnit
{
    PoolHashedCounted<hash_t, MeshResource> meshes;
    Vector<MeshResource::InlineAllocationEvent> meshes_allocation_events;
    Vector<MeshResource::DatabaseAllocationEvent> meshes_database_allocation_events;
    Vector<MeshResource::FreeEvent> meshes_free_events;

    inline static MeshResourceUnit allocate()
    {
        return MeshResourceUnit{PoolHashedCounted<hash_t, MeshResource>::allocate_default(), Vector<MeshResource::InlineAllocationEvent>::allocate(0),
                                 Vector<MeshResource::DatabaseAllocationEvent>::allocate(0), Vector<MeshResource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->meshes_allocation_events.empty());
        assert_true(this->meshes_database_allocation_events.empty());
        assert_true(this->meshes_free_events.empty());
        assert_true(this->meshes.empty());
#endif
        this->meshes_free_events.free();
        this->meshes_allocation_events.free();
        this->meshes_database_allocation_events.free();
        this->meshes.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->meshes_allocation_events.empty());
        assert_true(this->meshes_database_allocation_events.empty());
#endif
    };

    inline void deallocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        ResourceAlgorithm::deallocation_step(this->meshes, this->meshes_free_events, [&](const MeshResource& p_resource) {
            D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, p_resource.mesh);
        });
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        ResourceAlgorithm::allocation_step(this->meshes, this->meshes_database_allocation_events, this->meshes_allocation_events, p_database_connection, p_asset_database,
                                            [&](MeshResource& p_mesh_resource, const MeshResource::Asset::Value& p_value) {
                                               p_mesh_resource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator,
                                                                                                                                   p_value.initial_vertices, p_value.initial_indices);
                                            });
    };
};

struct MeshResourceComposition
{
    inline static Token<MeshResource> allocate_or_increment_inline(MeshResourceUnit& p_unit, const MeshResource::InlineAllocationInput& p_inline_input)
    {
        return ResourceAlgorithm::allocate_or_increment_inline_v2(p_unit.meshes, p_inline_input.id, [&](const ResourceIdentifiedHeader& p_header) {
            MeshResource l_mesh_resource = MeshResource{p_header, token_build_default<Mesh>()};
            return ResourceAlgorithm::push_resource_to_be_allocated_inline_v2(p_unit.meshes, l_mesh_resource, p_unit.meshes_allocation_events, p_inline_input.id, p_inline_input.asset);
        });
    };

    inline static Token<MeshResource> allocate_or_increment_database(MeshResourceUnit& p_unit, const MeshResource::DatabaseAllocationInput& p_inline_input)
    {
        return ResourceAlgorithm::allocate_or_increment_database_v2(p_unit.meshes, p_inline_input.id, [&](const ResourceIdentifiedHeader& p_header) {
            MeshResource l_mesh_resource = MeshResource{p_header, token_build_default<Mesh>()};
            return ResourceAlgorithm::push_resource_to_be_allocated_database_v2(p_unit.meshes, l_mesh_resource, p_unit.meshes_database_allocation_events, p_inline_input.id);
        });
    };

    inline static void decrement_or_release(MeshResourceUnit& p_unit, const Token<MeshResource> p_mesh_resource)
    {
        ResourceAlgorithm::decrement_or_release_resource_by_token_v3(p_unit.meshes, p_unit.meshes_free_events, p_unit.meshes_allocation_events, p_unit.meshes_database_allocation_events,
                                                                     p_mesh_resource, [](auto) {
                                                                     });
    };
};

struct ShaderResourceUnit
{
    PoolHashedCounted<hash_t, ShaderResource> shaders;
    Vector<ShaderResource::InlineAllocationEvent> shaders_allocation_events;
    Vector<ShaderResource::DatabaseAllocationEvent> shaders_database_allocation_events;
    Vector<ShaderResource::FreeEvent> shaders_free_events;

    inline static ShaderResourceUnit allocate()
    {
        return ShaderResourceUnit{PoolHashedCounted<hash_t, ShaderResource>::allocate_default(), Vector<ShaderResource::InlineAllocationEvent>::allocate(0),
                                   Vector<ShaderResource::DatabaseAllocationEvent>::allocate(0), Vector<ShaderResource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->shaders_allocation_events.empty());
        assert_true(this->shaders_free_events.empty());
        assert_true(this->shaders_database_allocation_events.empty());
        assert_true(this->shaders.empty());
#endif
        this->shaders_free_events.free();
        this->shaders_allocation_events.free();
        this->shaders_database_allocation_events.free();
        this->shaders.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->shaders_allocation_events.empty());
#endif
    };

    inline void deallocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        ResourceAlgorithm::deallocation_step(this->shaders, this->shaders_free_events, [&](const ShaderResource& p_resource) {
            D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, p_resource.shader);
        });
    };

    inline void allocation_step(ShaderModuleResourceUnit& p_shader_module_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection,
                                AssetDatabase& p_asset_database)
    {
        ResourceAlgorithm::allocation_step(this->shaders, this->shaders_database_allocation_events, this->shaders_allocation_events, p_database_connection, p_asset_database,
                                           ShaderResourceAllocation{p_shader_module_unit, p_renderer, p_gpu_context});
    };

    struct ShaderResourceAllocation
    {
        ShaderModuleResourceUnit& shader_module_unit;
        D3Renderer& renderer;
        GPUContext& gpu_context;

        inline void operator()(ShaderResource& p_mesh_resource, const ShaderResource::Asset::Value& p_value) const
        {
            ShaderModuleResource& l_vertex_shader = shader_module_unit.shader_modules.pool.get(p_mesh_resource.dependencies.vertex_shader);
            ShaderModuleResource& l_fragment_shader = shader_module_unit.shader_modules.pool.get(p_mesh_resource.dependencies.fragment_shader);
            p_mesh_resource.shader = D3RendererAllocatorComposition::allocate_colorstep_shader_with_shaderlayout(
                gpu_context.graphics_allocator, renderer.allocator, p_value.specific_parameters, p_value.execution_order,
                gpu_context.graphics_allocator.heap.graphics_pass.get(renderer.color_step.pass), p_value.shader_configuration,
                gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.shader_module), gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.shader_module));
        };
    };
};

struct ShaderResourceComposition
{
    inline static Token<ShaderResource> allocate_or_increment_inline(ShaderResourceUnit& p_unit, ShaderModuleResourceUnit& p_shader_module_unit,
                                                                      const ShaderResource::InlineAllocationInput& p_inline_input,
                                                                      const ShaderModuleResource::InlineAllocationInput& p_vertex_shader_input,
                                                                      const ShaderModuleResource::InlineAllocationInput& p_fragment_shader_input)
    {
        return ResourceAlgorithm::allocate_or_increment_inline_v2(
            p_unit.shaders, p_inline_input.id, InlineEventAllocator{p_unit, p_inline_input, InlineEventDependencyAllocator{p_shader_module_unit, p_vertex_shader_input, p_fragment_shader_input}});
    };

    inline static Token<ShaderResource> allocate_or_increment_database(ShaderResourceUnit& p_unit, ShaderModuleResourceUnit& p_shader_module_unit,
                                                                        const ShaderResource::DatabaseAllocationInput& p_inline_input,
                                                                        const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                        const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        return ResourceAlgorithm::allocate_or_increment_database_v2(
            p_unit.shaders, p_inline_input.id, DatabaseEventAllocator{p_unit, DatabaseEventDependencyAllocator{p_shader_module_unit, p_vertex_shader_input, p_fragment_shader_input}});
    };

    inline static void decrement_or_release(ShaderResourceUnit& p_unit, ShaderModuleResourceUnit& p_shader_module_unit, const Token<ShaderResource> p_shader_resource)
    {
        ResourceAlgorithm::decrement_or_release_resource_by_token_v3(p_unit.shaders, p_unit.shaders_free_events, p_unit.shaders_allocation_events, p_unit.shaders_database_allocation_events,
                                                                     p_shader_resource, [&](const ShaderResource& p_resource) {
                                                                         ShaderModuleResourceComposition::decrement_or_release(p_shader_module_unit, p_resource.dependencies.vertex_shader);
                                                                         ShaderModuleResourceComposition::decrement_or_release(p_shader_module_unit, p_resource.dependencies.fragment_shader);
                                                                     });
    };

    struct DatabaseEventDependencyAllocator
    {
        ShaderModuleResourceUnit& p_shader_module_unit;
        const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input;
        const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input;

        inline Token<ShaderModuleResource> vertex_shader_module() const
        {
            return ShaderModuleResourceComposition::allocate_or_increment_database(p_shader_module_unit, p_vertex_shader_input);
        };
        inline Token<ShaderModuleResource> fragment_shader_module() const
        {
            return ShaderModuleResourceComposition::allocate_or_increment_database(p_shader_module_unit, p_fragment_shader_input);
        };
    };

    struct InlineEventDependencyAllocator
    {
        ShaderModuleResourceUnit& p_shader_module_unit;
        const ShaderModuleResource::InlineAllocationInput& p_vertex_shader_input;
        const ShaderModuleResource::InlineAllocationInput& p_fragment_shader_input;

        inline Token<ShaderModuleResource> vertex_shader_module() const
        {
            return ShaderModuleResourceComposition::allocate_or_increment_inline(p_shader_module_unit, p_vertex_shader_input);
        };
        inline Token<ShaderModuleResource> fragment_shader_module() const
        {
            return ShaderModuleResourceComposition::allocate_or_increment_inline(p_shader_module_unit, p_fragment_shader_input);
        };
    };

    template <class EventDependencyAllocator> struct GenericResourceBuilder
    {
        const EventDependencyAllocator& p_event_dependency_allocator;
        inline ShaderResource operator()(const ResourceIdentifiedHeader& p_header) const
        {
            ShaderResource::Dependencies l_dependencies;
            l_dependencies.vertex_shader = p_event_dependency_allocator.vertex_shader_module();
            l_dependencies.fragment_shader = p_event_dependency_allocator.fragment_shader_module();
            return ShaderResource{p_header, token_build_default<ShaderIndex>(), l_dependencies};
        };
    };

    struct DatabaseEventAllocator
    {
        ShaderResourceUnit& p_unit;
        const DatabaseEventDependencyAllocator& p_event_dependency_allocator;

        inline Token<ShaderResource> operator()(const ResourceIdentifiedHeader& p_header) const
        {
            ShaderResource l_resource = GenericResourceBuilder<DatabaseEventDependencyAllocator>{p_event_dependency_allocator}.operator()(p_header);
            return ResourceAlgorithm::push_resource_to_be_allocated_database_v2(p_unit.shaders, l_resource, p_unit.shaders_database_allocation_events, p_header.id);
        };
    };

    struct InlineEventAllocator
    {
        ShaderResourceUnit& p_unit;
        const ShaderResource::InlineAllocationInput& p_inline_input;
        const InlineEventDependencyAllocator& p_event_dependency_allocator;

        inline Token<ShaderResource> operator()(const ResourceIdentifiedHeader& p_header) const
        {
            ShaderResource l_resource = GenericResourceBuilder<InlineEventDependencyAllocator>{p_event_dependency_allocator}.operator()(p_header);
            return ResourceAlgorithm::push_resource_to_be_allocated_inline_v2(p_unit.shaders, l_resource, p_unit.shaders_allocation_events, p_header.id, p_inline_input.asset);
        };
    };
};

struct MaterialResourceUnit
{
    PoolHashedCounted<hash_t, MaterialResource> materials;
    PoolOfVector<MaterialResource::DynamicDependency> material_dynamic_dependencies;
    Vector<MaterialResource::InlineAllocationEvent> materials_inline_allocation_events;
    Vector<MaterialResource::DatabaseAllocationEvent> materials_database_allocation_events;
    Vector<MaterialResource::FreeEvent> materials_free_events;

    inline static MaterialResourceUnit allocate()
    {
        return MaterialResourceUnit{PoolHashedCounted<hash_t, MaterialResource>::allocate_default(), PoolOfVector<MaterialResource::DynamicDependency>::allocate_default(),
                                     Vector<MaterialResource::InlineAllocationEvent>::allocate(0), Vector<MaterialResource::DatabaseAllocationEvent>::allocate(0),
                                     Vector<MaterialResource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->materials_inline_allocation_events.empty());
        assert_true(this->materials_database_allocation_events.empty());
        assert_true(this->materials_free_events.empty());
        assert_true(this->materials.empty());
        assert_true(!this->material_dynamic_dependencies.has_allocated_elements());
#endif
        this->materials_free_events.free();
        this->materials_inline_allocation_events.free();
        this->materials_database_allocation_events.free();
        this->materials.free();
        this->material_dynamic_dependencies.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->materials_inline_allocation_events.empty());
        assert_true(this->materials_database_allocation_events.empty());
#endif
    };

    inline void deallocation_step(ShaderResourceUnit& p_shader_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        ResourceAlgorithm::deallocation_step(this->materials, this->materials_free_events, [&](const MaterialResource& p_resource) {
            ShaderResource& l_linked_shader = p_shader_unit.shaders.pool.get(p_resource.dependencies.shader);
            p_renderer.allocator.heap.unlink_shader_with_material(l_linked_shader.shader, p_resource.material);
            D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, p_resource.material);
        });
    };

    inline void allocation_step(ShaderResourceUnit& p_shader_unit, TextureResourceUnit& p_texture_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection,
                                AssetDatabase& p_asset_database)
    {
        ResourceAlgorithm::allocation_step(
            this->materials, this->materials_database_allocation_events, this->materials_inline_allocation_events, p_database_connection, p_asset_database,
            [&](MaterialResource& p_resource, const MaterialResource::Asset::Value& p_value) {
                ShaderResource& l_shader = p_shader_unit.shaders.pool.get(p_resource.dependencies.shader);
                ShaderIndex& l_shader_index = p_renderer.allocator.heap.shaders.get(l_shader.shader);

                Material l_material_value = Material::allocate_empty(p_gpu_context.graphics_allocator, (uint32)ColorStep_const::shaderlayout_before.Size());

                for (loop(j, 0, p_value.parameters.parameters.get_size()))
                {
                    switch (p_value.parameters.get_parameter_type(j))
                    {
                    case ShaderParameter::Type::UNIFORM_HOST:
                    {
                        const Slice<int8> l_element = p_value.parameters.get_parameter_uniform_host_value(j);
                        l_material_value.add_and_allocate_buffer_host_parameter(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory.allocator,
                                                                                p_gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), l_element);
                    }
                    break;
                    case ShaderParameter::Type::TEXTURE_GPU:
                    {
                        const hash_t* l_texture_id = p_value.parameters.get_parameter_texture_gpu_value(j);
                        Token<TextureGPU> l_texture = p_texture_unit.textures.pool.get(p_texture_unit.textures.CountMap.get_value_nothashed(*l_texture_id)->token).texture;
                        l_material_value.add_texture_gpu_parameter(p_gpu_context.graphics_allocator, p_gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), l_texture,
                                                                   p_gpu_context.graphics_allocator.heap.textures_gpu.get(l_texture));
                    }
                    break;
                    default:
                        abort();
                    }
                };

                p_resource.material = p_renderer.allocator.allocate_material(l_material_value);
                p_renderer.allocator.heap.link_shader_with_material(l_shader.shader, p_resource.material);
            });
    };
};

struct MaterialResourceComposition
{

    inline static Token<MaterialResource> allocate_or_increment_inline(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit, ShaderModuleResourceUnit& p_shader_module_unit,
                                                                       TextureResourceUnit& p_texture_unit, const MaterialResource::InlineAllocationInput& p_inline_input,
                                                                        const ShaderResource::InlineAllocationInput& p_shader_input,
                                                                        const ShaderModuleResource::InlineAllocationInput& p_vertex_shader_input,
                                                                        const ShaderModuleResource::InlineAllocationInput& p_fragment_shader_input)
    {
        return ResourceAlgorithm::allocate_or_increment_inline_v2(
            p_unit.materials, p_inline_input.id,
            InlineEventAllocator{p_unit, p_inline_input,
                                 InlineEventDependencyAllocator{p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input.texture_dependencies_input, p_shader_input, p_vertex_shader_input,
                                                                p_fragment_shader_input}});
    };

    inline static Token<MaterialResource> allocate_or_increment_database(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit, ShaderModuleResourceUnit& p_shader_module_unit,
                                                                         TextureResourceUnit& p_texture_unit, const MaterialResource::DatabaseAllocationInput& p_inline_input,
                                                                          const ShaderResource::DatabaseAllocationInput& p_shader_input,
                                                                          const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                          const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        return ResourceAlgorithm::allocate_or_increment_database_v2(
            p_unit.materials, p_inline_input.id,
            DatabaseEventAllocator{
                p_unit, DatabaseEventDependencyAllocator{p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input, p_fragment_shader_input}});
    };

    inline static Token<MaterialResource> allocate_or_increment_database_and_load_dependecies(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit,
                                                                                              ShaderModuleResourceUnit& p_shader_module_unit, TextureResourceUnit& p_texture_unit,
                                                                                               DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database, const hash_t p_id)
    {
        return ResourceAlgorithm::allocate_or_increment_database_v2(
            p_unit.materials, p_id, DatabaseEventAllocatorRetrieveDependencies{p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_database_connection, p_asset_database});
    };

    inline static void decrement_or_release(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit, ShaderModuleResourceUnit& p_shader_module_unit, TextureResourceUnit& p_texture_unit,
                                            const Token<MaterialResource> p_material_resource)
    {
        ResourceAlgorithm::decrement_or_release_resource_by_token_v3(p_unit.materials, p_unit.materials_free_events, p_unit.materials_inline_allocation_events,
                                                                     p_unit.materials_database_allocation_events, p_material_resource, [&](const MaterialResource& p_ressource) {
                                                                         materialresource_release_recursively(p_ressource, p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit);
                                                                     });
    };

  private:
    inline static void materialresource_release_recursively(const MaterialResource& p_resource, MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit,
                                                             ShaderModuleResourceUnit& p_shader_module_unit, TextureResourceUnit& p_texture_unit)
    {
        ShaderResourceComposition::decrement_or_release(p_shader_unit, p_shader_module_unit, p_resource.dependencies.shader);
        Slice<MaterialResource::DynamicDependency> l_material_dynamic_dependencies = p_unit.material_dynamic_dependencies.get_vector(p_resource.dependencies.dynamic_dependencies);
        for (loop(i, 0, l_material_dynamic_dependencies.Size))
        {
            TextureResourceComposition::decrement_or_release(p_texture_unit, l_material_dynamic_dependencies.get(i).dependency);
        }
        p_unit.material_dynamic_dependencies.release_vector(p_resource.dependencies.dynamic_dependencies);
    };

    template <class EventDependencyAllocator> struct GenericResourceBuilder
    {
        MaterialResourceUnit& p_unit;
        const EventDependencyAllocator& p_event_dependency_allocator;

        inline MaterialResource operator()(const ResourceIdentifiedHeader& p_header) const
        {
            MaterialResource::Dependencies l_dependencies;
            l_dependencies.shader = p_event_dependency_allocator.shader();
            Span<MaterialResource::DynamicDependency> l_dynamic_textures = Span<MaterialResource::DynamicDependency>::allocate(p_event_dependency_allocator.get_texture_size());
            for (loop(i, 0, l_dynamic_textures.Capacity))
            {
                l_dynamic_textures.get(i) = MaterialResource::DynamicDependency{p_event_dependency_allocator.texture(i)};
            }
            l_dependencies.dynamic_dependencies = p_unit.material_dynamic_dependencies.alloc_vector_with_values(l_dynamic_textures.slice);
            l_dynamic_textures.free();

            return MaterialResource{p_header, token_build_default<Material>(), l_dependencies};
        };
    };

    struct DatabaseEventDependencyAllocator
    {
        ShaderResourceUnit& p_shader_unit;
        ShaderModuleResourceUnit& p_shader_module_unit;
        TextureResourceUnit& p_texture_unit;
        const MaterialResource::DatabaseAllocationInput& p_inline_input;
        const ShaderResource::DatabaseAllocationInput& p_shader_input;
        const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input;
        const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input;

        inline Token<ShaderResource> shader() const
        {
            return ShaderResourceComposition::allocate_or_increment_database(p_shader_unit, p_shader_module_unit, p_shader_input, p_vertex_shader_input, p_fragment_shader_input);
        };

        inline Token<TextureResource> texture(const uimax p_dependency_index) const
        {
            return TextureResourceComposition::allocate_or_increment_database(p_texture_unit, p_inline_input.texture_dependencies_input.get(p_dependency_index));
        };

        inline uimax get_texture_size() const
        {
            return p_inline_input.texture_dependencies_input.Size;
        };
    };

    struct DatabaseEventAllocator
    {
        MaterialResourceUnit& p_unit;
        const DatabaseEventDependencyAllocator& p_event_dependency_allocator;

        inline Token<MaterialResource> operator()(const ResourceIdentifiedHeader& p_header) const
        {
            MaterialResource l_ressource = GenericResourceBuilder<DatabaseEventDependencyAllocator>{p_unit, p_event_dependency_allocator}.operator()(p_header);
            return ResourceAlgorithm::push_resource_to_be_allocated_database_v2(p_unit.materials, l_ressource, p_unit.materials_database_allocation_events, p_header.id);
        };
    };

    struct DatabaseEventAllocatorRetrieveDependencies
    {
        MaterialResourceUnit& p_unit;
        ShaderResourceUnit& p_shader_unit;
        ShaderModuleResourceUnit& p_shader_module_unit;
        TextureResourceUnit& p_texture_unit;
        DatabaseConnection& p_database_connection;
        AssetDatabase& p_asset_database;

        inline Token<MaterialResource> operator()(const ResourceIdentifiedHeader& p_header) const
        {
            Span<int8> l_asset_dependencies_span = p_asset_database.get_asset_dependencies_blob(p_database_connection, p_header.id);
            MaterialResource::AssetDependencies l_asset_dependencies_asset = MaterialResource::AssetDependencies::build_from_binary(l_asset_dependencies_span);
            MaterialResource::AssetDependencies::Value l_asset_dependencies = MaterialResource::AssetDependencies::Value::build_from_asset(l_asset_dependencies_asset);
            Token<MaterialResource> l_resource = DatabaseEventAllocator{
                p_unit,
                DatabaseEventDependencyAllocator{
                    p_shader_unit, p_shader_module_unit, p_texture_unit,
                                                             MaterialResource::DatabaseAllocationInput{p_header.id, *(Slice<TextureResource::DatabaseAllocationInput>*)&l_asset_dependencies.textures},
                    ShaderResource::DatabaseAllocationInput{l_asset_dependencies.shader},
                                                             ShaderModuleResource::DatabaseAllocationInput{l_asset_dependencies.shader_dependencies.vertex_module},
                                                             ShaderModuleResource::DatabaseAllocationInput{
                        l_asset_dependencies.shader_dependencies
                            .fragment_module}}}.operator()(p_header);

            l_asset_dependencies_span.free();
            return l_resource;
        };
    };

    struct InlineEventDependencyAllocator
    {
        ShaderResourceUnit& p_shader_unit;
        ShaderModuleResourceUnit& p_shader_module_unit;
        TextureResourceUnit& p_texture_unit;
        const Slice<TextureResource::InlineAllocationInput>& p_texture_inputs;
        const ShaderResource::InlineAllocationInput& p_shader_input;
        const ShaderModuleResource::InlineAllocationInput& p_vertex_shader_input;
        const ShaderModuleResource::InlineAllocationInput& p_fragment_shader_input;

        inline Token<ShaderResource> shader() const
        {
            return ShaderResourceComposition::allocate_or_increment_inline(p_shader_unit, p_shader_module_unit, p_shader_input, p_vertex_shader_input, p_fragment_shader_input);
        };

        inline Token<TextureResource> texture(const uimax p_dependency_index) const
        {
            return TextureResourceComposition::allocate_or_increment_inline(p_texture_unit, p_texture_inputs.get(p_dependency_index));
        };

        inline uimax get_texture_size() const
        {
            return p_texture_inputs.Size;
        };
    };

    struct InlineEventAllocator
    {
        MaterialResourceUnit& p_unit;
        const MaterialResource::InlineAllocationInput& p_inline_input;
        const InlineEventDependencyAllocator& p_event_dependency_allocator;

        inline Token<MaterialResource> operator()(const ResourceIdentifiedHeader& p_header) const
        {
            MaterialResource l_resource = GenericResourceBuilder<InlineEventDependencyAllocator>{p_unit, p_event_dependency_allocator}.operator()(p_header);
            return ResourceAlgorithm::push_resource_to_be_allocated_inline_v2(p_unit.materials, l_resource, p_unit.materials_inline_allocation_events, l_resource.header.id, p_inline_input.asset);
        };
    };
};

/*
    The RenderResourceAllocator2 is resposible of allocating "AssetResources" for the Render system.
    AssetResources are internal data that can either be retrieved from the AssetDatabase or by providing an inline blob.
    The goal of this layer is to handle AssetDatabase connection by identifying resource by associating an asset_id to the allocate token.
    For exemple, this allow to reuse the same blob across multiple internal token.
    By default, AssetResources are read-only.
*/
struct RenderResourceAllocator2
{
    ShaderModuleResourceUnit shader_module_unit;
    TextureResourceUnit texture_unit;
    MeshResourceUnit mesh_unit;
    ShaderResourceUnit shader_unit;
    MaterialResourceUnit material_unit;

    inline static RenderResourceAllocator2 allocate()
    {
        return RenderResourceAllocator2{ShaderModuleResourceUnit::allocate(), TextureResourceUnit::allocate(), MeshResourceUnit::allocate(), ShaderResourceUnit::allocate(),
                                        MaterialResourceUnit::allocate()};
    };

    inline void free(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
#if __DEBUG
        this->shader_module_unit.assert_no_allocation_events();
        this->mesh_unit.assert_no_allocation_events();
        this->texture_unit.assert_no_allocation_events();
        this->shader_unit.assert_no_allocation_events();
        this->material_unit.assert_no_allocation_events();
#endif

        this->deallocation_step(p_renderer, p_gpu_context);

        this->shader_module_unit.free();
        this->shader_unit.free();
        this->texture_unit.free();
        this->mesh_unit.free();
        this->material_unit.free();
    };

    inline void deallocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        this->material_unit.deallocation_step(this->shader_unit, p_renderer, p_gpu_context);
        this->texture_unit.deallocation_step(p_gpu_context);
        this->shader_unit.deallocation_step(p_renderer, p_gpu_context);
        this->mesh_unit.deallocation_step(p_renderer, p_gpu_context);
        this->shader_module_unit.deallocation_step(p_gpu_context);
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        this->shader_module_unit.allocation_step(p_gpu_context, p_database_connection, p_asset_database);
        this->mesh_unit.allocation_step(p_renderer, p_gpu_context, p_database_connection, p_asset_database);
        this->shader_unit.allocation_step(this->shader_module_unit, p_renderer, p_gpu_context, p_database_connection, p_asset_database);
        this->texture_unit.allocation_step(p_gpu_context, p_database_connection, p_asset_database);
        this->material_unit.allocation_step(this->shader_unit, this->texture_unit, p_renderer, p_gpu_context, p_database_connection, p_asset_database);
    };
};
