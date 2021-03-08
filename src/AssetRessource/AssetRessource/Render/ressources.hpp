#pragma once

namespace v2
{
#define RessourceAllocationEvent_member_allocated_ressource allocated_ressource
#define RessourceAllocationEvent_member_asset asset

enum class RessourceAllocationType
{
    UNKNOWN = 0,
    INLINE = 1,
    ASSET_DATABASE = 2
};

struct RessourceIdentifiedHeader
{
    RessourceAllocationType allocation_type;
    int8 allocated;
    hash_t id;

    inline static RessourceIdentifiedHeader build_inline_with_id(const hash_t p_id)
    {
        return RessourceIdentifiedHeader{RessourceAllocationType::INLINE, 0, p_id};
    };

    inline static RessourceIdentifiedHeader build_database_with_id(const hash_t p_id)
    {
        return RessourceIdentifiedHeader{RessourceAllocationType::ASSET_DATABASE, 0, p_id};
    };
};

struct ShaderModuleRessource
{
    RessourceIdentifiedHeader header;
    Token(ShaderModule) shader_module;

    inline static ShaderModuleRessource build_inline_from_id(const hash_t p_id)
    {
        return ShaderModuleRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id), tk_bd(ShaderModule)};
    };

    inline static ShaderModuleRessource build_database_from_id(const hash_t p_id)
    {
        return ShaderModuleRessource{RessourceIdentifiedHeader::build_database_with_id(p_id), tk_bd(ShaderModule)};
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

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct AllocationEvent
    {
        Asset RessourceAllocationEvent_member_asset;
        Token(ShaderModuleRessource) RessourceAllocationEvent_member_allocated_ressource;
    };

    struct FreeEvent
    {
        Token(ShaderModuleRessource) ressource;

        inline static FreeEvent build_from_token(const Token(ShaderModuleRessource) p_token)
        {
            return FreeEvent{p_token};
        };
    };
};

struct MeshRessource
{
    RessourceIdentifiedHeader header;
    Token(Mesh) mesh;

    inline static MeshRessource build_inline_from_id(const hash_t p_id)
    {
        return MeshRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id), tk_bd(Mesh)};
    };

    inline static MeshRessource build_database_from_id(const hash_t p_id)
    {
        return MeshRessource{RessourceIdentifiedHeader::build_database_with_id(p_id), tk_bd(Mesh)};
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

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct AllocationEvent
    {
        MeshRessource::Asset RessourceAllocationEvent_member_asset;
        Token(MeshRessource) RessourceAllocationEvent_member_allocated_ressource;
    };

    struct FreeEvent
    {
        Token(MeshRessource) ressource;

        inline static FreeEvent build_from_token(const Token(MeshRessource) p_ressource)
        {
            return FreeEvent{p_ressource};
        };
    };
};

struct ShaderRessource
{
    struct Dependencies
    {
        Token(ShaderModuleRessource) vertex_shader;
        Token(ShaderModuleRessource) fragment_shader;
    };

    RessourceIdentifiedHeader header;
    Token(ShaderIndex) shader;
    Dependencies dependencies;

    inline static ShaderRessource build_from_id(const hash_t p_id)
    {
        return ShaderRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id), tk_bd(ShaderIndex), Dependencies{tk_bd(ShaderModuleRessource), tk_bd(ShaderModuleRessource)}};
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

            inline static Value build_from_asset(const AssetDependencies& p_dependencies)
            {
                Value l_return;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_dependencies.allocated_binary.slice);
                l_return.vertex_module = *l_deserializer.type<hash_t>();
                l_return.fragment_module = *l_deserializer.type<hash_t>();
                return l_return;
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

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct AllocationEvent
    {
        ShaderRessource::Asset RessourceAllocationEvent_member_asset;
        Token(ShaderRessource) RessourceAllocationEvent_member_allocated_ressource;
    };

    struct FreeEvent
    {
        Token(ShaderRessource) ressource;

        inline static ShaderRessource::FreeEvent build_from_token(const Token(ShaderRessource) p_token)
        {
            return ShaderRessource::FreeEvent{p_token};
        };
    };
};

struct TextureRessource
{
    RessourceIdentifiedHeader header;
    Token(TextureGPU) texture;

    struct Asset
    {
        Span<int8> allocated_binary;

        struct Value
        {
            v3ui size;
            Slice<int8> pixels;

            inline static Value build_from_asset(const Asset& p_asset)
            {
                Value l_value;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                l_value.size = *l_deserializer.type<v3ui>();
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
            BinarySerializer::slice(&l_binary, p_value.pixels);
            return build_from_binary(l_binary.Memory);
        };

        inline void free()
        {
            this->allocated_binary.free();
        };
    };

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
    };

    struct AllocationEvent
    {
        Asset RessourceAllocationEvent_member_asset;
        Token(TextureRessource) RessourceAllocationEvent_member_allocated_ressource;
    };

    struct FreeEvent
    {
        Token(TextureRessource) ressource;
    };
};

struct MaterialRessource
{
    struct DynamicDependency
    {
        Token(TextureRessource) dependency;
    };

    struct Dependencies
    {
        Token(ShaderRessource) shader;
        Token(Slice<MaterialRessource::DynamicDependency>) dynamic_dependencies;
    };

    RessourceIdentifiedHeader header;
    Token(Material) material;
    Dependencies dependencies;

    static MaterialRessource build_from_id(const hash_t p_id)
    {
        return MaterialRessource{RessourceIdentifiedHeader::build_inline_with_id(p_id), tk_bd(Material)};
    };

    struct Asset
    {
        Span<int8> allocated_binary;

        struct Value
        {
            struct Parameters
            {
                VaryingSlice parameters;

                inline ShaderParameter::Type get_parameter_type(const uimax p_index)
                {
                    return *(ShaderParameter::Type*)this->parameters.get_element(p_index).Begin;
                };

                inline hash_t* get_parameter_texture_gpu_value(const uimax p_index)
                {
#if RENDER_BOUND_TEST
                    assert_true(this->get_parameter_type(p_index) == ShaderParameter::Type::TEXTURE_GPU);
#endif
                    return slice_cast<hash_t>(this->parameters.get_element(p_index).slide_rv(sizeof(ShaderParameter::Type))).Begin;
                };

                inline Slice<int8> get_parameter_uniform_host_value(const uimax p_index)
                {
#if RENDER_BOUND_TEST
                    assert_true(this->get_parameter_type(p_index) == ShaderParameter::Type::UNIFORM_HOST);
#endif
                    return this->parameters.get_element(p_index).slide_rv(sizeof(ShaderParameter::Type));
                };

                inline static void add_parameter_texture(VaryingVector& p_parameters, const hash_t p_texture_hash)
                {
                    ShaderParameter::Type l_type = ShaderParameter::Type::TEXTURE_GPU;
                    p_parameters.push_back_2(Slice<ShaderParameter::Type>::build_asint8_memory_singleelement(&l_type), Slice<hash_t>::build_asint8_memory_singleelement(&p_texture_hash));
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
            ShaderRessource::AssetDependencies::Value shader_dependencies;
            Slice<hash_t> textures;

            inline static Value build_from_asset(const AssetDependencies& p_asset)
            {
                Value l_value;
                BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
                l_value.shader = *l_deserializer.type<hash_t>();
                // TODO -> a function from shader must be called
                l_value.shader_dependencies.vertex_module = *l_deserializer.type<hash_t>();
                l_value.shader_dependencies.fragment_module = *l_deserializer.type<hash_t>();
                l_value.textures = slice_cast<hash_t>(l_deserializer.slice());
                return l_value;
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

    struct InlineAllocationInput
    {
        hash_t id;
        Asset asset;
        Slice<TextureRessource::InlineAllocationInput> texture_dependencies_input;
    };

    struct DatabaseAllocationInput
    {
        hash_t id;
        Slice<TextureRessource::DatabaseAllocationInput> texture_dependencies_input;
    };

    struct AllocationEvent
    {
        Asset RessourceAllocationEvent_member_asset;
        Token(MaterialRessource) RessourceAllocationEvent_member_allocated_ressource;
    };

    struct FreeEvent
    {
        Token(MaterialRessource) ressource;
    };
};

} // namespace v2