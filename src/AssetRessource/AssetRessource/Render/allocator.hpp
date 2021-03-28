#pragma once

struct ShaderModuleRessourceUnit
{
    PoolHashedCounted<hash_t, ShaderModuleRessource> shader_modules;
    Vector<ShaderModuleRessource::AllocationEvent> shader_modules_allocation_events;
    Vector<ShaderModuleRessource::FreeEvent> shader_modules_free_events;

    inline static ShaderModuleRessourceUnit allocate()
    {
        return ShaderModuleRessourceUnit{PoolHashedCounted<hash_t, ShaderModuleRessource>::allocate_default(), Vector<ShaderModuleRessource::AllocationEvent>::allocate(0),
                                         Vector<ShaderModuleRessource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->shader_modules_allocation_events.empty());
        assert_true(this->shader_modules_free_events.empty());
        assert_true(this->shader_modules.empty());
#endif
        this->shader_modules_free_events.free();
        this->shader_modules_allocation_events.free();
        this->shader_modules.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->shader_modules_allocation_events.empty());
#endif
    };

    inline void deallocation_step(GPUContext& p_gpu_context)
    {
        for (loop_reverse(i, 0, this->shader_modules_free_events.Size))
        {
            auto& l_event = this->shader_modules_free_events.get(i);
            ShaderModuleRessource& l_ressource = this->shader_modules.pool.get(l_event.ressource);
            p_gpu_context.graphics_allocator.free_shader_module(l_ressource.shader_module);
            this->shader_modules.pool.release_element(l_event.ressource);
            this->shader_modules_free_events.pop_back();
        }
    };

    inline void allocation_step(GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->shader_modules_allocation_events.Size))
        {
            auto& l_event = this->shader_modules_allocation_events.get(i);
            ShaderModuleRessource& l_ressource = this->shader_modules.pool.get(l_event.allocated_ressource);
            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_database_connection, p_asset_database, l_ressource.header, &l_event.asset);

            ShaderModuleRessource::Asset::Value l_value = ShaderModuleRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_value.compiled_shader);
            l_ressource.header.allocated = 1;
            l_event.asset.free();
            this->shader_modules_allocation_events.pop_back();
        }
    };

    inline Token(ShaderModuleRessource) allocate_ressource(const hash_t p_id, const RessourceAllocationType p_ressource_allocation_type, const ShaderModuleRessource::Asset& p_asset)
    {
        Token(ShaderModuleRessource) l_shader_module_ressource = this->shader_modules.push_back_element(p_id, ShaderModuleRessource{RessourceIdentifiedHeader{p_ressource_allocation_type, 0, p_id}});
        this->shader_modules_allocation_events.push_back_element(ShaderModuleRessource::AllocationEvent{p_asset, l_shader_module_ressource});
        return l_shader_module_ressource;
    };

    inline Token(ShaderModuleRessource) increment_ressource(const hash_t p_id)
    {
        return this->shader_modules.increment(p_id);
    };

    inline void increment_ressource_from_token(const Token(ShaderModuleRessource) p_shader_module)
    {
        this->increment_ressource(this->shader_modules.pool.get(p_shader_module).header.id);
    };

    inline void release_ressource(const Token(ShaderModuleRessource) p_shader_module)
    {
        auto l_free_return_code = RessourceComposition::free_ressource_composition_2(this->shader_modules, this->shader_modules_free_events, this->shader_modules_allocation_events,
                                                                                     this->shader_modules.pool.get(p_shader_module));
        if (l_free_return_code == RessourceComposition::FreeRessourceCompositionReturnCode::POOL_DEALLOCATION_AWAITING)
        {
            this->shader_modules.pool.release_element(p_shader_module);
        }
    };

    inline void decrement_ressource(const hash_t p_id)
    {
        this->shader_modules.decrement(p_id);
    };

    inline int8 is_ressource_id_allocated(const hash_t p_id)
    {
        return this->shader_modules.has_key_nothashed(p_id);
    };
};

struct ShaderModuleRessourceComposition
{
    inline static Token(ShaderModuleRessource) allocate_or_increment_inline(ShaderModuleRessourceUnit& p_unit, const ShaderModuleRessource::InlineAllocationInput& p_inline_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.shader_modules.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::INLINE, p_inline_input.asset);
        }
    };

    inline static Token(ShaderModuleRessource) allocate_or_increment_database(ShaderModuleRessourceUnit& p_unit, const ShaderModuleRessource::DatabaseAllocationInput& p_inline_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.shader_modules.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::ASSET_DATABASE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::ASSET_DATABASE, ShaderModuleRessource::Asset{});
        }
    };
};

struct TextureRessourceUnit
{
    PoolHashedCounted<hash_t, TextureRessource> textures;
    Vector<TextureRessource::AllocationEvent> textures_allocation_events;
    Vector<TextureRessource::FreeEvent> textures_free_events;

    inline static TextureRessourceUnit allocate()
    {
        return TextureRessourceUnit{PoolHashedCounted<hash_t, TextureRessource>::allocate_default(), Vector<TextureRessource::AllocationEvent>::allocate(0),
                                    Vector<TextureRessource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->textures_allocation_events.empty());
        assert_true(this->textures_free_events.empty());
        assert_true(this->textures.empty());
#endif
        this->textures_free_events.free();
        this->textures_allocation_events.free();
        this->textures.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->textures_allocation_events.empty());
#endif
    };

    inline void deallocation_step(GPUContext& p_gpu_context)
    {
        for (loop_reverse(i, 0, this->textures_free_events.Size))
        {
            auto& l_event = this->textures_free_events.get(i);
            TextureRessource& l_ressource = this->textures.pool.get(l_event.ressource);
            GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_ressource.texture);
            this->textures.pool.release_element(l_event.ressource);
            this->textures_free_events.pop_back();
        }
    };

    inline void allocation_step(GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->textures_allocation_events.Size))
        {
            auto& l_event = this->textures_allocation_events.get(i);
            TextureRessource& l_ressource = this->textures.pool.get(l_event.allocated_ressource);

            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_database_connection, p_asset_database, l_ressource.header, &l_event.asset);

            TextureRessource::Asset::Value l_value = TextureRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.texture = ShaderParameterBufferAllocationFunctions::allocate_texture_gpu_for_shaderparameter(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory,
                                                                                                                     ImageFormat::build_color_2d(l_value.size, ImageUsageFlag::UNDEFINED));
            TextureGPU& l_texture_gpu = p_gpu_context.graphics_allocator.heap.textures_gpu.get(l_ressource.texture);
            BufferReadWrite::write_to_imagegpu(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, l_texture_gpu.Image,
                                               p_gpu_context.buffer_memory.allocator.gpu_images.get(l_texture_gpu.Image), l_value.pixels);

            l_event.asset.free();
            l_ressource.header.allocated = 1;
            this->textures_allocation_events.pop_back();
        }
    };

    inline Token(TextureRessource) allocate_ressource(const hash_t p_id, const RessourceAllocationType p_ressource_allocation_type, const TextureRessource::Asset& p_asset)
    {
        Token(TextureRessource) l_texture_ressource = this->textures.push_back_element(p_id, TextureRessource{RessourceIdentifiedHeader{p_ressource_allocation_type, 0, p_id}});
        this->textures_allocation_events.push_back_element(TextureRessource::AllocationEvent{p_asset, l_texture_ressource});
        return l_texture_ressource;
    };

    inline Token(TextureRessource) increment_ressource(const hash_t p_id)
    {
        return this->textures.increment(p_id);
    };

    inline void increment_ressource_from_token(const Token(TextureRessource) p_texture_ressource)
    {
        this->increment_ressource(this->textures.pool.get(p_texture_ressource).header.id);
    };

    inline void release_ressource(const Token(TextureRessource) p_texture_ressource)
    {
        auto l_free_return_code =
            RessourceComposition::free_ressource_composition_2(this->textures, this->textures_free_events, this->textures_allocation_events, this->textures.pool.get(p_texture_ressource));
        if (l_free_return_code == RessourceComposition::FreeRessourceCompositionReturnCode::POOL_DEALLOCATION_AWAITING)
        {
            this->textures.pool.release_element(p_texture_ressource);
        }
    };

    inline void decrement_ressource(const hash_t p_id)
    {
        this->textures.decrement(p_id);
    };

    inline int8 is_ressource_id_allocated(const hash_t p_id)
    {
        return this->textures.has_key_nothashed(p_id);
    };
};

struct TextureRessourceComposition
{
    inline static Token(TextureRessource) allocate_or_increment_inline(TextureRessourceUnit& p_unit, const TextureRessource::InlineAllocationInput& p_inline_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.textures.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::INLINE, p_inline_input.asset);
        }
    };

    inline static Token(TextureRessource) allocate_or_increment_database(TextureRessourceUnit& p_unit, const TextureRessource::DatabaseAllocationInput& p_inline_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.textures.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::ASSET_DATABASE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::ASSET_DATABASE, TextureRessource::Asset{});
        }
    };
};

struct MeshRessourceUnit
{
    PoolHashedCounted<hash_t, MeshRessource> meshes;
    Vector<MeshRessource::AllocationEvent> meshes_allocation_events;
    Vector<MeshRessource::FreeEvent> meshes_free_events;

    inline static MeshRessourceUnit allocate()
    {
        return MeshRessourceUnit{PoolHashedCounted<hash_t, MeshRessource>::allocate_default(), Vector<MeshRessource::AllocationEvent>::allocate(0), Vector<MeshRessource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->meshes_allocation_events.empty());
        assert_true(this->meshes_free_events.empty());
        assert_true(this->meshes.empty());
#endif
        this->meshes_free_events.free();
        this->meshes_allocation_events.free();
        this->meshes.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->meshes_allocation_events.empty());
#endif
    };

    inline void deallocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        for (loop_reverse(i, 0, this->meshes_free_events.Size))
        {
            auto& l_event = this->meshes_free_events.get(i);
            MeshRessource& l_ressource = this->meshes.pool.get(l_event.ressource);
            D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_ressource.mesh);
            this->meshes.pool.release_element(l_event.ressource);
            this->meshes_free_events.pop_back();
        }
    };

    inline void allocation_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->meshes_allocation_events.Size))
        {
            auto& l_event = this->meshes_allocation_events.get(i);

            MeshRessource& l_ressource = this->meshes.pool.get(l_event.allocated_ressource);
            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_database_connection, p_asset_database, l_ressource.header, &l_event.asset);

            MeshRessource::Asset::Value l_value = MeshRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_value.initial_vertices, l_value.initial_indices);
            l_ressource.header.allocated = 1;
            l_event.asset.free();
            this->meshes_allocation_events.pop_back();
        }
    };

    inline Token(MeshRessource) allocate_ressource(const hash_t p_id, const RessourceAllocationType p_ressource_allocation_type, const MeshRessource::Asset& p_asset)
    {
        Token(MeshRessource) l_mesh_ressource = this->meshes.push_back_element(p_id, MeshRessource{RessourceIdentifiedHeader{p_ressource_allocation_type, 0, p_id}});
        this->meshes_allocation_events.push_back_element(MeshRessource::AllocationEvent{p_asset, l_mesh_ressource});
        return l_mesh_ressource;
    };

    inline Token(MeshRessource) increment_ressource(const hash_t p_id)
    {
        return this->meshes.increment(p_id);
    };

    inline void increment_ressource_from_token(const Token(MeshRessource) p_mesh_ressource)
    {
        this->increment_ressource(this->meshes.pool.get(p_mesh_ressource).header.id);
    };

    inline void release_ressource(const Token(MeshRessource) p_mesh_ressource)
    {
        auto l_free_return_code = RessourceComposition::free_ressource_composition_2(this->meshes, this->meshes_free_events, this->meshes_allocation_events, this->meshes.pool.get(p_mesh_ressource));
        if (l_free_return_code == RessourceComposition::FreeRessourceCompositionReturnCode::POOL_DEALLOCATION_AWAITING)
        {
            this->meshes.pool.release_element(p_mesh_ressource);
        }
    };

    inline void decrement_ressource(const hash_t p_id)
    {
        this->meshes.decrement(p_id);
    };

    inline int8 is_ressource_id_allocated(const hash_t p_id)
    {
        return this->meshes.has_key_nothashed(p_id);
    };
};

struct MeshRessourceComposition
{
    inline static Token(MeshRessource) allocate_or_increment_inline(MeshRessourceUnit& p_unit, const MeshRessource::InlineAllocationInput& p_inline_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.meshes.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::INLINE, p_inline_input.asset);
        }
    };

    inline static Token(MeshRessource) allocate_or_increment_database(MeshRessourceUnit& p_unit, const MeshRessource::DatabaseAllocationInput& p_inline_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.meshes.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::ASSET_DATABASE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::ASSET_DATABASE, MeshRessource::Asset{});
        }
    };
};

struct ShaderRessourceUnit
{
    PoolHashedCounted<hash_t, ShaderRessource> shaders;
    Vector<ShaderRessource::AllocationEvent> shaders_allocation_events;
    Vector<ShaderRessource::FreeEvent> shaders_free_events;

    inline static ShaderRessourceUnit allocate()
    {
        return ShaderRessourceUnit{PoolHashedCounted<hash_t, ShaderRessource>::allocate_default(), Vector<ShaderRessource::AllocationEvent>::allocate(0),
                                   Vector<ShaderRessource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->shaders_allocation_events.empty());
        assert_true(this->shaders_free_events.empty());
        assert_true(this->shaders.empty());
#endif
        this->shaders_free_events.free();
        this->shaders_allocation_events.free();
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
        for (loop_reverse(i, 0, this->shaders_free_events.Size))
        {
            auto& l_event = this->shaders_free_events.get(i);
            ShaderRessource& l_ressource = this->shaders.pool.get(l_event.ressource);
            D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, l_ressource.shader);
            this->shaders.pool.release_element(l_event.ressource);
            this->shaders_free_events.pop_back();
        }
    };

    inline void allocation_step(ShaderModuleRessourceUnit& p_shader_module_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection,
                                AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->shaders_allocation_events.Size))
        {
            auto& l_event = this->shaders_allocation_events.get(i);
            ShaderRessource& l_ressource = this->shaders.pool.get(l_event.allocated_ressource);

            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_database_connection, p_asset_database, l_ressource.header, &l_event.asset);

            ShaderModuleRessource& l_vertex_shader = p_shader_module_unit.shader_modules.pool.get(l_ressource.dependencies.vertex_shader);
            ShaderModuleRessource& l_fragment_shader = p_shader_module_unit.shader_modules.pool.get(l_ressource.dependencies.fragment_shader);

            ShaderRessource::Asset::Value l_value = ShaderRessource::Asset::Value::build_from_asset(l_event.asset);
            l_ressource.shader = D3RendererAllocatorComposition::allocate_colorstep_shader_with_shaderlayout(
                p_gpu_context.graphics_allocator, p_renderer.allocator, l_value.specific_parameters, l_value.execution_order,
                p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass), l_value.shader_configuration,
                p_gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.shader_module), p_gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.shader_module));
            l_event.asset.free();
            l_ressource.header.allocated = 1;
            this->shaders_allocation_events.pop_back();
        }
    };

    inline Token(ShaderRessource)
        allocate_ressource(const hash_t p_id, const RessourceAllocationType p_ressource_allocation_type, const ShaderRessource::Asset& p_asset, const ShaderRessource::Dependencies& p_dependencies)
    {
        Token(ShaderRessource) l_shader_module_ressource =
            this->shaders.push_back_element(p_id, ShaderRessource{RessourceIdentifiedHeader{p_ressource_allocation_type, 0, p_id}, token_build_default(ShaderIndex), p_dependencies});
        this->shaders_allocation_events.push_back_element(ShaderRessource::AllocationEvent{p_asset, l_shader_module_ressource});
        return l_shader_module_ressource;
    };

    inline Token(ShaderRessource) increment_ressource(const hash_t p_id)
    {
        return this->shaders.increment(p_id);
    };

    inline void increment_ressource_from_token(const Token(ShaderRessource) p_shader)
    {
        this->increment_ressource(this->shaders.pool.get(p_shader).header.id);
    };

    inline RessourceComposition::FreeRessourceCompositionReturnCode release_ressource(const Token(ShaderRessource) p_shader)
    {
        return RessourceComposition::free_ressource_composition_2(this->shaders, this->shaders_free_events, this->shaders_allocation_events, this->shaders.pool.get(p_shader));
    };

    inline PoolHashedCounted<hash_t, ShaderRessource>::CountElement* decrement_ressource(const hash_t p_id)
    {
        return this->shaders.decrement(p_id);
    };

    inline int8 is_ressource_id_allocated(const hash_t p_id)
    {
        return this->shaders.has_key_nothashed(p_id);
    };
};

struct ShaderRessourceComposition
{
    inline static Token(ShaderRessource)
        allocate_or_increment_inline(ShaderRessourceUnit& p_unit, ShaderModuleRessourceUnit& p_shader_module_unit, const ShaderRessource::InlineAllocationInput& p_inline_input,
                                     const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader_input, const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.shaders.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            ShaderRessource::Dependencies l_dependencies;
            l_dependencies.vertex_shader = ShaderModuleRessourceComposition::allocate_or_increment_inline(p_shader_module_unit, p_vertex_shader_input);
            l_dependencies.fragment_shader = ShaderModuleRessourceComposition::allocate_or_increment_inline(p_shader_module_unit, p_fragment_shader_input);
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::INLINE, p_inline_input.asset, l_dependencies);
        }
    };

    inline static Token(ShaderRessource)
        allocate_or_increment_database(ShaderRessourceUnit& p_unit, ShaderModuleRessourceUnit& p_shader_module_unit, const ShaderRessource::DatabaseAllocationInput& p_inline_input,
                                       const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input, const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.shaders.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::ASSET_DATABASE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            ShaderRessource::Dependencies l_dependencies;
            l_dependencies.vertex_shader = ShaderModuleRessourceComposition::allocate_or_increment_database(p_shader_module_unit, p_vertex_shader_input);
            l_dependencies.fragment_shader = ShaderModuleRessourceComposition::allocate_or_increment_database(p_shader_module_unit, p_fragment_shader_input);
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::ASSET_DATABASE, ShaderRessource::Asset{}, l_dependencies);
        }
    };

    inline static void decrement_or_release(ShaderRessourceUnit& p_unit, ShaderModuleRessourceUnit& p_shader_module_unit, const Token(ShaderRessource) p_shader_ressource)
    {
        switch (p_unit.release_ressource(p_shader_ressource))
        {
        case RessourceComposition::FreeRessourceCompositionReturnCode::FREE_EVENT_PUSHED:
        {
            ShaderRessource& l_shader_ressource = p_unit.shaders.pool.get(p_shader_ressource);
            p_shader_module_unit.release_ressource(l_shader_ressource.dependencies.vertex_shader);
            p_shader_module_unit.release_ressource(l_shader_ressource.dependencies.fragment_shader);
            break;
        }
        case RessourceComposition::FreeRessourceCompositionReturnCode::POOL_DEALLOCATION_AWAITING:
        {
            ShaderRessource& l_shader_ressource = p_unit.shaders.pool.get(p_shader_ressource);
            p_shader_module_unit.release_ressource(l_shader_ressource.dependencies.vertex_shader);
            p_shader_module_unit.release_ressource(l_shader_ressource.dependencies.fragment_shader);
            p_unit.shaders.pool.release_element(p_shader_ressource);
            break;
        };
        case RessourceComposition::FreeRessourceCompositionReturnCode::DECREMENTED:
            break;
        }
    };
};

struct MaterialRessourceUnit
{
    PoolHashedCounted<hash_t, MaterialRessource> materials;
    PoolOfVector<MaterialRessource::DynamicDependency> material_dynamic_dependencies;
    Vector<MaterialRessource::AllocationEvent> materials_allocation_events;
    Vector<MaterialRessource::FreeEvent> materials_free_events;

    inline static MaterialRessourceUnit allocate()
    {
        return MaterialRessourceUnit{PoolHashedCounted<hash_t, MaterialRessource>::allocate_default(), PoolOfVector<MaterialRessource::DynamicDependency>::allocate_default(),
                                     Vector<MaterialRessource::AllocationEvent>::allocate(0), Vector<MaterialRessource::FreeEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->materials_allocation_events.empty());
        assert_true(this->materials_free_events.empty());
        assert_true(this->materials.empty());
        assert_true(!this->material_dynamic_dependencies.has_allocated_elements());
#endif
        this->materials_free_events.free();
        this->materials_allocation_events.free();
        this->materials.free();
        this->material_dynamic_dependencies.free();
    };

    inline void assert_no_allocation_events()
    {
#if __DEBUG
        assert_true(this->materials_allocation_events.empty());
#endif
    };

    inline void deallocation_step(ShaderRessourceUnit& p_shader_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context)
    {
        for (loop_reverse(i, 0, this->materials_free_events.Size))
        {
            auto& l_event = this->materials_free_events.get(i);
            MaterialRessource& l_ressource = this->materials.pool.get(l_event.ressource);
            ShaderRessource& l_linked_shader = p_shader_unit.shaders.pool.get(l_ressource.dependencies.shader);
            p_renderer.allocator.heap.unlink_shader_with_material(l_linked_shader.shader, l_ressource.material);
            D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_ressource.material);
            this->materials.pool.release_element(l_event.ressource);
            this->materials_free_events.pop_back();
        }
    };

    inline void allocation_step(ShaderRessourceUnit& p_shader_unit, TextureRessourceUnit& p_texture_unit, D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection,
                                AssetDatabase& p_asset_database)
    {
        for (loop_reverse(i, 0, this->materials_allocation_events.Size))
        {
            auto& l_event = this->materials_allocation_events.get(i);
            MaterialRessource& l_ressource = this->materials.pool.get(l_event.allocated_ressource);

            RessourceComposition::retrieve_ressource_asset_from_database_if_necessary(p_database_connection, p_asset_database, l_ressource.header, &l_event.asset);

            ShaderRessource& l_shader = p_shader_unit.shaders.pool.get(l_ressource.dependencies.shader);
            ShaderIndex& l_shader_index = p_renderer.allocator.heap.shaders.get(l_shader.shader);

            MaterialRessource::Asset::Value l_value = MaterialRessource::Asset::Value::build_from_asset(l_event.asset);
            Material l_material_value = Material::allocate_empty(p_gpu_context.graphics_allocator, 1);

            for (loop(j, 0, l_value.parameters.parameters.get_size()))
            {
                switch (l_value.parameters.get_parameter_type(j))
                {
                case ShaderParameter::Type::UNIFORM_HOST:
                {
                    Slice<int8> l_element = l_value.parameters.get_parameter_uniform_host_value(j);
                    l_material_value.add_and_allocate_buffer_host_parameter(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory.allocator,
                                                                            p_gpu_context.graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), l_element);
                }
                break;
                case ShaderParameter::Type::TEXTURE_GPU:
                {
                    hash_t* l_texture_id = l_value.parameters.get_parameter_texture_gpu_value(j);
                    Token(TextureGPU) l_texture = p_texture_unit.textures.pool.get(p_texture_unit.textures.CountMap.get_value_nothashed(*l_texture_id)->token).texture;
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
            this->materials_allocation_events.pop_back();
        }
    };

    inline Token(MaterialRessource)
        allocate_ressource(const hash_t p_id, const RessourceAllocationType p_ressource_allocation_type, const MaterialRessource::Asset& p_asset, const MaterialRessource::Dependencies& p_dependencies)
    {
        Token(MaterialRessource) l_shader_module_ressource =
            this->materials.push_back_element(p_id, MaterialRessource{RessourceIdentifiedHeader{p_ressource_allocation_type, 0, p_id}, token_build_default(Material), p_dependencies});
        this->materials_allocation_events.push_back_element(MaterialRessource::AllocationEvent{p_asset, l_shader_module_ressource});
        return l_shader_module_ressource;
    };

    inline Token(MaterialRessource) increment_ressource(const hash_t p_id)
    {
        return this->materials.increment(p_id);
    };

    inline void increment_ressource_from_token(const Token(MaterialRessource) p_shader)
    {
        this->increment_ressource(this->materials.pool.get(p_shader).header.id);
    };

    inline RessourceComposition::FreeRessourceCompositionReturnCode release_ressource(const Token(MaterialRessource) p_shader)
    {
        return RessourceComposition::free_ressource_composition_2(this->materials, this->materials_free_events, this->materials_allocation_events, this->materials.pool.get(p_shader));
    };

    inline PoolHashedCounted<hash_t, MaterialRessource>::CountElement* decrement_ressource(const hash_t p_id)
    {
        return this->materials.decrement(p_id);
    };

    inline int8 is_ressource_id_allocated(const hash_t p_id)
    {
        return this->materials.has_key_nothashed(p_id);
    };
};

struct MaterialRessourceComposition
{
    inline static Token(MaterialRessource)
        allocate_or_increment_inline(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                                     const MaterialRessource::InlineAllocationInput& p_inline_input, const ShaderRessource::InlineAllocationInput& p_shader_input,
                                     const ShaderModuleRessource::InlineAllocationInput& p_vertex_shader_input, const ShaderModuleRessource::InlineAllocationInput& p_fragment_shader_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.materials.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::INLINE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            MaterialRessource::Dependencies l_dependencies;
            l_dependencies.shader = ShaderRessourceComposition::allocate_or_increment_inline(p_shader_unit, p_shader_module_unit, p_shader_input, p_vertex_shader_input, p_fragment_shader_input);
            Span<MaterialRessource::DynamicDependency> l_dynamic_textures = Span<MaterialRessource::DynamicDependency>::allocate(p_inline_input.texture_dependencies_input.Size);
            for (loop(i, 0, l_dynamic_textures.Capacity))
            {
                l_dynamic_textures.get(i) =
                    MaterialRessource::DynamicDependency{TextureRessourceComposition::allocate_or_increment_inline(p_texture_unit, *Slice_get(&p_inline_input.texture_dependencies_input, i))};
            }
            l_dependencies.dynamic_dependencies = p_unit.material_dynamic_dependencies.alloc_vector_with_values(l_dynamic_textures.slice);
            l_dynamic_textures.free();
            return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::INLINE, p_inline_input.asset, l_dependencies);
        }
    };

    inline static Token(MaterialRessource)
        allocate_database(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                          const MaterialRessource::DatabaseAllocationInput& p_inline_input, const ShaderRessource::DatabaseAllocationInput& p_shader_input,
                          const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input, const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        MaterialRessource::Dependencies l_dependencies;
        l_dependencies.shader = ShaderRessourceComposition::allocate_or_increment_database(p_shader_unit, p_shader_module_unit, p_shader_input, p_vertex_shader_input, p_fragment_shader_input);
        Span<MaterialRessource::DynamicDependency> l_dynamic_textures = Span<MaterialRessource::DynamicDependency>::allocate(p_inline_input.texture_dependencies_input.Size);
        for (loop(i, 0, l_dynamic_textures.Capacity))
        {
            l_dynamic_textures.get(i) =
                MaterialRessource::DynamicDependency{TextureRessourceComposition::allocate_or_increment_database(p_texture_unit, *Slice_get(&p_inline_input.texture_dependencies_input, i))};
        }
        l_dependencies.dynamic_dependencies = p_unit.material_dynamic_dependencies.alloc_vector_with_values(l_dynamic_textures.slice);
        l_dynamic_textures.free();
        return p_unit.allocate_ressource(p_inline_input.id, RessourceAllocationType::ASSET_DATABASE, MaterialRessource::Asset{}, l_dependencies);
    };

    inline static Token(MaterialRessource)
        allocate_or_increment_database(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                                       const MaterialRessource::DatabaseAllocationInput& p_inline_input, const ShaderRessource::DatabaseAllocationInput& p_shader_input,
                                       const ShaderModuleRessource::DatabaseAllocationInput& p_vertex_shader_input, const ShaderModuleRessource::DatabaseAllocationInput& p_fragment_shader_input)
    {
        if (p_unit.is_ressource_id_allocated(p_inline_input.id))
        {
#if __DEBUG
            assert_true(p_unit.materials.get(p_inline_input.id).header.allocation_type == RessourceAllocationType::ASSET_DATABASE);
#endif
            return p_unit.increment_ressource(p_inline_input.id);
        }
        else
        {
            return allocate_database(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, p_inline_input, p_shader_input, p_vertex_shader_input, p_fragment_shader_input);
        }
    };

    inline static Token(MaterialRessource)
        allocate_or_increment_database_and_load_dependecies(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit,
                                                            TextureRessourceUnit& p_texture_unit, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database, const hash_t p_id)
    {
        if (p_unit.is_ressource_id_allocated(p_id))
        {
            return p_unit.increment_ressource(p_id);
        }
        else
        {
            Span<int8> l_asset_dependencies = p_asset_database.get_asset_dependencies_blob(p_database_connection, p_id);
            MaterialRessource::AssetDependencies::Value l_asset_dependencies_value =
                MaterialRessource::AssetDependencies::Value::build_from_asset(MaterialRessource::AssetDependencies{l_asset_dependencies});

            MaterialRessource::DatabaseAllocationInput l_material_database_input;
            l_material_database_input.id = p_id;
            l_material_database_input.texture_dependencies_input = *(Slice<TextureRessource::DatabaseAllocationInput>*)&l_asset_dependencies_value.textures;

            Token(MaterialRessource) l_material_ressource =
                allocate_database(p_unit, p_shader_unit, p_shader_module_unit, p_texture_unit, l_material_database_input, ShaderRessource::DatabaseAllocationInput{l_asset_dependencies_value.shader},
                                  ShaderModuleRessource::DatabaseAllocationInput{l_asset_dependencies_value.shader_dependencies.vertex_module},
                                  ShaderModuleRessource::DatabaseAllocationInput{l_asset_dependencies_value.shader_dependencies.fragment_module});
            l_asset_dependencies.free();
            return l_material_ressource;
        }
    };

    inline static void decrement_or_release(MaterialRessourceUnit& p_unit, ShaderRessourceUnit& p_shader_unit, ShaderModuleRessourceUnit& p_shader_module_unit, TextureRessourceUnit& p_texture_unit,
                                            const Token(MaterialRessource) p_material_ressource)
    {
        switch (p_unit.release_ressource(p_material_ressource))
        {
        case RessourceComposition::FreeRessourceCompositionReturnCode::FREE_EVENT_PUSHED:
        {
            MaterialRessource& l_material_ressource = p_unit.materials.pool.get(p_material_ressource);
            ShaderRessourceComposition::decrement_or_release(p_shader_unit, p_shader_module_unit, l_material_ressource.dependencies.shader);
            Slice<MaterialRessource::DynamicDependency> l_material_dynamic_dependencies = p_unit.material_dynamic_dependencies.get_vector(l_material_ressource.dependencies.dynamic_dependencies);
            for (loop(i, 0, l_material_dynamic_dependencies.Size))
            {
                p_texture_unit.release_ressource(Slice_get(&l_material_dynamic_dependencies, i)->dependency);
            }
            p_unit.material_dynamic_dependencies.release_vector(l_material_ressource.dependencies.dynamic_dependencies);
            break;
        }
        case RessourceComposition::FreeRessourceCompositionReturnCode::POOL_DEALLOCATION_AWAITING:
        {
            MaterialRessource& l_material_ressource = p_unit.materials.pool.get(p_material_ressource);
            ShaderRessourceComposition::decrement_or_release(p_shader_unit, p_shader_module_unit, l_material_ressource.dependencies.shader);
            Slice<MaterialRessource::DynamicDependency> l_material_dynamic_dependencies = p_unit.material_dynamic_dependencies.get_vector(l_material_ressource.dependencies.dynamic_dependencies);
            for (loop(i, 0, l_material_dynamic_dependencies.Size))
            {
                p_texture_unit.release_ressource(Slice_get(&l_material_dynamic_dependencies, i)->dependency);
            }

            p_unit.materials.pool.release_element(p_material_ressource);
            p_unit.material_dynamic_dependencies.release_vector(l_material_ressource.dependencies.dynamic_dependencies);
            break;
        };
        case RessourceComposition::FreeRessourceCompositionReturnCode::DECREMENTED:
            break;
        }
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
