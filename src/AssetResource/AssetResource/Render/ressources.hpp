#pragma once

struct ShaderModuleResource
{
    ResourceIdentifiedHeader header;
    ShaderModule_Token resource;

    RESOURCE_DECLARE_TYPES(ShaderModule);
    using sToken = iResource_Token<ShaderModuleResource>::sToken;
    using sTokenValue = iResource_Token<ShaderModuleResource>::sTokenValue;

    inline static ShaderModuleResource build_inline_from_id(const hash_t p_id)
    {
        return ShaderModuleResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<ShaderModule_TokenValue>()};
    };

    inline static ShaderModuleResource build_database_from_id(const hash_t p_id)
    {
        return ShaderModuleResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<ShaderModule_TokenValue>()};
    };

    struct Asset
    {
        Span<int8> allocated_binary;

        inline void free()
        {
            this->allocated_binary.free();
        };

        struct Value
        {
            Slice<int8> compiled_shader;

            inline static Value build_from_asset(const Asset& p_asset)
            {
                return Value{p_asset.allocated_binary.slice};
            };
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_value)
        {
            return Asset{Span<int8>::allocate_slice(p_value.compiled_shader)};
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct DatabaseAllocationEvent
    {
        RESOURCE_DatabaseAllocationEvent_DECLARE_TYPES(ShaderModuleResource);

        hash_t id;
        ShaderModuleResource::sToken allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct InlineAllocationEvent
    {
        RESOURCE_InlineAllocationEvent_DECLARE_TYPES(ShaderModuleResource);

        Asset asset;
        ShaderModuleResource::sToken allocated_resource;
    };

    struct FreeEvent
    {
        RESOURCE_FreeEvent_DECLARE_TYPES(ShaderModuleResource);

        ShaderModuleResource::sToken allocated_resource;

        inline static FreeEvent build_from_token(const ShaderModuleResource::sToken p_token)
        {
            return FreeEvent{p_token};
        };
    };
};

struct MeshResource
{
    ResourceIdentifiedHeader header;
    Mesh_Token resource;

    RESOURCE_DECLARE_TYPES(Mesh);
    using sToken = iResource_Token<MeshResource>::sToken;
    using sTokenValue = iResource_Token<MeshResource>::sTokenValue;

    inline static MeshResource build_inline_from_id(const hash_t p_id)
    {
        return MeshResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<Mesh_TokenValue>()};
    };

    inline static MeshResource build_database_from_id(const hash_t p_id)
    {
        return MeshResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<Mesh_TokenValue>()};
    };

    struct Asset
    {
        Span<int8> allocated_binary;

        inline void free()
        {
            this->allocated_binary.free();
        }

        struct Value
        {
            Slice<Vertex> initial_vertices;
            Slice<uint32> initial_indices;

            inline static Value build_from_asset(const Asset& p_asset)
            {
                Value l_value;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                l_value.initial_vertices = slice_cast<Vertex>(l_deserializer.slice());
                l_value.initial_indices = slice_cast<uint32>(l_deserializer.slice());
                return l_value;
            };

            inline uimax binary_size() const
            {
                return BinarySize::slice(this->initial_vertices.build_asint8()) + BinarySize::slice(this->initial_indices.build_asint8());
            };
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_values)
        {
            Span<int8> l_binary_buffer = Span<int8>::allocate(p_values.binary_size());
            VectorSlice<int8> l_binary = VectorSlice<int8>::build(l_binary_buffer.slice, 0);
            BinarySerializer::slice(iVector<VectorSlice<int8>>{l_binary}, p_values.initial_vertices.build_asint8());
            BinarySerializer::slice(iVector<VectorSlice<int8>>{l_binary}, p_values.initial_indices.build_asint8());
            return build_from_binary(l_binary_buffer);
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct DatabaseAllocationEvent
    {
        RESOURCE_DatabaseAllocationEvent_DECLARE_TYPES(MeshResource);

        hash_t id;
        MeshResource::sToken allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct InlineAllocationEvent
    {
        RESOURCE_InlineAllocationEvent_DECLARE_TYPES(MeshResource);

        Asset asset;
        MeshResource::sToken allocated_resource;
    };

    struct FreeEvent
    {
        RESOURCE_FreeEvent_DECLARE_TYPES(MeshResource);

        MeshResource::sToken allocated_resource;

        inline static FreeEvent build_from_token(const MeshResource::sToken p_resource)
        {
            return FreeEvent{p_resource};
        };
    };
};

struct ShaderResource
{
    struct Dependencies
    {
        ShaderModuleResource::sToken vertex_shader;
        ShaderModuleResource::sToken fragment_shader;
    };

    ResourceIdentifiedHeader header;
    ShaderIndex_Token shader;
    Dependencies dependencies;

    RESOURCE_DECLARE_TYPES(ShaderIndex);
    using sToken = iResource_Token<ShaderResource>::sToken;
    using sTokenValue = iResource_Token<ShaderResource>::sTokenValue;

    inline static ShaderResource build_from_id(const hash_t p_id)
    {
        return ShaderResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<ShaderIndex_TokenValue>(),
                              Dependencies{token_build_default<ShaderModuleResource::sTokenValue>(), token_build_default<ShaderModuleResource::sTokenValue>()}};
    };

    struct Asset
    {
        Span<int8> allocated_binary;

        struct Value
        {
            Slice<ShaderLayoutParameterType> specific_parameters;
            uimax execution_order;
            ShaderConfiguration shader_configuration;

            inline static Value build_from_asset(const Asset& p_asset)
            {
                Value l_value;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                l_value.specific_parameters = slice_cast<ShaderLayoutParameterType>(l_deserializer.slice());
                l_value.execution_order = *l_deserializer.type<uimax>();
                l_value.shader_configuration = *l_deserializer.type<ShaderConfiguration>();
                return l_value;
            };

            inline uimax binary_size() const
            {
                return BinarySize::slice(this->specific_parameters.build_asint8()) + sizeof(this->execution_order) + sizeof(this->shader_configuration);
            };
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_values)
        {
            Span<int8> l_binary_buffer = Span<int8>::allocate(p_values.binary_size());
            VectorSlice<int8> l_binary = VectorSlice<int8>::build(l_binary_buffer.slice, 0);
            BinarySerializer::slice(iVector<VectorSlice<int8>>{l_binary}, p_values.specific_parameters.build_asint8());
            BinarySerializer::type(iVector<VectorSlice<int8>>{l_binary}, p_values.execution_order);
            BinarySerializer::type(iVector<VectorSlice<int8>>{l_binary}, p_values.shader_configuration);
            return build_from_binary(l_binary_buffer);
        };

        inline void free()
        {
            this->allocated_binary.free();
        }
    };

    struct AssetDependencies
    {
        Span<int8> allocated_binary;

        struct Value
        {
            hash_t vertex_module;
            hash_t fragment_module;

            inline static Value build_from_binarydeserializer(BinaryDeserializer& p_deserializer)
            {
                Value l_return;
                l_return.vertex_module = *p_deserializer.type<hash_t>();
                l_return.fragment_module = *p_deserializer.type<hash_t>();
                return l_return;
            };

            inline static Value build_from_asset(const AssetDependencies& p_dependencies)
            {
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_dependencies.allocated_binary.slice);
                return build_from_binarydeserializer(l_deserializer);
            };

            inline uimax binary_size() const
            {
                return sizeof(this->vertex_module) + sizeof(this->fragment_module);
            };

            template <class BufferContainer> inline void push_to_binary_buffer(iVector<BufferContainer> in_out_buffer) const
            {
                iVector<BufferContainer>::Assert::element_type<int8>();

                BinarySerializer::type(in_out_buffer, this->vertex_module);
                BinarySerializer::type(in_out_buffer, this->fragment_module);
            };
        };

        inline static AssetDependencies allocate_from_values(const Value& p_values)
        {
            Span<int8> l_binary_buffer = Span<int8>::allocate(p_values.binary_size());
            VectorSlice<int8> l_binary = VectorSlice<int8>::build(l_binary_buffer.slice, 0);
            p_values.push_to_binary_buffer(iVector<VectorSlice<int8>>{l_binary});
            return AssetDependencies{l_binary_buffer};
        };

        inline void free()
        {
            this->allocated_binary.free();
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct DatabaseAllocationEvent
    {
        RESOURCE_DatabaseAllocationEvent_DECLARE_TYPES(ShaderResource);

        hash_t id;
        ShaderResource::sToken allocated_resource;
    };

    struct InlineAllocationEvent
    {
        RESOURCE_InlineAllocationEvent_DECLARE_TYPES(ShaderResource);

        ShaderResource::Asset asset;
        ShaderResource::sToken allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct FreeEvent
    {
        RESOURCE_FreeEvent_DECLARE_TYPES(ShaderResource);

        ShaderResource::sToken allocated_resource;

        inline static ShaderResource::FreeEvent build_from_token(const ShaderResource::sToken p_token)
        {
            return ShaderResource::FreeEvent{p_token};
        };
    };
};

struct TextureResource
{
    ResourceIdentifiedHeader header;
    TextureGPU_Token resource;

    RESOURCE_DECLARE_TYPES(TextureGPU);
    using sToken = iResource_Token<TextureResource>::sToken;
    using sTokenValue = iResource_Token<TextureResource>::sTokenValue;

    struct Asset
    {
        Span<int8> allocated_binary;

        struct Value
        {
            v3ui size;
            int8 channel_nb;
            Slice<int8> pixels;

            inline static Value build_from_asset(const Asset& p_asset)
            {
                Value l_value;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                l_value.size = *l_deserializer.type<v3ui>();
                l_value.channel_nb = *l_deserializer.type<int8>();
                l_value.pixels = l_deserializer.slice();
                return l_value;
            };

            inline uimax binary_size() const
            {
                return sizeof(this->size) + sizeof(this->channel_nb) + BinarySize::slice(this->pixels);
            };
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_value)
        {
            Span<int8> l_binary_buffer = Span<int8>::allocate(p_value.binary_size());
            VectorSlice<int8> l_binary = VectorSlice<int8>::build(l_binary_buffer.slice, 0);
            BinarySerializer::type(iVector<VectorSlice<int8>>{l_binary}, p_value.size);
            BinarySerializer::type(iVector<VectorSlice<int8>>{l_binary}, p_value.channel_nb);
            BinarySerializer::slice(iVector<VectorSlice<int8>>{l_binary}, p_value.pixels);
            return build_from_binary(l_binary_buffer);
        };

        inline void free()
        {
            this->allocated_binary.free();
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct DatabaseAllocationEvent
    {
        RESOURCE_DatabaseAllocationEvent_DECLARE_TYPES(TextureResource);

        hash_t id;
        TextureResource::sToken allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct InlineAllocationEvent
    {
        RESOURCE_InlineAllocationEvent_DECLARE_TYPES(TextureResource);

        Asset asset;
        TextureResource::sToken allocated_resource;
    };

    struct FreeEvent
    {
        RESOURCE_FreeEvent_DECLARE_TYPES(TextureResource);

        TextureResource::sToken allocated_resource;
    };
};

struct MaterialResource
{
    RESOURCE_DECLARE_TYPES(Material);
    using sToken = iResource_Token<MaterialResource>::sToken;
    using sTokenValue = iResource_Token<MaterialResource>::sTokenValue;

    struct DynamicDependency
    {
        TextureResource::sToken dependency;
    };

    struct Dependencies
    {
        ShaderResource::sToken shader;
        PoolOfVector<MaterialResource::DynamicDependency>::sToken dynamic_dependencies;
    };

    ResourceIdentifiedHeader header;
    Material_Token material;
    Dependencies dependencies;

    static MaterialResource build_from_id(const hash_t p_id)
    {
        return MaterialResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<Material_TokenValue>()};
    };

    struct Asset
    {
        Span<int8> allocated_binary;

        struct Value
        {
            struct Parameters
            {
                VaryingSlice parameters;

                inline ShaderParameter::Type get_parameter_type(const uimax p_index) const
                {
                    return *(ShaderParameter::Type*)this->parameters.get_element(p_index).Begin;
                };

                inline hash_t* get_parameter_texture_gpu_value(const uimax p_index)
                {
#if __DEBUG
                    assert_true(this->get_parameter_type(p_index) == ShaderParameter::Type::TEXTURE_GPU);
#endif
                    return slice_cast<hash_t>(this->parameters.get_element(p_index).slide_rv(sizeof(ShaderParameter::Type))).Begin;
                };

                inline const hash_t* get_parameter_texture_gpu_value(const uimax p_index) const
                {
                    return ((Parameters*)this)->get_parameter_texture_gpu_value(p_index);
                }

                inline Slice<int8> get_parameter_uniform_host_value(const uimax p_index)
                {
#if __DEBUG
                    assert_true(this->get_parameter_type(p_index) == ShaderParameter::Type::UNIFORM_HOST);
#endif
                    return this->parameters.get_element(p_index).slide_rv(sizeof(ShaderParameter::Type));
                };

                inline const Slice<int8> get_parameter_uniform_host_value(const uimax p_index) const
                {
                    return ((Parameters*)this)->get_parameter_uniform_host_value(p_index);
                };

                inline static void add_parameter_texture(VaryingVector& p_parameters, const hash_t p_texture_hash)
                {
                    ShaderParameter::Type l_type = ShaderParameter::Type::TEXTURE_GPU;
                    p_parameters.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_type), Slice<hash_t>::build_asint8_memory_singleelement(&p_texture_hash));
                };

                inline static void add_parameter_hostbuffer(VaryingVector& p_parameters, const Slice<int8>& p_buffer)
                {
                    ShaderParameter::Type l_type = ShaderParameter::Type::UNIFORM_HOST;
                    p_parameters.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_type), p_buffer);
                };
            };

            Parameters parameters;

            inline static Value build_from_asset(const Asset& p_asset)
            {
                Value l_value;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                l_value.parameters.parameters = l_deserializer.varying_slice();
                return l_value;
            };

            inline uimax binary_size() const
            {
                return BinarySize::varying_slice(this->parameters.parameters);
            };
        };

        inline void free()
        {
            this->allocated_binary.free();
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_value)
        {
            Span<int8> l_binary_buffer = Span<int8>::allocate(p_value.binary_size());
            VectorSlice<int8> l_binary = VectorSlice<int8>::build(l_binary_buffer.slice, 0);
            BinarySerializer::varying_slice(iVector<VectorSlice<int8>>{l_binary}, p_value.parameters.parameters);
            return build_from_binary(l_binary_buffer);
        };
    };

    struct AssetDependencies
    {
        Span<int8> allocated_binary;

        struct Value
        {
            hash_t shader;
            ShaderResource::AssetDependencies::Value shader_dependencies;
            Slice<hash_t> textures;

            inline static Value build_from_binarydeserializer(BinaryDeserializer& p_deserializer)
            {
                Value l_value;
                l_value.shader = *p_deserializer.type<hash_t>();
                l_value.shader_dependencies = ShaderResource::AssetDependencies::Value::build_from_binarydeserializer(p_deserializer);
                l_value.textures = slice_cast<hash_t>(p_deserializer.slice());
                return l_value;
            };

            inline static Value build_from_asset(const AssetDependencies& p_asset)
            {
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                return build_from_binarydeserializer(l_deserializer);
            };

            template <class BufferContainer> inline void push_to_binary_buffer(iVector<BufferContainer> in_out_buffer) const
            {
                iVector<BufferContainer>::Assert::element_type<int8>();

                BinarySerializer::type(in_out_buffer, this->shader);
                BinarySerializer::type(in_out_buffer, this->shader_dependencies);
                BinarySerializer::slice(in_out_buffer, this->textures.build_asint8());
            };

            inline uimax binary_size() const
            {
                return sizeof(this->shader) + sizeof(this->shader_dependencies) + BinarySize::slice(this->textures.build_asint8());
            };
        };

        inline void free()
        {
            this->allocated_binary.free();
        };

        inline static AssetDependencies build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return AssetDependencies{p_allocated_binary};
        };

        inline static AssetDependencies allocate_from_values(const Value& p_value)
        {
            Span<int8> l_binary_buffer = Span<int8>::allocate(p_value.binary_size());
            VectorSlice<int8> l_binary = VectorSlice<int8>::build(l_binary_buffer.slice, 0);
            p_value.push_to_binary_buffer(iVector<VectorSlice<int8>>{l_binary});
            return build_from_binary(l_binary_buffer);
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
        Slice<TextureResource::DatabaseAllocationInput> texture_dependencies_input;
    };

    struct DatabaseAllocationEvent
    {
        RESOURCE_DatabaseAllocationEvent_DECLARE_TYPES(MaterialResource);

        hash_t id;
        MaterialResource::sToken allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
        Slice<TextureResource::InlineAllocationInput> texture_dependencies_input;
    };

    struct InlineAllocationEvent
    {
        RESOURCE_InlineAllocationEvent_DECLARE_TYPES(MaterialResource);

        Asset asset;
        MaterialResource::sToken allocated_resource;
    };

    struct FreeEvent
    {
        RESOURCE_FreeEvent_DECLARE_TYPES(MaterialResource);

        MaterialResource::sToken allocated_resource;
    };
};