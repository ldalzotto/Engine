#pragma once

namespace v2
{
struct RenderRessourceHeap
{
    PoolHashedCounted<hash_t, ShaderModuleRessource> shader_modules_v2;
    PoolHashedCounted<hash_t, MeshRessource> mesh_v2;
    PoolHashedCounted<hash_t, ShaderRessource> shaders_v3;
    PoolHashedCounted<hash_t, TextureRessource> textures;
    PoolHashedCounted<hash_t, MaterialRessource> materials;
    PoolOfVector<MaterialRessource::DynamicDependency> material_dynamic_dependencies;

    inline static RenderRessourceHeap allocate()
    {
        return RenderRessourceHeap{PoolHashedCounted<hash_t, ShaderModuleRessource>::allocate_default(), PoolHashedCounted<hash_t, MeshRessource>::allocate_default(),
                                   PoolHashedCounted<hash_t, ShaderRessource>::allocate_default(),       PoolHashedCounted<hash_t, TextureRessource>::allocate_default(),
                                   PoolHashedCounted<hash_t, MaterialRessource>::allocate_default(),     PoolOfVector<MaterialRessource::DynamicDependency>::allocate_default()};
    };

    inline void free()
    {
#if RENDER_BOUND_TEST
        assert_true(this->shaders_v3.empty());
        assert_true(this->mesh_v2.empty());
        assert_true(this->shaders_v3.empty());
        assert_true(this->textures.empty());
        assert_true(this->materials.empty());
        assert_true(!this->material_dynamic_dependencies.has_allocated_elements());
#endif
        this->shader_modules_v2.free();
        this->mesh_v2.free();
        this->shaders_v3.free();
        this->textures.free();
        this->materials.free();
        this->material_dynamic_dependencies.free();
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
    RenderRessourceHeap heap;

    Vector<ShaderModuleRessource::AllocationEvent> shadermodule_allocation_events;
    Vector<MeshRessource::AllocationEvent> mesh_allocation_events;
    Vector<ShaderRessource::AllocationEvent> shader_allocation_events;
    Vector<TextureRessource::AllocationEvent> texture_allocation_events;
    Vector<MaterialRessource::AllocationEvent> material_allocation_events;

    Vector<ShaderModuleRessource::FreeEvent> shadermodule_free_events;
    Vector<MeshRessource::FreeEvent> mesh_free_events;
    Vector<ShaderRessource::FreeEvent> shader_free_events;
    Vector<TextureRessource::FreeEvent> texture_free_events;
    Vector<MaterialRessource::FreeEvent> material_free_events;

    inline static RenderRessourceAllocator2 allocate()
    {
        return RenderRessourceAllocator2{
            RenderRessourceHeap::allocate(),
            Vector<ShaderModuleRessource::AllocationEvent>::allocate(0),
            Vector<MeshRessource::AllocationEvent>::allocate(0),
            Vector<ShaderRessource::AllocationEvent>::allocate(0),
            Vector<TextureRessource::AllocationEvent>::allocate(0),
            Vector<MaterialRessource::AllocationEvent>::allocate(0),
            Vector<ShaderModuleRessource::FreeEvent>::allocate(0),
            Vector<MeshRessource::FreeEvent>::allocate(0),
            Vector<ShaderRessource::FreeEvent>::allocate(0),
            Vector<TextureRessource::FreeEvent>::allocate(0),
            Vector<MaterialRessource::FreeEvent>::allocate(0),
        };
    };

    inline void free(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
#if RENDER_BOUND_TEST
        assert_true(this->shadermodule_allocation_events.empty());
        assert_true(this->mesh_allocation_events.empty());
        assert_true(this->shader_allocation_events.empty());
        assert_true(this->texture_allocation_events.empty());
        assert_true(this->material_allocation_events.empty());
#endif

        this->deallocation_step(p_renderer, p_gpu_context);

        this->heap.free();

#if RENDER_BOUND_TEST
        assert_true(this->mesh_free_events.empty());
        assert_true(this->shader_free_events.empty());
        assert_true(this->texture_free_events.empty());
        assert_true(this->material_free_events.empty());
#endif

        this->shadermodule_allocation_events.free();
        this->mesh_allocation_events.free();
        this->shader_allocation_events.free();
        this->texture_allocation_events.free();
        this->material_allocation_events.free();
        this->shadermodule_free_events.free();
        this->mesh_free_events.free();
        this->shader_free_events.free();
        this->texture_free_events.free();
        this->material_free_events.free();
    };

    inline void deallocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        for (loop_reverse(i, 0, this->material_free_events.Size))
        {
            auto& l_event = this->material_free_events.get(i);
            MaterialRessource& l_ressource = this->heap.materials.pool.get(l_event.ressource);
            ShaderRessource& l_linked_shader = this->heap.shaders_v3.pool.get(l_ressource.dependencies.shader);
            p_renderer.allocator.heap.unlink_shader_with_material(l_linked_shader.shader, l_ressource.material);
            D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_ressource.material);
            this->heap.materials.pool.release_element(l_event.ressource);
            this->material_free_events.pop_back();
        }

        for (loop_reverse(i, 0, this->texture_free_events.Size))
        {
            auto& l_event = this->texture_free_events.get(i);
            TextureRessource& l_ressource = this->heap.textures.pool.get(l_event.ressource);
            GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_ressource.texture);
            this->heap.textures.pool.release_element(l_event.ressource);
            this->texture_free_events.pop_back();
        }

        for (loop_reverse(i, 0, this->shader_free_events.Size))
        {
            auto& l_event = this->shader_free_events.get(i);
            ShaderRessource& l_ressource = this->heap.shaders_v3.pool.get(l_event.ressource);
            D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, l_ressource.shader);
            this->heap.shaders_v3.pool.release_element(l_event.ressource);
            this->shader_free_events.pop_back();
        }

        for (loop_reverse(i, 0, this->mesh_free_events.Size))
        {
            auto& l_event = this->mesh_free_events.get(i);
            MeshRessource& l_ressource = this->heap.mesh_v2.pool.get(l_event.ressource);
            D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_ressource.mesh);
            this->heap.mesh_v2.pool.release_element(l_event.ressource);
            this->mesh_free_events.pop_back();
        }

        for (loop_reverse(i, 0, this->shadermodule_free_events.Size))
        {
            auto& l_event = this->shadermodule_free_events.get(i);
            ShaderModuleRessource& l_ressource = this->heap.shader_modules_v2.pool.get(l_event.ressource);
            p_gpu_context.graphics_allocator.free_shader_module(l_ressource.shader_module);
            this->heap.shader_modules_v2.pool.release_element(l_event.ressource);
            this->shadermodule_free_events.pop_back();
        }
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->shadermodule_allocation_events.Size))
        {
            auto& l_event = this->shadermodule_allocation_events.get(i);
            ShaderModuleRessource& l_ressource = this->heap.shader_modules_v2.pool.get(l_event.allocated_ressource);
            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_asset_database, l_ressource.header, &l_event.asset);

            ShaderModuleRessource::Asset::Value l_value = ShaderModuleRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_value.compiled_shader);
            l_ressource.header.allocated = 1;
            l_event.asset.free();
            this->shadermodule_allocation_events.pop_back();
        }

        for (loop_reverse(i, 0, this->mesh_allocation_events.Size))
        {
            auto& l_event = this->mesh_allocation_events.get(i);

            MeshRessource& l_ressource = this->heap.mesh_v2.pool.get(l_event.allocated_ressource);
            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_asset_database, l_ressource.header, &l_event.asset);

            MeshRessource::Asset::Value l_value = MeshRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_value.initial_vertices, l_value.initial_indices);
            l_ressource.header.allocated = 1;
            l_event.asset.free();
            this->mesh_allocation_events.pop_back();
        }

        for (loop_reverse(i, 0, this->shader_allocation_events.Size))
        {
            auto& l_event = this->shader_allocation_events.get(i);
            ShaderRessource& l_ressource = this->heap.shaders_v3.pool.get(l_event.allocated_ressource);

            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_asset_database, l_ressource.header, &l_event.asset);

            ShaderModuleRessource& l_vertex_shader = this->heap.shader_modules_v2.pool.get(l_ressource.dependencies.vertex_shader);
            ShaderModuleRessource& l_fragment_shader = this->heap.shader_modules_v2.pool.get(l_ressource.dependencies.fragment_shader);

            ShaderRessource::Asset::Value l_value = ShaderRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.shader = D3RendererAllocatorComposition::allocate_colorstep_shader_with_shaderlayout(
                p_gpu_context.graphics_allocator, p_renderer.allocator, l_value.specific_parameters, l_value.execution_order,
                p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass), l_value.shader_configuration,
                p_gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.shader_module), p_gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.shader_module));
            l_event.asset.free();
            l_ressource.header.allocated = 1;
            this->shader_allocation_events.pop_back();
        }

        for (loop_reverse(i, 0, this->texture_allocation_events.Size))
        {
            auto& l_event = this->texture_allocation_events.get(i);
            TextureRessource& l_ressource = this->heap.textures.pool.get(l_event.allocated_ressource);

            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_asset_database, l_ressource.header, &l_event.asset);

            TextureRessource::Asset::Value l_value = TextureRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.texture = ShaderParameterBufferAllocationFunctions::allocate_texture_gpu_for_shaderparameter(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory,
                                                                                                                     ImageFormat::build_color_2d(l_value.size, ImageUsageFlag::UNDEFINED));
            TextureGPU& l_texture_gpu = p_gpu_context.graphics_allocator.heap.textures_gpu.get(l_ressource.texture);
            BufferReadWrite::write_to_imagegpu(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, l_texture_gpu.Image,
                                               p_gpu_context.buffer_memory.allocator.gpu_images.get(l_texture_gpu.Image), l_value.pixels);

            l_event.asset.free();
            l_ressource.header.allocated = 1;
            this->texture_allocation_events.pop_back();
        }

        for (loop_reverse(i, 0, this->material_allocation_events.Size))
        {
            auto& l_event = this->material_allocation_events.get(i);
            MaterialRessource& l_ressource = this->heap.materials.pool.get(l_event.allocated_ressource);

            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_asset_database, l_ressource.header, &l_event.asset);

            ShaderRessource& l_shader = this->heap.shaders_v3.pool.get(l_ressource.dependencies.shader);
            ShaderIndex& l_shader_index = p_renderer.allocator.heap.shaders.get(l_shader.shader);

            MaterialRessource::Asset::Value l_value = MaterialRessource::Asset::Value::build_from_asset(l_event.asset);
            Material l_material_value = Material::allocate_empty(p_gpu_context.graphics_allocator, 1);

            for (loop(j, 0, l_value.parameters.get_size()))
            {
                Slice<int8> l_element = l_value.parameters.get_element(j);
                switch (*(ShaderParameter::Type*)l_element.Begin)
                {
                case ShaderParameter::Type::UNIFORM_HOST:
                {
                    l_element.slide(sizeof(ShaderParameter::Type));
                    l_material_value.add_and_allocate_buffer_host_parameter(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory.allocator,
                                                                            p_gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), l_element);
                }
                break;
                case ShaderParameter::Type::TEXTURE_GPU:
                {
                    l_element.slide(sizeof(ShaderParameter::Type));
                    hash_t l_texture_id = slice_cast<hash_t>(l_element).get(0);
                    Token(TextureGPU) l_texture = this->heap.textures.pool.get(this->heap.textures.CountMap.get_value_nothashed(l_texture_id)->token).texture;
                    l_material_value.add_texture_gpu_parameter(p_gpu_context.graphics_allocator, p_gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), l_texture,
                                                               p_gpu_context.graphics_allocator.heap.textures_gpu.get(l_texture));
                }
                break;
                default:
                    abort();
                }
            };

            l_ressource.material = p_renderer.allocator.allocate_material(l_material_value);
            p_renderer.allocator.heap.link_shader_with_material(l_shader.shader, l_ressource.material);

            l_event.asset.free();
            l_ressource.header.allocated = 1;
            this->material_allocation_events.pop_back();
        }
    };

    inline Token(ShaderModuleRessource) allocate_shadermodule_inline(const ShaderModuleRessource::InlineAllocationInput& p_shader_module)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(this->heap.shader_modules_v2, this->shadermodule_allocation_events, p_shader_module.id,
                                                                             ShaderModuleRessource::build_inline_from_id, [&p_shader_module](const Token(ShaderModuleRessource) p_allocated_ressource) {
                                                                                 return ShaderModuleRessource::AllocationEvent{p_shader_module.asset, p_allocated_ressource};
                                                                             });
    };

    inline Token(ShaderModuleRessource) allocate_shadermodule_database(const ShaderModuleRessource::DatabaseAllocationInput& p_shader_module)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(this->heap.shader_modules_v2, this->shadermodule_allocation_events, p_shader_module.id,
                                                                             ShaderModuleRessource::build_database_from_id, [](const Token(ShaderModuleRessource) p_allocated_ressource) {
                                                                                 return ShaderModuleRessource::AllocationEvent{ShaderModuleRessource::Asset{}, p_allocated_ressource};
                                                                             });
    };

    inline void free_shadermodule(const ShaderModuleRessource& p_shader_module)
    {
        RessourceComposition::free_ressource_composition_explicit(this->heap.shader_modules_v2, this->shadermodule_allocation_events, this->shadermodule_free_events, p_shader_module.header,
                                                                  ShaderModuleRessource::FreeEvent::build_from_token, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});
    };

    inline Token(ShaderRessource) allocate_shader_v2_inline(const ShaderRessource::InlineAllocationInput& p_shader, const ShaderRessource::Dependencies& p_dependencies)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(
            this->heap.shaders_v3, this->shader_allocation_events, p_shader.id,
            [&p_dependencies](hash_t p_id) {
                return ShaderRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id), tk_bd(ShaderIndex),
                                       ShaderRessource::Dependencies{p_dependencies.vertex_shader, p_dependencies.fragment_shader}};
            },
            [&p_shader](const Token(ShaderRessource) p_allocated_ressource) {
                return ShaderRessource::AllocationEvent{p_shader.asset, p_allocated_ressource};
            });
    };

    inline Token(ShaderRessource) allocate_shader_v2_database(const ShaderRessource::DatabaseAllocationInput& p_shader, const ShaderRessource::Dependencies& p_dependencies)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(
            this->heap.shaders_v3, this->shader_allocation_events, p_shader.id,
            [&p_dependencies](hash_t p_id) {
                return ShaderRessource{RessourceIdentifiedHeader::build_database_with_id(p_id), tk_bd(ShaderIndex),
                                       ShaderRessource::Dependencies{p_dependencies.vertex_shader, p_dependencies.fragment_shader}};
            },
            [](const Token(ShaderRessource) p_allocated_ressource) {
                return ShaderRessource::AllocationEvent{ShaderRessource::Asset{}, p_allocated_ressource};
            });
    };

    inline void free_shader(const ShaderRessource& p_shader)
    {
        RessourceComposition::free_ressource_composition_explicit(this->heap.shaders_v3, this->shader_allocation_events, this->shader_free_events, p_shader.header,
                                                                  ShaderRessource::FreeEvent::build_from_token, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});
    };

    inline Token(MeshRessource) allocate_mesh_inline(const MeshRessource::InlineAllocationInput& p_mesh)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(this->heap.mesh_v2, this->mesh_allocation_events, p_mesh.id, MeshRessource::build_inline_from_id,
                                                                             [&p_mesh](const Token(MeshRessource) p_allocated_ressource) {
                                                                                 return MeshRessource::AllocationEvent{p_mesh.asset, p_allocated_ressource};
                                                                             });
    };

    inline Token(MeshRessource) allocate_mesh_database(const MeshRessource::DatabaseAllocationInput& p_mesh)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(this->heap.mesh_v2, this->mesh_allocation_events, p_mesh.id, MeshRessource::build_database_from_id,
                                                                             [](const Token(MeshRessource) p_allocated_ressource) {
                                                                                 return MeshRessource::AllocationEvent{MeshRessource::Asset{}, p_allocated_ressource};
                                                                             });
    };

    inline void free_mesh(const MeshRessource& p_mesh)
    {
        RessourceComposition::free_ressource_composition_explicit(this->heap.mesh_v2, this->mesh_allocation_events, this->mesh_free_events, p_mesh.header, MeshRessource::FreeEvent::build_from_token,
                                                                  RessourceComposition::AllocationEventFoundSlot::FreeAsset{});
    };

    inline Token(TextureRessource) allocate_texture_inline(const TextureRessource::InlineRessourceInput& p_texture_ressource)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(
            this->heap.textures, this->texture_allocation_events, p_texture_ressource.id, [](const hash_t p_id) { return TextureRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id)}; },
            [&p_texture_ressource](const Token(TextureRessource) p_allocated_ressource) {
                return TextureRessource::AllocationEvent{p_texture_ressource.asset, p_allocated_ressource};
            });
    };

    inline Token(TextureRessource) allocate_texture_database(const TextureRessource::DatabaseRessourceInput& p_texture_ressource)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(
            this->heap.textures, this->texture_allocation_events, p_texture_ressource.id, [](const hash_t p_id) { return TextureRessource{RessourceIdentifiedHeader::build_database_with_id(p_id)}; },
            [](const Token(TextureRessource) p_allocated_ressource) {
                return TextureRessource::AllocationEvent{TextureRessource::Asset{}, p_allocated_ressource};
            });
    };

    inline void free_texture(const TextureRessource& p_texture_ressource)
    {
        RessourceComposition::free_ressource_composition_explicit(
            this->heap.textures, this->texture_allocation_events, this->texture_free_events, p_texture_ressource.header,
            [](const Token(TextureRessource) p_texture_ressource) { return TextureRessource::FreeEvent{p_texture_ressource}; }, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});
    };

    inline Token(Slice<MaterialRessource::DynamicDependency>) allocate_material_parameter_dependencies_inline(const MaterialRessource::InlineAllocationInput& p_material_ressource)
    {
        return RessourceComposition::allocate_dynamic_dependencies(this->heap.materials, this->heap.material_dynamic_dependencies, p_material_ressource.id, [this, &p_material_ressource]() {
            Span<MaterialRessource::DynamicDependency> l_material_texture_ressources = Span<MaterialRessource::DynamicDependency>::allocate(p_material_ressource.texture_dependencies_input.Size);
            for (loop(i, 0, p_material_ressource.texture_dependencies_input.Size))
            {
                l_material_texture_ressources.get(0) = MaterialRessource::DynamicDependency{this->allocate_texture_inline(p_material_ressource.texture_dependencies_input.get(0))};
            }
            Token(Slice<MaterialRessource::DynamicDependency>) l_parameters = this->heap.material_dynamic_dependencies.alloc_vector_with_values(l_material_texture_ressources.slice);
            l_material_texture_ressources.free();
            return l_parameters;
        });
    };

    inline Token(Slice<MaterialRessource::DynamicDependency>) allocate_material_parameter_dependencies_database(const MaterialRessource::DatabaseAllocationInput& p_material_ressource)
    {
        return RessourceComposition::allocate_dynamic_dependencies(this->heap.materials, this->heap.material_dynamic_dependencies, p_material_ressource.id, [this, &p_material_ressource]() {
            Span<MaterialRessource::DynamicDependency> l_material_texture_ressources = Span<MaterialRessource::DynamicDependency>::allocate(p_material_ressource.texture_dependencies_input.Size);
            for (loop(i, 0, p_material_ressource.texture_dependencies_input.Size))
            {
                l_material_texture_ressources.get(0) = MaterialRessource::DynamicDependency{this->allocate_texture_database(p_material_ressource.texture_dependencies_input.get(0))};
            }
            Token(Slice<MaterialRessource::DynamicDependency>) l_parameters = this->heap.material_dynamic_dependencies.alloc_vector_with_values(l_material_texture_ressources.slice);
            l_material_texture_ressources.free();
            return l_parameters;
        });
    };

    inline void free_material_parameter_dependencies(const MaterialRessource& p_material)
    {
        Slice<MaterialRessource::DynamicDependency> l_material_parameter_dependencies = this->heap.material_dynamic_dependencies.get_vector(p_material.dependencies.dynamic_dependencies);
        for (loop(i, 0, l_material_parameter_dependencies.Size))
        {
            this->free_texture(this->heap.textures.pool.get(l_material_parameter_dependencies.get(i).dependency));
        }
        this->heap.material_dynamic_dependencies.release_vector(p_material.dependencies.dynamic_dependencies);
    };

    inline Token(MaterialRessource) allocate_material_inline(const MaterialRessource::InlineAllocationInput& p_material_ressource, const MaterialRessource::Dependencies& p_dependencies)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(
            this->heap.materials, this->material_allocation_events, p_material_ressource.id,
            [&p_dependencies](const hash_t p_id) {
                return MaterialRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id), tk_bd(Material), p_dependencies};
            },
            [&p_material_ressource](const Token(MaterialRessource) p_allocated_ressource) {
                return MaterialRessource::AllocationEvent{p_material_ressource.asset, p_allocated_ressource};
            });
    };

    inline Token(MaterialRessource) allocate_material_database(const MaterialRessource::DatabaseAllocationInput& p_material_ressource, const MaterialRessource::Dependencies& p_dependencies)
    {
        return RessourceComposition::allocate_ressource_composition_explicit(
            this->heap.materials, this->material_allocation_events, p_material_ressource.id,
            [&p_dependencies](const hash_t p_id) {
                return MaterialRessource{RessourceIdentifiedHeader::build_database_with_id(p_id), tk_bd(Material), p_dependencies};
            },
            [](const Token(MaterialRessource) p_allocated_ressource) {
                return MaterialRessource::AllocationEvent{MaterialRessource::Asset{}, p_allocated_ressource};
            });
    };

    inline int8 free_material(const MaterialRessource& p_material)
    {
        return RessourceComposition::free_ressource_composition_explicit(
            this->heap.materials, this->material_allocation_events, this->material_free_events, p_material.header,
            [](Token(MaterialRessource) p_removed_token) { return MaterialRessource::FreeEvent{p_removed_token}; }, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});
    };
};

struct RenderRessourceAllocator2Composition
{
    inline static Token(ShaderRessource)
        allocate_shader_v2_inline_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const ShaderRessource::InlineAllocationInput& p_shader,
                                                    const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader, const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader)
    {
        ShaderRessource::Dependencies l_shader_dependencies;
        l_shader_dependencies.vertex_shader = p_render_ressource_allocator.allocate_shadermodule_inline(p_vertex_shader);
        l_shader_dependencies.fragment_shader = p_render_ressource_allocator.allocate_shadermodule_inline(p_fragment_shader);
        return p_render_ressource_allocator.allocate_shader_v2_inline(p_shader, l_shader_dependencies);
    };

    inline static Token(ShaderRessource)
        allocate_shader_v2_database_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const ShaderRessource::DatabaseAllocationInput& p_shader,
                                                      const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader, const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader)
    {
        ShaderRessource::Dependencies l_shader_dependencies;
        l_shader_dependencies.vertex_shader = p_render_ressource_allocator.allocate_shadermodule_database(p_vertex_shader);
        l_shader_dependencies.fragment_shader = p_render_ressource_allocator.allocate_shadermodule_database(p_fragment_shader);
        return p_render_ressource_allocator.allocate_shader_v2_database(p_shader, l_shader_dependencies);
    };

    inline static Token(MaterialRessource)
        allocate_material_inline_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const MaterialRessource::InlineAllocationInput& p_material,
                                                   const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader, const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader,
                                                   const ShaderRessource::InlineAllocationInput& p_shader)
    {
        Token(ShaderRessource) l_shader_ressource =
            RenderRessourceAllocator2Composition::allocate_shader_v2_inline_with_dependencies(p_render_ressource_allocator, p_shader, p_vertex_shader, p_fragment_shader);
        Token(Slice<MaterialRessource::DynamicDependency>) l_material_parameters = p_render_ressource_allocator.allocate_material_parameter_dependencies_inline(p_material);
        return p_render_ressource_allocator.allocate_material_inline(p_material, MaterialRessource::Dependencies{l_shader_ressource, l_material_parameters});
    };

    inline static Token(MaterialRessource)
        allocate_material_database_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const MaterialRessource::DatabaseAllocationInput& p_material,
                                                     const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader, const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader,
                                                     const ShaderRessource::DatabaseAllocationInput& p_shader)
    {
        Token(ShaderRessource) l_shader_ressource =
            RenderRessourceAllocator2Composition::allocate_shader_v2_database_with_dependencies(p_render_ressource_allocator, p_shader, p_vertex_shader, p_fragment_shader);
        Token(Slice<MaterialRessource::DynamicDependency>) l_material_parameters = p_render_ressource_allocator.allocate_material_parameter_dependencies_database(p_material);
        return p_render_ressource_allocator.allocate_material_database(p_material, MaterialRessource::Dependencies{l_shader_ressource, l_material_parameters});
    };

    inline static Token(MaterialRessource) allocate_material_database_with_asset_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const hash_t p_id,
                                                                                              const MaterialRessource::AssetDependencies& p_material_asset_dependencies)
    {
        MaterialRessource::AssetDependencies::Value l_material_asset_dependencies_value = MaterialRessource::AssetDependencies::Value::build_from_asset(p_material_asset_dependencies);
        return allocate_material_database_with_dependencies(
            p_render_ressource_allocator,
            MaterialRessource::DatabaseAllocationInput{p_id, slice_cast<TextureRessource::DatabaseRessourceInput>(l_material_asset_dependencies_value.textures.build_asint8())},
            ShaderModuleRessource::DatabaseAllocationInput{l_material_asset_dependencies_value.shader_dependencies.vertex_module},
            ShaderModuleRessource::DatabaseAllocationInput{l_material_asset_dependencies_value.shader_dependencies.fragment_module},
            ShaderRessource::DatabaseAllocationInput{l_material_asset_dependencies_value.shader});
    };

    inline static Token(MaterialRessource)
        allcoate_material_database_and_load_asset_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, AssetDatabase& p_asset_database, const hash_t p_id)
    {
        // TODO -> we are potentially doing a database request for nothing. If the ressource is already allocated, then the asset dependencies will be allocated for nothing.
        // because there is no need to reallocate
        Span<int8> l_asset_dependencies = p_asset_database.get_asset_dependencies_blob(p_id);
        Token(MaterialRessource) l_ressource = allocate_material_database_with_asset_dependencies(p_render_ressource_allocator, p_id, MaterialRessource::AssetDependencies{l_asset_dependencies});
        l_asset_dependencies.free();

        return l_ressource;
    };

    inline static void free_shader_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const Token(ShaderRessource) p_shader_ressource)
    {
        ShaderRessource& l_shader_ressource = p_render_ressource_allocator.heap.shaders_v3.pool.get(p_shader_ressource);
        ShaderModuleRessource& l_vertex_module_ressource = p_render_ressource_allocator.heap.shader_modules_v2.pool.get(l_shader_ressource.dependencies.vertex_shader);
        ShaderModuleRessource& l_fragment_module_ressource = p_render_ressource_allocator.heap.shader_modules_v2.pool.get(l_shader_ressource.dependencies.fragment_shader);
        p_render_ressource_allocator.free_shadermodule(l_vertex_module_ressource);
        p_render_ressource_allocator.free_shadermodule(l_fragment_module_ressource);
        p_render_ressource_allocator.free_shader(l_shader_ressource);
    };

    inline static void free_material_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator, const Token(MaterialRessource) p_material_ressource)
    {
        MaterialRessource& l_material_ressource = p_render_ressource_allocator.heap.materials.pool.get(p_material_ressource);
        free_shader_with_dependencies(p_render_ressource_allocator, l_material_ressource.dependencies.shader);
        if (p_render_ressource_allocator.free_material(l_material_ressource))
        {
            p_render_ressource_allocator.free_material_parameter_dependencies(l_material_ressource);
        }
    };
};

}; // namespace v2