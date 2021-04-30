#pragma once

enum class ResourceAllocationType
{
    UNKNOWN = 0,
    INLINE = 1,
    ASSET_DATABASE = 2
};

struct ResourceIdentifiedHeader
{
    ResourceAllocationType allocation_type;
    int8 allocated;
    hash_t id;

    inline static ResourceIdentifiedHeader build_inline_with_id(const hash_t p_id)
    {
        return ResourceIdentifiedHeader{ResourceAllocationType::INLINE, 0, p_id};
    };

    inline static ResourceIdentifiedHeader build_database_with_id(const hash_t p_id)
    {
        return ResourceIdentifiedHeader{ResourceAllocationType::ASSET_DATABASE, 0, p_id};
    };
};

struct ShaderModuleResource
{
    ResourceIdentifiedHeader header;
    Token<ShaderModule> shader_module;

    inline static ShaderModuleResource build_inline_from_id(const hash_t p_id)
    {
        return ShaderModuleResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<ShaderModule>()};
    };

    inline static ShaderModuleResource build_database_from_id(const hash_t p_id)
    {
        return ShaderModuleResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<ShaderModule>()};
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
        hash_t id;
        Token<ShaderModuleResource> allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct InlineAllocationEvent
    {
        Asset asset;
        Token<ShaderModuleResource> allocated_resource;
    };

    struct FreeEvent
    {
        Token<ShaderModuleResource> allocated_resource;

        inline static FreeEvent build_from_token(const Token<ShaderModuleResource> p_token)
        {
            return FreeEvent{p_token};
        };
    };
};

struct MeshResource
{
    ResourceIdentifiedHeader header;
    Token<Mesh> mesh;

    inline static MeshResource build_inline_from_id(const hash_t p_id)
    {
        return MeshResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<Mesh>()};
    };

    inline static MeshResource build_database_from_id(const hash_t p_id)
    {
        return MeshResource{ResourceIdentifiedHeader::build_database_with_id(p_id), token_build_default<Mesh>()};
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
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_values)
        {
            Vector<int8> l_binary = Vector<int8>::allocate(0);
            BinarySerializer::slice(&l_binary, p_values.initial_vertices.build_asint8());
            BinarySerializer::slice(&l_binary, p_values.initial_indices.build_asint8());
            return build_from_binary(l_binary.Memory);
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct DatabaseAllocationEvent
    {
        hash_t id;
        Token<MeshResource> allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct InlineAllocationEvent
    {
        MeshResource::Asset asset;
        Token<MeshResource> allocated_resource;
    };

    struct FreeEvent
    {
        Token<MeshResource> allocated_resource;

        inline static FreeEvent build_from_token(const Token<MeshResource> p_resource)
        {
            return FreeEvent{p_resource};
        };
    };
};

struct ShaderResource
{
    struct Dependencies
    {
        Token<ShaderModuleResource> vertex_shader;
        Token<ShaderModuleResource> fragment_shader;
    };

    ResourceIdentifiedHeader header;
    Token<ShaderIndex> shader;
    Dependencies dependencies;

    inline static ShaderResource build_from_id(const hash_t p_id)
    {
        return ShaderResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<ShaderIndex>(),
                               Dependencies{token_build_default<ShaderModuleResource>(), token_build_default<ShaderModuleResource>()}};
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
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_values)
        {
            Vector<int8> l_binary = Vector<int8>::allocate(0);
            BinarySerializer::slice(&l_binary, p_values.specific_parameters.build_asint8());
            BinarySerializer::type(&l_binary, p_values.execution_order);
            BinarySerializer::type(&l_binary, p_values.shader_configuration);
            return build_from_binary(l_binary.Memory);
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

            inline void push_to_binary_buffer(Vector<int8>* in_out_buffer) const
            {
                BinarySerializer::type(in_out_buffer, this->vertex_module);
                BinarySerializer::type(in_out_buffer, this->fragment_module);
            };
        };

        inline static AssetDependencies allocate_from_values(const Value& p_values)
        {
            Vector<int8> l_binary = Vector<int8>::allocate(0);
            p_values.push_to_binary_buffer(&l_binary);
            return AssetDependencies{l_binary.Memory};
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
        hash_t id;
        Token<ShaderResource> allocated_resource;
    };

    struct InlineAllocationEvent
    {
        ShaderResource::Asset asset;
        Token<ShaderResource> allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct FreeEvent
    {
        Token<ShaderResource> allocated_resource;

        inline static ShaderResource::FreeEvent build_from_token(const Token<ShaderResource> p_token)
        {
            return ShaderResource::FreeEvent{p_token};
        };
    };
};

struct TextureResource
{
    ResourceIdentifiedHeader header;
    Token<TextureGPU> texture;

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
        };

        inline static Asset build_from_binary(const Span<int8>& p_allocated_binary)
        {
            return Asset{p_allocated_binary};
        };

        inline static Asset allocate_from_values(const Value& p_value)
        {
            Vector<int8> l_binary = Vector<int8>::allocate(0);
            BinarySerializer::type(&l_binary, p_value.size);
            BinarySerializer::type(&l_binary, p_value.channel_nb);
            BinarySerializer::slice(&l_binary, p_value.pixels);
            return build_from_binary(l_binary.Memory);
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
        hash_t id;
        Token<TextureResource> allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct InlineAllocationEvent
    {
        Asset asset;
        Token<TextureResource> allocated_resource;
    };

    struct FreeEvent
    {
        Token<TextureResource> allocated_resource;
    };
};

struct MaterialResource
{
    struct DynamicDependency
    {
        Token<TextureResource> dependency;
    };

    struct Dependencies
    {
        Token<ShaderResource> shader;
        Token<Slice<MaterialResource::DynamicDependency>> dynamic_dependencies;
    };

    ResourceIdentifiedHeader header;
    Token<Material> material;
    Dependencies dependencies;

    static MaterialResource build_from_id(const hash_t p_id)
    {
        return MaterialResource{ResourceIdentifiedHeader::build_inline_with_id(p_id), token_build_default<Material>()};
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
            Vector<int8> l_binary = Vector<int8>::allocate(0);
            BinarySerializer::varying_slice(&l_binary, p_value.parameters.parameters);
            return build_from_binary(l_binary.Memory);
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

            inline void push_to_binary_buffer(Vector<int8>* in_out_buffer) const
            {
                BinarySerializer::type(in_out_buffer, this->shader);
                BinarySerializer::type(in_out_buffer, this->shader_dependencies);
                BinarySerializer::slice(in_out_buffer, this->textures.build_asint8());
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
            Vector<int8> l_binary = Vector<int8>::allocate(0);
            p_value.push_to_binary_buffer(&l_binary);
            return build_from_binary(l_binary.Memory);
        };
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
        Slice<TextureResource::DatabaseAllocationInput> texture_dependencies_input;
    };

    struct DatabaseAllocationEvent
    {
        hash_t id;
        Token<MaterialResource> allocated_resource;
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
        Slice<TextureResource::InlineAllocationInput> texture_dependencies_input;
    };

    struct InlineAllocationEvent
    {
        Asset asset;
        Token<MaterialResource> allocated_resource;
    };

    struct FreeEvent
    {
        Token<MaterialResource> allocated_resource;
    };
};