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

    RESOURCEUNIT_DECLARE_TYPES(ShaderModuleResource);

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

    inline Vector<ShaderModuleResource::DatabaseAllocationEvent>& get_database_allocation_events()
    {
        return this->shader_module_database_allocation_events;
    };

    inline Vector<ShaderModuleResource::InlineAllocationEvent>& get_inline_allocation_events()
    {
        return this->shader_modules_allocation_events;
    };

    inline Vector<ShaderModuleResource::FreeEvent>& get_free_events()
    {
        return this->shader_modules_free_events;
    };

    inline PoolHashedCounted<hash_t, ShaderModuleResource>& get_pool()
    {
        return this->shader_modules;
    };

    struct ResourceAllocationFunction
    {
        GPUContext* gpu_context;

        inline static ResourceAllocationFunction build(GPUContext& p_gpu_context)
        {
            return ResourceAllocationFunction{&p_gpu_context};
        };

        inline void operator()(ShaderModuleResource& p_resource, const ShaderModuleResource::Asset::Value& p_value) const
        {
            p_resource.resource = gpu_context->graphics_allocator.allocate_shader_module(p_value.compiled_shader);
        };
    };

    struct ResourceFreeFunction
    {
        GPUContext* gpu_context;

        inline static ResourceFreeFunction build(GPUContext& p_gpu_context)
        {
            return ResourceFreeFunction{&p_gpu_context};
        };

        inline void operator()(const ShaderModuleResource& p_resource) const
        {
            this->gpu_context->graphics_allocator.free_shader_module(p_resource.resource);
        };
    };

    struct ResourceIncrementDatabaseFunction
    {
        inline ShaderModuleResource operator()(const hash_t p_id) const
        {
            return ShaderModuleResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<ShaderModule>()};
        };
    };

    struct ResourceIncrementInlineFunction
    {
        inline ShaderModuleResource operator()(const hash_t p_id) const
        {
            return ShaderModuleResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<ShaderModule>()};
        };
    };

    struct ResourceDecrementFunction
    {
        inline void operator()(const ShaderModuleResource&) const {}
    };
};

struct ShaderModuleResourceComposition
{
    inline static Token<ShaderModuleResource> allocate_or_increment_inline(ShaderModuleResourceUnit& p_unit, const ShaderModuleResource::InlineAllocationInput& p_inline_input)
    {
        return iResourceUnit<ShaderModuleResourceUnit>{p_unit}.allocate_or_increment_inline(p_inline_input.id, p_inline_input.asset, ShaderModuleResourceUnit::ResourceIncrementInlineFunction{});
    };

    inline static Token<ShaderModuleResource> allocate_or_increment_database(ShaderModuleResourceUnit& p_unit, const ShaderModuleResource::DatabaseAllocationInput& p_inline_input)
    {
        return iResourceUnit<ShaderModuleResourceUnit>{p_unit}.allocate_or_increment_database(p_inline_input.id, ShaderModuleResourceUnit::ResourceIncrementDatabaseFunction{});
    };

    inline static void decrement_or_release(ShaderModuleResourceUnit& p_unit, const Token<ShaderModuleResource> p_resource)
    {
        iResourceUnit<ShaderModuleResourceUnit>{p_unit}.decrement_or_release_resource_by_token(p_resource, ShaderModuleResourceUnit::ResourceDecrementFunction{});
    };
};

struct TextureResourceUnit
{
    PoolHashedCounted<hash_t, TextureResource> textures;
    Vector<TextureResource::InlineAllocationEvent> textures_allocation_events;
    Vector<TextureResource::DatabaseAllocationEvent> texture_database_allocation_events;
    Vector<TextureResource::FreeEvent> textures_free_events;

    RESOURCEUNIT_DECLARE_TYPES(TextureResource);

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

    inline Vector<TextureResource::DatabaseAllocationEvent>& get_database_allocation_events()
    {
        return this->texture_database_allocation_events;
    };

    inline Vector<TextureResource::InlineAllocationEvent>& get_inline_allocation_events()
    {
        return this->textures_allocation_events;
    };

    inline Vector<TextureResource::FreeEvent>& get_free_events()
    {
        return this->textures_free_events;
    };

    inline PoolHashedCounted<hash_t, TextureResource>& get_pool()
    {
        return this->textures;
    };

    struct ResourceAllocationFunction
    {
        GPUContext& gpu_context;

        inline void operator()(TextureResource& p_resource, const TextureResource::Asset::Value& p_value) const
        {
            p_resource.resource = ShaderParameterBufferAllocationFunctions::allocate_texture_gpu_for_shaderparameter(gpu_context.graphics_allocator, gpu_context.buffer_memory,
                                                                                                                     ImageFormat::build_color_2d(p_value.size, ImageUsageFlag::UNDEFINED));
            TextureGPU& l_texture_gpu = gpu_context.graphics_allocator.heap.textures_gpu.get(p_resource.resource);
            BufferReadWrite::write_to_imagegpu(gpu_context.buffer_memory.allocator, gpu_context.buffer_memory.events, l_texture_gpu.Image,
                                               gpu_context.buffer_memory.allocator.gpu_images.get(l_texture_gpu.Image), p_value.pixels);
        }
    };

    struct ResourceFreeFunction
    {
        GPUContext& gpu_context;

        inline void operator()(const TextureResource& p_resource) const
        {
            GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(this->gpu_context.buffer_memory, this->gpu_context.graphics_allocator, p_resource.resource);
        }
    };

    struct ResourceIncrementDatabaseFunction
    {
        inline TextureResource operator()(const hash_t p_id) const
        {
            return TextureResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<TextureGPU>()};
        };
    };

    struct ResourceIncrementInlineFunction
    {
        inline TextureResource operator()(const hash_t p_id) const
        {
            return TextureResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<TextureGPU>()};
        };
    };

    struct ResourceDecrementFunction
    {
        inline void operator()(const TextureResource&) const {

        };
    };
};

struct TextureResourceComposition
{
    inline static Token<TextureResource> allocate_or_increment_inline(TextureResourceUnit& p_unit, const TextureResource::InlineAllocationInput& p_inline_input)
    {
        return iResourceUnit<TextureResourceUnit>{p_unit}.allocate_or_increment_inline(p_inline_input.id, p_inline_input.asset, TextureResourceUnit::ResourceIncrementInlineFunction{});
    };

    inline static Token<TextureResource> allocate_or_increment_database(TextureResourceUnit& p_unit, const TextureResource::DatabaseAllocationInput& p_inline_input)
    {
        return iResourceUnit<TextureResourceUnit>{p_unit}.allocate_or_increment_database(p_inline_input.id, TextureResourceUnit::ResourceIncrementDatabaseFunction{});
    };

    inline static void decrement_or_release(TextureResourceUnit& p_unit, const Token<TextureResource> p_resource)
    {
        iResourceUnit<TextureResourceUnit>{p_unit}.decrement_or_release_resource_by_token(p_resource, TextureResourceUnit::ResourceDecrementFunction{});
    };
};

struct MeshResourceUnit
{
    PoolHashedCounted<hash_t, MeshResource> meshes;
    Vector<MeshResource::InlineAllocationEvent> meshes_allocation_events;
    Vector<MeshResource::DatabaseAllocationEvent> meshes_database_allocation_events;
    Vector<MeshResource::FreeEvent> meshes_free_events;

    RESOURCEUNIT_DECLARE_TYPES(MeshResource);

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

    inline Vector<MeshResource::DatabaseAllocationEvent>& get_database_allocation_events()
    {
        return this->meshes_database_allocation_events;
    };

    inline Vector<MeshResource::InlineAllocationEvent>& get_inline_allocation_events()
    {
        return this->meshes_allocation_events;
    };

    inline Vector<MeshResource::FreeEvent>& get_free_events()
    {
        return this->meshes_free_events;
    };

    inline PoolHashedCounted<hash_t, MeshResource>& get_pool()
    {
        return this->meshes;
    };

    struct ResourceAllocationFunc
    {
        D3Renderer& renderer;
        GPUContext& gpu_context;
        inline void operator()(MeshResource& p_resource, const MeshResource::Asset::Value& p_value) const
        {
            p_resource.resource =
                D3RendererAllocatorComposition::allocate_mesh_with_buffers(this->gpu_context.buffer_memory, this->renderer.allocator, p_value.initial_vertices, p_value.initial_indices);
        };
    };

    struct ResourceFreeFunc
    {
        D3Renderer& renderer;
        GPUContext& gpu_context;
        inline void operator()(const MeshResource& p_resource) const
        {
            D3RendererAllocatorComposition::free_mesh_with_buffers(this->gpu_context.buffer_memory, this->renderer.allocator, p_resource.resource);
        };
    };

    struct ResourceIncrementDatabaseFunction
    {
        inline MeshResource operator()(const hash_t p_id) const
        {
            return MeshResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<Mesh>()};
        };
    };

    struct ResourceIncrementInlineFunction
    {
        inline MeshResource operator()(const hash_t p_id) const
        {
            return MeshResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<Mesh>()};
        };
    };

    struct ResourceDecrementFunc
    {
        inline void operator()(const MeshResource&) const {

        };
    };
};

struct MeshResourceComposition
{
    inline static Token<MeshResource> allocate_or_increment_inline(MeshResourceUnit& p_unit, const MeshResource::InlineAllocationInput& p_inline_input)
    {
        return iResourceUnit<MeshResourceUnit>{p_unit}.allocate_or_increment_inline(p_inline_input.id, p_inline_input.asset, MeshResourceUnit::ResourceIncrementInlineFunction{});
    };

    inline static Token<MeshResource> allocate_or_increment_database(MeshResourceUnit& p_unit, const MeshResource::DatabaseAllocationInput& p_inline_input)
    {
        return iResourceUnit<MeshResourceUnit>{p_unit}.allocate_or_increment_database(p_inline_input.id, MeshResourceUnit::ResourceIncrementDatabaseFunction{});
    };

    inline static void decrement_or_release(MeshResourceUnit& p_unit, const Token<MeshResource> p_mesh_resource)
    {
        iResourceUnit<MeshResourceUnit>{p_unit}.decrement_or_release_resource_by_token(p_mesh_resource, MeshResourceUnit::ResourceDecrementFunc{});
    };
};

struct ShaderResourceUnit
{
    PoolHashedCounted<hash_t, ShaderResource> shaders;
    Vector<ShaderResource::InlineAllocationEvent> shaders_allocation_events;
    Vector<ShaderResource::DatabaseAllocationEvent> shaders_database_allocation_events;
    Vector<ShaderResource::FreeEvent> shaders_free_events;

    RESOURCEUNIT_DECLARE_TYPES(ShaderResource);

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

    inline Vector<ShaderResource::DatabaseAllocationEvent>& get_database_allocation_events()
    {
        return this->shaders_database_allocation_events;
    };

    inline Vector<ShaderResource::InlineAllocationEvent>& get_inline_allocation_events()
    {
        return this->shaders_allocation_events;
    };

    inline Vector<ShaderResource::FreeEvent>& get_free_events()
    {
        return this->shaders_free_events;
    };

    inline PoolHashedCounted<hash_t, ShaderResource>& get_pool()
    {
        return this->shaders;
    };

    struct ResourceAllocationFunc
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
                gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.resource), gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.resource));
        };
    };

    struct ResourceFreeFunc
    {
        D3Renderer& renderer;
        GPUContext& gpu_context;
        inline void operator()(const ShaderResource& p_resource) const
        {
            D3RendererAllocatorComposition::free_shader_with_shaderlayout(this->gpu_context.graphics_allocator, this->renderer.allocator, p_resource.shader);
        };
    };

    struct ResourceIncrementDatabaseFunction
    {
        ShaderModuleResourceUnit& shadermodule_unit;
        const ShaderModuleResource::DatabaseAllocationInput& vertex_inline_input;
        const ShaderModuleResource::DatabaseAllocationInput& fragment_inline_input;

        inline ShaderResource operator()(const hash_t p_id) const
        {
            ShaderResource::Dependencies l_dependencies;
            l_dependencies.vertex_shader = ShaderModuleResourceComposition::allocate_or_increment_database(this->shadermodule_unit, this->vertex_inline_input);
            l_dependencies.fragment_shader = ShaderModuleResourceComposition::allocate_or_increment_database(this->shadermodule_unit, this->fragment_inline_input);
            return ShaderResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<ShaderIndex>(), l_dependencies};
        };
    };

    struct ResourceIncrementInlineFunction
    {
        ShaderModuleResourceUnit& shadermodule_unit;
        const ShaderModuleResource::InlineAllocationInput& vertex_inline_input;
        const ShaderModuleResource::InlineAllocationInput& fragment_inline_input;

        inline ShaderResource operator()(const hash_t p_id) const
        {
            ShaderResource::Dependencies l_dependencies;
            l_dependencies.vertex_shader = ShaderModuleResourceComposition::allocate_or_increment_inline(this->shadermodule_unit, this->vertex_inline_input);
            l_dependencies.fragment_shader = ShaderModuleResourceComposition::allocate_or_increment_inline(this->shadermodule_unit, this->fragment_inline_input);
            return ShaderResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<ShaderIndex>(), l_dependencies};
        };
    };

    struct ResourceDecrementFunction
    {
        ShaderModuleResourceUnit& shadermodule_unit;
        inline void operator()(const ShaderResource& p_resource) const
        {
            ShaderModuleResourceComposition::decrement_or_release(this->shadermodule_unit, p_resource.dependencies.vertex_shader);
            ShaderModuleResourceComposition::decrement_or_release(this->shadermodule_unit, p_resource.dependencies.fragment_shader);
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
        return iResourceUnit<ShaderResourceUnit>{p_unit}.allocate_or_increment_inline(
            p_inline_input.id, p_inline_input.asset, ShaderResourceUnit::ResourceIncrementInlineFunction{p_shader_module_unit, p_vertex_shader_input, p_fragment_shader_input});
    };

    inline static Token<ShaderResource> allocate_or_increment_database(ShaderResourceUnit& p_unit, ShaderModuleResourceUnit& p_shader_module_unit,
                                                                       const ShaderResource::DatabaseAllocationInput& p_inline_input,
                                                                       const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                       const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        return iResourceUnit<ShaderResourceUnit>{p_unit}.allocate_or_increment_database(
            p_inline_input.id, ShaderResourceUnit::ResourceIncrementDatabaseFunction{p_shader_module_unit, p_vertex_shader_input, p_fragment_shader_input});
    };

    inline static void decrement_or_release(ShaderResourceUnit& p_unit, ShaderModuleResourceUnit& p_shader_module_unit, const Token<ShaderResource> p_shader_resource)
    {
        iResourceUnit<ShaderResourceUnit>{p_unit}.decrement_or_release_resource_by_token(p_shader_resource, ShaderResourceUnit::ResourceDecrementFunction{p_shader_module_unit});
    };
};

struct MaterialResourceUnit
{
    PoolHashedCounted<hash_t, MaterialResource> materials;
    PoolOfVector<MaterialResource::DynamicDependency> material_dynamic_dependencies;
    Vector<MaterialResource::InlineAllocationEvent> materials_inline_allocation_events;
    Vector<MaterialResource::DatabaseAllocationEvent> materials_database_allocation_events;
    Vector<MaterialResource::FreeEvent> materials_free_events;

    RESOURCEUNIT_DECLARE_TYPES(MaterialResource);

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

    inline Vector<MaterialResource::DatabaseAllocationEvent>& get_database_allocation_events()
    {
        return this->materials_database_allocation_events;
    };

    inline Vector<MaterialResource::InlineAllocationEvent>& get_inline_allocation_events()
    {
        return this->materials_inline_allocation_events;
    };

    inline Vector<MaterialResource::FreeEvent>& get_free_events()
    {
        return this->materials_free_events;
    };

    inline PoolHashedCounted<hash_t, MaterialResource>& get_pool()
    {
        return this->materials;
    };

    struct ResourceAllocationFunc
    {
        TextureResourceUnit& texture_unit;
        ShaderResourceUnit& shader_resource_unit;
        D3Renderer& renderer;
        GPUContext& gpu_context;

        inline void operator()(MaterialResource& p_resource, const MaterialResource::Asset::Value& p_value) const
        {
            ShaderResource& l_shader = this->shader_resource_unit.shaders.pool.get(p_resource.dependencies.shader);
            ShaderIndex& l_shader_index = this->renderer.allocator.heap.shaders.get(l_shader.shader);

            Material l_material_value = Material::allocate_empty(this->gpu_context.graphics_allocator, (uint32)ColorStep_const::shaderlayout_before.Size());

            for (loop(j, 0, p_value.parameters.parameters.get_size()))
            {
                switch (p_value.parameters.get_parameter_type(j))
                {
                case ShaderParameter::Type::UNIFORM_HOST:
                {
                    const Slice<int8> l_element = p_value.parameters.get_parameter_uniform_host_value(j);
                    l_material_value.add_and_allocate_buffer_host_parameter(this->gpu_context.graphics_allocator, this->gpu_context.buffer_memory.allocator,
                                                                            this->gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), l_element);
                }
                break;
                case ShaderParameter::Type::TEXTURE_GPU:
                {
                    const hash_t* l_texture_id = p_value.parameters.get_parameter_texture_gpu_value(j);
                    Token<TextureGPU> l_texture = this->texture_unit.textures.get_nothashed(*l_texture_id).resource;
                    l_material_value.add_texture_gpu_parameter(this->gpu_context.graphics_allocator, this->gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout),
                                                               l_texture, this->gpu_context.graphics_allocator.heap.textures_gpu.get(l_texture));
                }
                break;
                default:
                    abort();
                }
            };

            p_resource.material = this->renderer.allocator.allocate_material(l_material_value);
            this->renderer.allocator.heap.link_shader_with_material(l_shader.shader, p_resource.material);
        };
    };

    struct ResourceFreeFunc
    {
        ShaderResourceUnit& shader_unit;
        D3Renderer& renderer;
        GPUContext& gpu_context;
        inline void operator()(const MaterialResource& p_resource) const
        {
            ShaderResource& l_linked_shader = this->shader_unit.shaders.pool.get(p_resource.dependencies.shader);
            this->renderer.allocator.heap.unlink_shader_with_material(l_linked_shader.shader, p_resource.material);
            D3RendererAllocatorComposition::free_material_with_parameters(this->gpu_context.buffer_memory, this->gpu_context.graphics_allocator, this->renderer.allocator, p_resource.material);
        };
    };

    template <class EventDependencyAllocator> struct GenericResourceBuilder
    {
        MaterialResourceUnit& p_unit;
        const EventDependencyAllocator& p_event_dependency_allocator;

        inline MaterialResource::Dependencies operator()() const
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

            return l_dependencies;
        };
    };

    struct DatabaseDependencyIncrement
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

    struct ResourceLoadIncrementDatabaseFunction
    {
        MaterialResourceUnit& p_unit;
        ShaderResourceUnit& p_shader_unit;
        ShaderModuleResourceUnit& p_shader_module_unit;
        TextureResourceUnit& p_texture_unit;
        DatabaseConnection& p_database_connection;
        AssetDatabase& p_asset_database;

        inline MaterialResource operator()(const hash_t p_id) const
        {
            Span<int8> l_asset_dependencies_span = p_asset_database.get_asset_dependencies_blob(p_database_connection, p_id);
            MaterialResource::AssetDependencies l_asset_dependencies_asset = MaterialResource::AssetDependencies::build_from_binary(l_asset_dependencies_span);
            MaterialResource::AssetDependencies::Value l_asset_dependencies = MaterialResource::AssetDependencies::Value::build_from_asset(l_asset_dependencies_asset);

            MaterialResource::Dependencies l_resource_dependencies = GenericResourceBuilder<DatabaseDependencyIncrement>{
                p_unit, DatabaseDependencyIncrement{
                            p_shader_unit, p_shader_module_unit, p_texture_unit,
                            MaterialResource::DatabaseAllocationInput{p_id, *(Slice<TextureResource::DatabaseAllocationInput>*)&l_asset_dependencies.textures},
                            ShaderResource::DatabaseAllocationInput{l_asset_dependencies.shader}, ShaderModuleResource::DatabaseAllocationInput{l_asset_dependencies.shader_dependencies.vertex_module},
                            ShaderModuleResource::DatabaseAllocationInput{
                                l_asset_dependencies.shader_dependencies
                                    .fragment_module}}}.operator()();

            l_asset_dependencies_span.free();
            return MaterialResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<Material>(), l_resource_dependencies};
        };
    };

    struct ResourceIncrementDatabaseFunction
    {
        MaterialResourceUnit& p_unit;
        ShaderResourceUnit& p_shader_unit;
        ShaderModuleResourceUnit& p_shader_module_unit;
        TextureResourceUnit& p_texture_unit;
        const MaterialResource::DatabaseAllocationInput& p_inline_input;
        const ShaderResource::DatabaseAllocationInput& p_shader_input;
        const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input;
        const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input;

        inline MaterialResource operator()(const hash_t p_id) const
        {
            MaterialResource::Dependencies l_resource_dependencies = GenericResourceBuilder<DatabaseDependencyIncrement>{
                p_unit, DatabaseDependencyIncrement{
                            p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input,
                            p_fragment_shader_input}}.operator()();
            return MaterialResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<Material>(), l_resource_dependencies};
        };
    };

    struct InlineDependencyIncrement
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

    struct ResourceIncrementInlineFunction
    {
        MaterialResourceUnit& p_unit;
        const MaterialResource::InlineAllocationInput& p_inline_input;
        const InlineDependencyIncrement& p_event_dependency_allocator;

        inline MaterialResource operator()(const hash_t p_id) const
        {
            return MaterialResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<Material>(),
                                    GenericResourceBuilder<InlineDependencyIncrement>{p_unit, p_event_dependency_allocator}.operator()()};
        };
    };

    struct ResourceDecrementFunction
    {
        MaterialResourceUnit& p_unit;
        ShaderResourceUnit& p_shader_unit;
        ShaderModuleResourceUnit& p_shader_module_unit;
        TextureResourceUnit& p_texture_unit;

        inline void operator()(const MaterialResource& p_resource) const
        {
            ShaderResourceComposition::decrement_or_release(p_shader_unit, p_shader_module_unit, p_resource.dependencies.shader);
            Slice<MaterialResource::DynamicDependency> l_material_dynamic_dependencies = p_unit.material_dynamic_dependencies.get_vector(p_resource.dependencies.dynamic_dependencies);
            for (loop(i, 0, l_material_dynamic_dependencies.Size))
            {
                TextureResourceComposition::decrement_or_release(p_texture_unit, l_material_dynamic_dependencies.get(i).dependency);
            }
            p_unit.material_dynamic_dependencies.release_vector(p_resource.dependencies.dynamic_dependencies);
        };
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

        return iResourceUnit<MaterialResourceUnit>{p_unit}.allocate_or_increment_inline(
            p_inline_input.id, p_inline_input.asset,
            MaterialResourceUnit::ResourceIncrementInlineFunction{p_unit, p_inline_input,
                                                                  MaterialResourceUnit::InlineDependencyIncrement{p_shader_unit, p_shader_module_unit, p_texture_unit,
                                                                                                                  p_inline_input.texture_dependencies_input, p_shader_input, p_vertex_shader_input,
                                                                                                                  p_fragment_shader_input}});
    };

    inline static Token<MaterialResource> allocate_or_increment_database(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit, ShaderModuleResourceUnit& p_shader_module_unit,
                                                                         TextureResourceUnit& p_texture_unit, const MaterialResource::DatabaseAllocationInput& p_inline_input,
                                                                         const ShaderResource::DatabaseAllocationInput& p_shader_input,
                                                                         const ShaderModuleResource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                         const ShaderModuleResource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        return iResourceUnit<MaterialResourceUnit>{p_unit}.allocate_or_increment_database(
            p_inline_input.id, MaterialResourceUnit::ResourceIncrementDatabaseFunction{p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input,
                                                                                       p_vertex_shader_input, p_fragment_shader_input});
    };

    inline static Token<MaterialResource> allocate_or_increment_database_and_load_dependecies(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit,
                                                                                              ShaderModuleResourceUnit& p_shader_module_unit, TextureResourceUnit& p_texture_unit,
                                                                                              DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database, const hash_t p_id)
    {
        return iResourceUnit<MaterialResourceUnit>{p_unit}.allocate_or_increment_database(
            p_id, MaterialResourceUnit::ResourceLoadIncrementDatabaseFunction{p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_database_connection, p_asset_database});
    };

    inline static void decrement_or_release(MaterialResourceUnit& p_unit, ShaderResourceUnit& p_shader_unit, ShaderModuleResourceUnit& p_shader_module_unit, TextureResourceUnit& p_texture_unit,
                                            const Token<MaterialResource> p_material_resource)
    {

        iResourceUnit<MaterialResourceUnit>{p_unit}.decrement_or_release_resource_by_token(
            p_material_resource, MaterialResourceUnit::ResourceDecrementFunction{p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit});
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
        iResourceUnit<MaterialResourceUnit>{this->material_unit}.deallocation_step(MaterialResourceUnit::ResourceFreeFunc{this->shader_unit, p_renderer, p_gpu_context});
        iResourceUnit<TextureResourceUnit>{this->texture_unit}.deallocation_step(TextureResourceUnit::ResourceFreeFunction{p_gpu_context});
        iResourceUnit<ShaderResourceUnit>{this->shader_unit}.deallocation_step(ShaderResourceUnit::ResourceFreeFunc{p_renderer, p_gpu_context});
        iResourceUnit<MeshResourceUnit>{this->mesh_unit}.deallocation_step(MeshResourceUnit::ResourceFreeFunc{p_renderer, p_gpu_context});
        iResourceUnit<ShaderModuleResourceUnit>{this->shader_module_unit}.deallocation_step(ShaderModuleResourceUnit::ResourceFreeFunction::build(p_gpu_context));
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        iResourceUnit<ShaderModuleResourceUnit>{this->shader_module_unit}.allocation_step(p_database_connection, p_asset_database,
                                                                                          ShaderModuleResourceUnit::ResourceAllocationFunction::build(p_gpu_context));
        iResourceUnit<MeshResourceUnit>{this->mesh_unit}.allocation_step(p_database_connection, p_asset_database, MeshResourceUnit::ResourceAllocationFunc{p_renderer, p_gpu_context});
        iResourceUnit<ShaderResourceUnit>{this->shader_unit}.allocation_step(p_database_connection, p_asset_database,
                                                                             ShaderResourceUnit::ResourceAllocationFunc{this->shader_module_unit, p_renderer, p_gpu_context});
        iResourceUnit<TextureResourceUnit>{this->texture_unit}.allocation_step(p_database_connection, p_asset_database, TextureResourceUnit::ResourceAllocationFunction{p_gpu_context});
        iResourceUnit<MaterialResourceUnit>{this->material_unit}.allocation_step(p_database_connection, p_asset_database,
                                                                                 MaterialResourceUnit::ResourceAllocationFunc{this->texture_unit, this->shader_unit, p_renderer, p_gpu_context});
    };
};
