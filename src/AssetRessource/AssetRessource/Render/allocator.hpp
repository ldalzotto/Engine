#pragma once

struct ShaderModuleRessourceUnit
{
    PoolHashedCounted<hash_t, ShaderModuleRessource> shader_modules;
    Vector<ShaderModuleRessource::InlineAllocationEvent> shader_modules_allocation_events;
    Vector<ShaderModuleRessource::DatabaseAllocationEvent> shader_module_database_allocation_events;
    Vector<ShaderModuleRessource::FreeEvent> shader_modules_free_events;

    inline static ShaderModuleRessourceUnit allocate()
    {
        return ShaderModuleRessourceUnit{PoolHashedCounted<hash_t, ShaderModuleRessource>::allocate_default(), Vector<ShaderModuleRessource::InlineAllocationEvent>::allocate(0),
                                         Vector<ShaderModuleRessource::DatabaseAllocationEvent>::allocate(0), Vector<ShaderModuleRessource::FreeEvent>::allocate(0)};
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
        RessourceAlgorithm::deallocation_step(this->shader_modules, this->shader_modules_free_events, [&](const ShaderModuleRessource& p_ressource) {
            p_gpu_context.graphics_allocator.free_shader_module(p_ressource.shader_module);
        });
    };

    inline void allocation_step(GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        RessourceAlgorithm::allocation_step(this->shader_modules, this->shader_module_database_allocation_events, this->shader_modules_allocation_events, p_database_connection, p_asset_database,
                                            [&](ShaderModuleRessource& p_ressource, const ShaderModuleRessource::Asset::Value& p_value) {
                                                p_ressource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(p_value.compiled_shader);
                                            });
    };
};

struct ShaderModuleRessourceComposition
{
    inline static Token<ShaderModuleRessource> allocate_or_increment_inline(ShaderModuleRessourceUnit& p_unit, const ShaderModuleRessource::InlineAllocationInput& p_inline_input)
    {
        return RessourceAlgorithm::allocate_or_increment_inline(p_unit.shader_modules, p_unit.shader_modules_allocation_events, p_inline_input.id, p_inline_input.asset,
                                                                RessourceAlgorithm::ressourceobject_builder_default<ShaderModuleRessource>);
    };

    inline static Token<ShaderModuleRessource> allocate_or_increment_database(ShaderModuleRessourceUnit& p_unit, const ShaderModuleRessource::DatabaseAllocationInput& p_inline_input)
    {
        return RessourceAlgorithm::allocate_or_increment_database(p_unit.shader_modules, p_unit.shader_module_database_allocation_events, p_inline_input.id,
                                                                  RessourceAlgorithm::ressourceobject_builder_default<ShaderModuleRessource>);
    };

    inline static void decrement_or_release(ShaderModuleRessourceUnit& p_unit, const Token<ShaderModuleRessource> p_ressource)
    {
        RessourceAlgorithm::decrement_or_release_ressource_by_token_v3(p_unit.shader_modules, p_unit.shader_modules_free_events, p_unit.shader_modules_allocation_events,
                                                                       p_unit.shader_module_database_allocation_events, p_ressource, [](auto) {
                                                                       });
    };
};

struct TextureRessourceUnit
{
    PoolHashedCounted<hash_t, TextureRessource> textures;
    Vector<TextureRessource::InlineAllocationEvent> textures_allocation_events;
    Vector<TextureRessource::DatabaseAllocationEvent> texture_database_allocation_events;
    Vector<TextureRessource::FreeEvent> textures_free_events;

    inline static TextureRessourceUnit allocate()
    {
        return TextureRessourceUnit{PoolHashedCounted<hash_t, TextureRessource>::allocate_default(), Vector<TextureRessource::InlineAllocationEvent>::allocate(0),
                                    Vector<TextureRessource::DatabaseAllocationEvent>::allocate(0), Vector<TextureRessource::FreeEvent>::allocate(0)};
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
        RessourceAlgorithm::deallocation_step(this->textures, this->textures_free_events, [&](const TextureRessource& p_ressource) {
            GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_ressource.texture);
        });
    };

    inline void allocation_step(GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        RessourceAlgorithm::allocation_step(this->textures, this->texture_database_allocation_events, this->textures_allocation_events, p_database_connection, p_asset_database,
                                            TextureRessourceAllocation{p_gpu_context});
    };

    struct TextureRessourceAllocation
    {
        GPUContext& gpu_context;

        inline void operator()(TextureRessource& p_ressource, const TextureRessource::Asset::Value& p_value) const
        {
            p_ressource.texture = ShaderParameterBufferAllocationFunctions::allocate_texture_gpu_for_shaderparameter(gpu_context.graphics_allocator, gpu_context.buffer_memory,
                                                                                                                     ImageFormat::build_color_2d(p_value.size, ImageUsageFlag::UNDEFINED));
            TextureGPU& l_texture_gpu = gpu_context.graphics_allocator.heap.textures_gpu.get(p_ressource.texture);
            BufferReadWrite::write_to_imagegpu(gpu_context.buffer_memory.allocator, gpu_context.buffer_memory.events, l_texture_gpu.Image,
                                               gpu_context.buffer_memory.allocator.gpu_images.get(l_texture_gpu.Image), p_value.pixels);
        }
    };
};

struct TextureRessourceComposition
{
    inline static Token<TextureRessource> allocate_or_increment_inline(TextureRessourceUnit& p_unit, const TextureRessource::InlineAllocationInput& p_inline_input)
    {
        return RessourceAlgorithm::allocate_or_increment_inline(p_unit.textures, p_unit.textures_allocation_events, p_inline_input.id, p_inline_input.asset,
                                                                RessourceAlgorithm::ressourceobject_builder_default<TextureRessource>);
    };

    inline static Token<TextureRessource> allocate_or_increment_database(TextureRessourceUnit& p_unit, const TextureRessource::DatabaseAllocationInput& p_inline_input)
    {
        return RessourceAlgorithm::allocate_or_increment_database(p_unit.textures, p_unit.texture_database_allocation_events, p_inline_input.id,
                                                                  RessourceAlgorithm::ressourceobject_builder_default<TextureRessource>);
    };

    inline static void decrement_or_release(TextureRessourceUnit& p_unit, const Token<TextureRessource> p_ressource)
    {
        RessourceAlgorithm::decrement_or_release_ressource_by_token_v3(p_unit.textures, p_unit.textures_free_events, p_unit.textures_allocation_events, p_unit.texture_database_allocation_events,
                                                                       p_ressource, [](auto) {
                                                                       });
    };
};

struct MeshRessourceUnit
{
    PoolHashedCounted<hash_t, MeshRessource> meshes;
    Vector<MeshRessource::InlineAllocationEvent> meshes_allocation_events;
    Vector<MeshRessource::DatabaseAllocationEvent> meshes_database_allocation_events;
    Vector<MeshRessource::FreeEvent> meshes_free_events;

    inline static MeshRessourceUnit allocate()
    {
        return MeshRessourceUnit{PoolHashedCounted<hash_t, MeshRessource>::allocate_default(), Vector<MeshRessource::InlineAllocationEvent>::allocate(0),
                                 Vector<MeshRessource::DatabaseAllocationEvent>::allocate(0), Vector<MeshRessource::FreeEvent>::allocate(0)};
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
        RessourceAlgorithm::deallocation_step(this->meshes, this->meshes_free_events, [&](const MeshRessource& p_ressource) {
            D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, p_ressource.mesh);
        });
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        RessourceAlgorithm::allocation_step(this->meshes, this->meshes_database_allocation_events, this->meshes_allocation_events, p_database_connection, p_asset_database,
                                            [&](MeshRessource& p_mesh_ressource, const MeshRessource::Asset::Value& p_value) {
                                                p_mesh_ressource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator,
                                                                                                                                   p_value.initial_vertices, p_value.initial_indices);
                                            });
    };
};

struct MeshRessourceComposition
{
    inline static Token<MeshRessource> allocate_or_increment_inline(MeshRessourceUnit& p_unit, const MeshRessource::InlineAllocationInput& p_inline_input)
    {
        return RessourceAlgorithm::allocate_or_increment_inline(p_unit.meshes, p_unit.meshes_allocation_events, p_inline_input.id, p_inline_input.asset,
                                                                RessourceAlgorithm::ressourceobject_builder_default<MeshRessource>);
    };

    inline static Token<MeshRessource> allocate_or_increment_database(MeshRessourceUnit& p_unit, const MeshRessource::DatabaseAllocationInput& p_inline_input)
    {
        return RessourceAlgorithm::allocate_or_increment_database(p_unit.meshes, p_unit.meshes_database_allocation_events, p_inline_input.id,
                                                                  RessourceAlgorithm::ressourceobject_builder_default<MeshRessource>);
    };

    inline static void decrement_or_release(MeshRessourceUnit& p_unit, const Token<MeshRessource> p_mesh_ressource)
    {
        RessourceAlgorithm::decrement_or_release_ressource_by_token_v3(p_unit.meshes, p_unit.meshes_free_events, p_unit.meshes_allocation_events, p_unit.meshes_database_allocation_events,
                                                                       p_mesh_ressource, [](auto) {
                                                                       });
    };
};

// TODO -> ressource depedencies are stored inside the ressource itself. Wouldn't it be better to store dependencies in a separated Pool ?
struct ShaderRessourceUnit
{
    PoolHashedCounted<hash_t, ShaderRessource> shaders;
    Vector<ShaderRessource::InlineAllocationEvent> shaders_allocation_events;
    Vector<ShaderRessource::DatabaseAllocationEvent> shaders_database_allocation_events;
    Vector<ShaderRessource::FreeEvent> shaders_free_events;

    inline static ShaderRessourceUnit allocate()
    {
        return ShaderRessourceUnit{PoolHashedCounted<hash_t, ShaderRessource>::allocate_default(), Vector<ShaderRessource::InlineAllocationEvent>::allocate(0),
                                   Vector<ShaderRessource::DatabaseAllocationEvent>::allocate(0), Vector<ShaderRessource::FreeEvent>::allocate(0)};
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
        RessourceAlgorithm::deallocation_step(this->shaders, this->shaders_free_events, [&](const ShaderRessource& p_ressource) {
            D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, p_ressource.shader);
        });
    };

    inline void allocation_step(ShaderModuleRessourceUnit& p_shader_module_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection,
                                AssetDatabase& p_asset_database)
    {
        RessourceAlgorithm::allocation_step(this->shaders, this->shaders_database_allocation_events, this->shaders_allocation_events, p_database_connection, p_asset_database,
                                            ShaderRessourceAllocation{p_shader_module_unit, p_renderer, p_gpu_context});
    };

    struct ShaderRessourceAllocation
    {
        ShaderModuleRessourceUnit& shader_module_unit;
        D3Renderer& renderer;
        GPUContext& gpu_context;
        inline void operator()(ShaderRessource& p_mesh_ressource, const ShaderRessource::Asset::Value& p_value) const
        {
            ShaderModuleRessource& l_vertex_shader = shader_module_unit.shader_modules.pool.get(p_mesh_ressource.dependencies.vertex_shader);
            ShaderModuleRessource& l_fragment_shader = shader_module_unit.shader_modules.pool.get(p_mesh_ressource.dependencies.fragment_shader);
            p_mesh_ressource.shader = D3RendererAllocatorComposition::allocate_colorstep_shader_with_shaderlayout(
                gpu_context.graphics_allocator, renderer.allocator, p_value.specific_parameters, p_value.execution_order,
                gpu_context.graphics_allocator.heap.graphics_pass.get(renderer.color_step.pass), p_value.shader_configuration,
                gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.shader_module), gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.shader_module));
        };
    };
};

struct ShaderRessourceComposition
{
    inline static Token<ShaderRessource> allocate_or_increment_inline(ShaderRessourceUnit& p_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                                      const ShaderRessource::InlineAllocationInput& p_inline_input,
                                                                      const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader_input,
                                                                      const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader_input)
    {
        return RessourceAlgorithm::allocate_or_increment_inline(
            p_unit.shaders, p_unit.shaders_allocation_events, p_inline_input.id, p_inline_input.asset, [&](const RessourceIdentifiedHeader& p_ressource_header) {
                ShaderRessource::Dependencies l_dependencies;
                l_dependencies.vertex_shader = ShaderModuleRessourceComposition::allocate_or_increment_inline(p_shader_module_unit, p_vertex_shader_input);
                l_dependencies.fragment_shader = ShaderModuleRessourceComposition::allocate_or_increment_inline(p_shader_module_unit, p_fragment_shader_input);
                return ShaderRessource{p_ressource_header, token_build_default<ShaderIndex>(), l_dependencies};
            });
    };

    inline static Token<ShaderRessource> allocate_or_increment_database(ShaderRessourceUnit& p_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                                        const ShaderRessource::DatabaseAllocationInput& p_inline_input,
                                                                        const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                        const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        return RessourceAlgorithm::allocate_or_increment_database(
            p_unit.shaders, p_unit.shaders_database_allocation_events, p_inline_input.id, [&](const RessourceIdentifiedHeader& p_ressource_header) {
                ShaderRessource::Dependencies l_dependencies;
                l_dependencies.vertex_shader = ShaderModuleRessourceComposition::allocate_or_increment_database(p_shader_module_unit, p_vertex_shader_input);
                l_dependencies.fragment_shader = ShaderModuleRessourceComposition::allocate_or_increment_database(p_shader_module_unit, p_fragment_shader_input);
                return ShaderRessource{p_ressource_header, token_build_default<ShaderIndex>(), l_dependencies};
            });
    };

    inline static void decrement_or_release(ShaderRessourceUnit& p_unit, ShaderModuleRessourceUnit& p_shader_module_unit, const Token<ShaderRessource> p_shader_ressource)
    {
        RessourceAlgorithm::decrement_or_release_ressource_by_token_v3(p_unit.shaders, p_unit.shaders_free_events, p_unit.shaders_allocation_events, p_unit.shaders_database_allocation_events,
                                                                       p_shader_ressource, [&](const ShaderRessource& p_ressource) {
                                                                           ShaderModuleRessourceComposition::decrement_or_release(p_shader_module_unit, p_ressource.dependencies.vertex_shader);
                                                                           ShaderModuleRessourceComposition::decrement_or_release(p_shader_module_unit, p_ressource.dependencies.fragment_shader);
                                                                       });
    };
};

// TODO -> ressource depedencies are stored inside the ressource itself. Wouldn't it be better to store dependencies in a separated Pool ?
struct MaterialRessourceUnit
{
    PoolHashedCounted<hash_t, MaterialRessource> materials;
    PoolOfVector<MaterialRessource::DynamicDependency> material_dynamic_dependencies;
    Vector<MaterialRessource::InlineAllocationEvent> materials_allocation_events;
    Vector<MaterialRessource::DatabaseAllocationEvent> materials_database_allocation_events;
    Vector<MaterialRessource::FreeEvent> materials_free_events;

    inline static MaterialRessourceUnit allocate()
    {
        return MaterialRessourceUnit{PoolHashedCounted<hash_t, MaterialRessource>::allocate_default(), PoolOfVector<MaterialRessource::DynamicDependency>::allocate_default(),
                                     Vector<MaterialRessource::InlineAllocationEvent>::allocate(0), Vector<MaterialRessource::DatabaseAllocationEvent>::allocate(0),
                                     Vector<MaterialRessource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->materials_allocation_events.empty());
        assert_true(this->materials_database_allocation_events.empty());
        assert_true(this->materials_free_events.empty());
        assert_true(this->materials.empty());
        assert_true(!this->material_dynamic_dependencies.has_allocated_elements());
#endif
        this->materials_free_events.free();
        this->materials_allocation_events.free();
        this->materials_database_allocation_events.free();
        this->materials.free();
        this->material_dynamic_dependencies.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->materials_allocation_events.empty());
        assert_true(this->materials_database_allocation_events.empty());
#endif
    };

    inline void deallocation_step(ShaderRessourceUnit& p_shader_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        RessourceAlgorithm::deallocation_step(this->materials, this->materials_free_events, [&](const MaterialRessource& p_ressource) {
            ShaderRessource& l_linked_shader = p_shader_unit.shaders.pool.get(p_ressource.dependencies.shader);
            p_renderer.allocator.heap.unlink_shader_with_material(l_linked_shader.shader, p_ressource.material);
            D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, p_ressource.material);
        });
    };

    inline void allocation_step(ShaderRessourceUnit& p_shader_unit, TextureRessourceUnit& p_texture_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection,
                                AssetDatabase& p_asset_database)
    {
        RessourceAlgorithm::allocation_step(
            this->materials, this->materials_database_allocation_events, this->materials_allocation_events, p_database_connection, p_asset_database,
            [&](MaterialRessource& p_ressource, const MaterialRessource::Asset::Value& p_value) {
                ShaderRessource& l_shader = p_shader_unit.shaders.pool.get(p_ressource.dependencies.shader);
                ShaderIndex& l_shader_index = p_renderer.allocator.heap.shaders.get(l_shader.shader);

                Material l_material_value = Material::allocate_empty(p_gpu_context.graphics_allocator, 1);

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

                p_ressource.material = p_renderer.allocator.allocate_material(l_material_value);
                p_renderer.allocator.heap.link_shader_with_material(l_shader.shader, p_ressource.material);
            });
    };
};

struct MaterialRessourceComposition
{
    inline static Token<MaterialRessource> allocate_or_increment_inline(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                                        TextureRessourceUnit& p_texture_unit, const MaterialRessource::InlineAllocationInput& p_inline_input,
                                                                        const ShaderRessource::InlineAllocationInput& p_shader_input,
                                                                        const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader_input,
                                                                        const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader_input)
    {
        return RessourceAlgorithm::allocate_or_increment_inline(p_unit.materials, p_unit.materials_allocation_events, p_inline_input.id, p_inline_input.asset,
                                                                [&](const RessourceIdentifiedHeader& p_header) {
                                                                    return materialressource_inline_allocate_recursively(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input,
                                                                                                                         p_shader_input, p_vertex_shader_input, p_fragment_shader_input, p_header);
                                                                });
    };

    inline static Token<MaterialRessource> allocate_database(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                             TextureRessourceUnit& p_texture_unit, const MaterialRessource::DatabaseAllocationInput& p_inline_input,
                                                             const ShaderRessource::DatabaseAllocationInput& p_shader_input,
                                                             const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input,
                                                             const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input)
    {

        return RessourceAlgorithm::push_ressource_to_be_allocated_database(
            p_unit.materials, p_unit.materials_database_allocation_events, p_inline_input.id, [&](const RessourceIdentifiedHeader& p_header) {
                return materialressource_database_allocate_recursively(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input,
                                                                       p_fragment_shader_input, p_header);
            });
    };

    inline static Token<MaterialRessource> allocate_or_increment_database(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                                          TextureRessourceUnit& p_texture_unit, const MaterialRessource::DatabaseAllocationInput& p_inline_input,
                                                                          const ShaderRessource::DatabaseAllocationInput& p_shader_input,
                                                                          const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                          const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        return RessourceAlgorithm::allocate_or_increment_database(p_unit.materials, p_unit.materials_database_allocation_events, p_inline_input.id, [&](const RessourceIdentifiedHeader& p_header) {
            return materialressource_database_allocate_recursively(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input,
                                                                   p_fragment_shader_input, p_header);
        });
    };

    inline static Token<MaterialRessource> allocate_or_increment_database_and_load_dependecies(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit,
                                                                                               ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                                                                                               DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database, const hash_t p_id)
    {
        return RessourceAlgorithm::allocate_or_increment_database(p_unit.materials, p_unit.materials_database_allocation_events, p_id, [&](const RessourceIdentifiedHeader& p_header) {
            MaterialRessource l_ressource;
            RessourceAlgorithm::with_asset_dependencies<MaterialRessource>(
                p_database_connection, p_asset_database, p_id, [&](const MaterialRessource::AssetDependencies::Value& p_asset_dependencies_value) {
                    MaterialRessource::DatabaseAllocationInput l_material_database_input;
                    l_material_database_input.id = p_id;
                    l_material_database_input.texture_dependencies_input = *(Slice<TextureRessource::DatabaseAllocationInput>*)&p_asset_dependencies_value.textures;

                    l_ressource = materialressource_database_allocate_recursively(
                        p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, l_material_database_input, ShaderRessource::DatabaseAllocationInput{p_asset_dependencies_value.shader},
                        ShaderModuleRessource::DatabaseAllocationInput{p_asset_dependencies_value.shader_dependencies.vertex_module},
                        ShaderModuleRessource::DatabaseAllocationInput{p_asset_dependencies_value.shader_dependencies.fragment_module}, p_header);
                });
            return l_ressource;
        });
    };

    inline static void decrement_or_release(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                                            const Token<MaterialRessource> p_material_ressource)
    {
        RessourceAlgorithm::decrement_or_release_ressource_by_token_v3(p_unit.materials, p_unit.materials_free_events, p_unit.materials_allocation_events, p_unit.materials_database_allocation_events,
                                                                       p_material_ressource, [&](const MaterialRessource& p_ressource) {
                                                                           materialressource_release_recursively(p_ressource, p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit);
                                                                       });
    };

  private:
    inline static MaterialRessource materialressource_database_allocate_recursively(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                                                    TextureRessourceUnit& p_texture_unit, const MaterialRessource::DatabaseAllocationInput& p_inline_input,
                                                                                    const ShaderRessource::DatabaseAllocationInput& p_shader_input,
                                                                                    const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input,
                                                                                    const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input,
                                                                                    const RessourceIdentifiedHeader& p_header)
    {
        return materialressource_T_allocate_recursively(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input, p_fragment_shader_input,
                                                        p_header, ShaderRessourceComposition::allocate_or_increment_database, TextureRessourceComposition::allocate_or_increment_database);
    };

    inline static MaterialRessource materialressource_inline_allocate_recursively(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                                                  TextureRessourceUnit& p_texture_unit, const MaterialRessource::InlineAllocationInput& p_inline_input,
                                                                                  const ShaderRessource::InlineAllocationInput& p_shader_input,
                                                                                  const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader_input,
                                                                                  const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader_input,
                                                                                  const RessourceIdentifiedHeader& p_header)
    {
        return materialressource_T_allocate_recursively(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input, p_fragment_shader_input,
                                                        p_header, ShaderRessourceComposition::allocate_or_increment_inline, TextureRessourceComposition::allocate_or_increment_inline);
    };

    template <class MaterialRessourceAllocationInput, class ShaderRessourceAllocationInputType, class ShaderModuleRessourceAllocationInput, class ShaderRessourceAllocateOrIncrementFunc,
              class TextureRessourceAllocateofIncrementFunc>
    inline static MaterialRessource
    materialressource_T_allocate_recursively(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                                             const MaterialRessourceAllocationInput& p_material_ressource_allocation_input, const ShaderRessourceAllocationInputType& p_shader_input,
                                             const ShaderModuleRessourceAllocationInput& p_vertex_shader_input, const ShaderModuleRessourceAllocationInput& p_fragment_shader_input,
                                             const RessourceIdentifiedHeader& p_header, const ShaderRessourceAllocateOrIncrementFunc& p_shader_ressource_allocate_or_increment_func,
                                             const TextureRessourceAllocateofIncrementFunc& p_texture_ressource_allocate_or_increment_func)
    {
        MaterialRessource::Dependencies l_dependencies;
        l_dependencies.shader = p_shader_ressource_allocate_or_increment_func(p_shader_unit, p_shader_module_unit, p_shader_input, p_vertex_shader_input, p_fragment_shader_input);
        Span<MaterialRessource::DynamicDependency> l_dynamic_textures = Span<MaterialRessource::DynamicDependency>::allocate(p_material_ressource_allocation_input.texture_dependencies_input.Size);
        for (loop(i, 0, l_dynamic_textures.Capacity))
        {
            l_dynamic_textures.get(i) =
                MaterialRessource::DynamicDependency{p_texture_ressource_allocate_or_increment_func(p_texture_unit, p_material_ressource_allocation_input.texture_dependencies_input.get(i))};
        }
        l_dependencies.dynamic_dependencies = p_unit.material_dynamic_dependencies.alloc_vector_with_values(l_dynamic_textures.slice);
        l_dynamic_textures.free();
        return MaterialRessource{p_header, token_build_default<Material>(), l_dependencies};
    };

    inline static void materialressource_release_recursively(const MaterialRessource& p_ressource, MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit,
                                                             ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit)
    {
        ShaderRessourceComposition::decrement_or_release(p_shader_unit, p_shader_module_unit, p_ressource.dependencies.shader);
        Slice<MaterialRessource::DynamicDependency> l_material_dynamic_dependencies = p_unit.material_dynamic_dependencies.get_vector(p_ressource.dependencies.dynamic_dependencies);
        for (loop(i, 0, l_material_dynamic_dependencies.Size))
        {
            TextureRessourceComposition::decrement_or_release(p_texture_unit, l_material_dynamic_dependencies.get(i).dependency);
        }
        p_unit.material_dynamic_dependencies.release_vector(p_ressource.dependencies.dynamic_dependencies);
    };
};

/*
    The RenderRessourceAllocator2 is resposible of allocating "AssetRessources" for the Render system.
    AssetRessources are internal data that can either be retrieved from the AssetDatabase or by providing an inline blob.
    The goal of this layer is to handle AssetDatabase connection by identifying ressource by associating an asset_id to the allocate token.
    For exemple, this allow to reuse the same blob across multiple internal token.
    By default, AssetRessources are read-only.
*/
struct RenderRessourceAllocator2
{
    ShaderModuleRessourceUnit shader_module_unit;
    TextureRessourceUnit texture_unit;
    MeshRessourceUnit mesh_unit;
    ShaderRessourceUnit shader_unit;
    MaterialRessourceUnit material_unit;

    inline static RenderRessourceAllocator2 allocate()
    {
        return RenderRessourceAllocator2{ShaderModuleRessourceUnit::allocate(), TextureRessourceUnit::allocate(), MeshRessourceUnit::allocate(), ShaderRessourceUnit::allocate(),
                                         MaterialRessourceUnit::allocate()};
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
