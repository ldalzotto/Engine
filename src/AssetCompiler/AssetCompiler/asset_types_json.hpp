#pragma once

inline Span<int8> AssetCompiler_open_and_read_asset_file(const Slice<int8>& p_asset_root_path, const Slice<int8>& p_relative_asset_path)
{
    Span<int8> l_asset_full_path = Span<int8>::allocate_slice_3(p_asset_root_path, p_relative_asset_path, Slice_build_begin_end<int8>("\0", 0, 1));
    File l_asset_file = File::open(l_asset_full_path.slice);
    Span<int8> l_asset_file_content = l_asset_file.read_file_allocate();
    l_asset_file.free();
    l_asset_full_path.free();
    return l_asset_file_content;
};

enum class AssetType
{
    UNDEFINED = 0,
    SHADER_MODULE = UNDEFINED + 1,
    SHADER = SHADER_MODULE + 1,
    MESH = SHADER + 1,
    TEXTURE = MESH + 1,
    MATERIAL = TEXTURE + 1
};

namespace AssetType_Const
{
static const Slice<int8> SHADER_MODULE_NAME = Slice_int8_build_rawstr("SHADER_MODULE");
static const Slice<int8> SHADER_NAME = Slice_int8_build_rawstr("SHADER");
static const Slice<int8> MATERIAL_NAME = Slice_int8_build_rawstr("MATERIAL");
static const Slice<int8> MESH_NAME = Slice_int8_build_rawstr("MESH");
static const Slice<int8> TEXTURE_NAME = Slice_int8_build_rawstr("TEXTURE");
}; // namespace AssetType_Const

inline const Slice<int8> AssetType_getName(const AssetType p_asset_type)
{
    switch (p_asset_type)
    {
    case AssetType::SHADER_MODULE:
        return AssetType_Const::SHADER_MODULE_NAME;
    case AssetType::SHADER:
        return AssetType_Const::SHADER_NAME;
    case AssetType::MATERIAL:
        return AssetType_Const::MATERIAL_NAME;
    case AssetType::MESH:
        return AssetType_Const::MESH_NAME;
    case AssetType::TEXTURE:
        return AssetType_Const::TEXTURE_NAME;
    default:
        abort();
    }
};

namespace AssetJSON_Const
{
const Slice<int8> SHADER = Slice_int8_build_rawstr("SHADER");
const Slice<int8> MATERIAL = Slice_int8_build_rawstr("MATERIAL");
} // namespace AssetJSON_Const

struct AssetJSON
{
    inline static AssetType get_value_of_asset_json(JSONDeserializer* p_json_deserializer, JSONDeserializer* out_val_deserializer)
    {
        p_json_deserializer->next_field("type");
        AssetType l_type = AssetType::UNDEFINED;
        if (Slice_compare(&p_json_deserializer->get_currentfield().value, &AssetJSON_Const::SHADER))
        {
            l_type = AssetType::SHADER;
        }
        else if (Slice_compare(&p_json_deserializer->get_currentfield().value, &AssetJSON_Const::MATERIAL))
        {
            l_type = AssetType::MATERIAL;
        }

        *out_val_deserializer = JSONDeserializer::allocate_default();
        p_json_deserializer->next_object("val", out_val_deserializer);
        return l_type;
    };

    inline static void move_json_deserializer_to_value(JSONDeserializer* p_json_deserializer, JSONDeserializer* out_value_json_deserializer)
    {
        p_json_deserializer->next_field("type");
        *out_value_json_deserializer = JSONDeserializer::allocate_default();
        p_json_deserializer->next_object("val", out_value_json_deserializer);
    };
};

namespace ShaderAssetJSON_Const
{
const Slice<int8> TEXTURE_FRAGMENT = Slice_int8_build_rawstr("TEXTURE_FRAGMENT");
const Slice<int8> UNIFORM_BUFFER_VERTEX_FRAGMENT = Slice_int8_build_rawstr("UNIFORM_BUFFER_VERTEX_FRAGMENT");
const Slice<int8> UNIFORM_BUFFER_VERTEX = Slice_int8_build_rawstr("UNIFORM_BUFFER_VERTEX");

const Slice<int8> Always = Slice_int8_build_rawstr("Always");
const Slice<int8> LessOrEqual = Slice_int8_build_rawstr("LessOrEqual");
const Slice<int8> Greater = Slice_int8_build_rawstr("Greater");
const Slice<int8> NotEqual = Slice_int8_build_rawstr("NotEqual");
const Slice<int8> GreaterOrEqual = Slice_int8_build_rawstr("GreaterOrEqual");
const Slice<int8> Invalid = Slice_int8_build_rawstr("Invalid");
}; // namespace ShaderAssetJSON_Const

struct ShaderAssetJSON
{
    inline static ShaderLayoutParameterType slice_to_shaderlayoutparamtertype(const Slice<int8>& p_slice)
    {
        if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::TEXTURE_FRAGMENT))
        {
            return ShaderLayoutParameterType::TEXTURE_FRAGMENT;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::UNIFORM_BUFFER_VERTEX_FRAGMENT))
        {
            return ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::UNIFORM_BUFFER_VERTEX))
        {
            return ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX;
        }
        else
        {
            abort();
        }
    };

    inline static ShaderConfiguration::CompareOp slice_to_compareop(const Slice<int8>& p_slice)
    {
        if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::Always))
        {
            return ShaderConfiguration::CompareOp::Always;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::LessOrEqual))
        {
            return ShaderConfiguration::CompareOp::LessOrEqual;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::Greater))
        {
            return ShaderConfiguration::CompareOp::Greater;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::NotEqual))
        {
            return ShaderConfiguration::CompareOp::NotEqual;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::GreaterOrEqual))
        {
            return ShaderConfiguration::CompareOp::GreaterOrEqual;
        }
        else if (Slice_compare(&p_slice, &ShaderAssetJSON_Const::Invalid))
        {
            return ShaderConfiguration::CompareOp::Invalid;
        }
        else
        {
            abort();
        }
    };

    inline static Span<int8> allocate_asset_from_json(JSONDeserializer* p_json_deserializer)
    {
        ShaderRessource::Asset::Value l_shader_ressource_value{};
        Vector<ShaderLayoutParameterType> l_shader_layout_parameter_types_deserialized = Vector<ShaderLayoutParameterType>::allocate(0);

        p_json_deserializer->next_field("execution_order");
        l_shader_ressource_value.execution_order = FromString::auimax(p_json_deserializer->get_currentfield().value);
        p_json_deserializer->next_field("vertex");
        // l_shader_ressource_dependency_value.vertex_module = HashSlice(p_json_deserializer->get_currentfield().value);
        p_json_deserializer->next_field("fragment");
        // l_shader_ressource_dependency_value.fragment_module = HashSlice(p_json_deserializer->get_currentfield().value);

        JSONDeserializer l_layout_deserializer = JSONDeserializer::allocate_default();
        if (p_json_deserializer->next_object("layout", &l_layout_deserializer))
        {
            JSONDeserializer l_layout_parameters_deserializer = JSONDeserializer::allocate_default();
            if (l_layout_deserializer.next_array("parameters", &l_layout_parameters_deserializer))
            {
                JSONDeserializer l_layout_parameter_deserializer = JSONDeserializer::allocate_default();
                while (l_layout_parameters_deserializer.next_array_object(&l_layout_parameter_deserializer))
                {
                    if (l_layout_parameter_deserializer.next_field("type"))
                    {
                        l_shader_layout_parameter_types_deserialized.push_back_element(slice_to_shaderlayoutparamtertype(l_layout_parameter_deserializer.get_currentfield().value));
                    };
                }
                l_layout_parameter_deserializer.free();
            }
            l_layout_parameters_deserializer.free();
        }
        l_shader_ressource_value.specific_parameters = l_shader_layout_parameter_types_deserialized.to_slice();

        if (p_json_deserializer->next_field("ztest"))
        {
            l_shader_ressource_value.shader_configuration.ztest = slice_to_compareop(p_json_deserializer->get_currentfield().value);
        }

        p_json_deserializer->next_field("zwrite");
        l_shader_ressource_value.shader_configuration.zwrite = FromString::auint8(p_json_deserializer->get_currentfield().value);

        ShaderRessource::Asset l_asset = ShaderRessource::Asset::allocate_from_values(l_shader_ressource_value);

        l_layout_deserializer.free();
        l_shader_layout_parameter_types_deserialized.free();

        return l_asset.allocated_binary;
    };

    inline static void push_dependencies_from_json_to_buffer(JSONDeserializer* p_json_deserializer, Vector<int8>* in_out_buffer)
    {
        ShaderRessource::AssetDependencies::Value l_shader_dependencies_value{};

        p_json_deserializer->next_field("execution_order");
        p_json_deserializer->next_field("vertex");
        l_shader_dependencies_value.vertex_module = HashSlice(p_json_deserializer->get_currentfield().value);
        p_json_deserializer->next_field("fragment");
        l_shader_dependencies_value.fragment_module = HashSlice(p_json_deserializer->get_currentfield().value);

        l_shader_dependencies_value.push_to_binary_buffer(in_out_buffer);
    };

    inline static ShaderRessource::AssetDependencies allocate_dependencies_from_json(JSONDeserializer* p_json_deserializer)
    {
        Vector<int8> l_binary = Vector<int8>::allocate(0);
        push_dependencies_from_json_to_buffer(p_json_deserializer, &l_binary);
        return ShaderRessource::AssetDependencies{l_binary.Memory};
    };
};

namespace MaterialAssetJSON_Const
{
const Slice<int8> UNIFORM_HOST = Slice_int8_build_rawstr("UNIFORM_HOST");
const Slice<int8> UNIFORM_GPU = Slice_int8_build_rawstr("UNIFORM_GPU");
const Slice<int8> TEXTURE_GPU = Slice_int8_build_rawstr("TEXTURE_GPU");
}; // namespace MaterialAssetJSON_Const

struct MaterialAssetJSON
{
    inline static ShaderParameter::Type slice_to_shaderparametertype(const Slice<int8>& p_slice)
    {
        if (Slice_compare(&p_slice, &MaterialAssetJSON_Const::UNIFORM_HOST))
        {
            return ShaderParameter::Type::UNIFORM_HOST;
        }
        else if (Slice_compare(&p_slice, &MaterialAssetJSON_Const::UNIFORM_GPU))
        {
            return ShaderParameter::Type::UNIFORM_GPU;
        }
        else if (Slice_compare(&p_slice, &MaterialAssetJSON_Const::TEXTURE_GPU))
        {
            return ShaderParameter::Type::TEXTURE_GPU;
        }
        else
        {
            abort();
        }
    };

    inline static Span<int8> allocate_asset_from_json(JSONDeserializer* p_json_deserializer)
    {
        VaryingVector l_material_parameters = VaryingVector::allocate_default();
        p_json_deserializer->next_field("shader");

        JSONDeserializer l_parameter_array_deserializer = JSONDeserializer::allocate_default();
        if (p_json_deserializer->next_array("parameters", &l_parameter_array_deserializer))
        {
            JSONDeserializer l_parameter_deserializer = JSONDeserializer::allocate_default();
            while (l_parameter_array_deserializer.next_array_object(&l_parameter_deserializer))
            {
                l_parameter_deserializer.next_field("type");
                Slice<int8> l_material_parameter_type = l_parameter_deserializer.get_currentfield().value;
                switch (slice_to_shaderparametertype(l_material_parameter_type))
                {
                case ShaderParameter::Type::TEXTURE_GPU:
                {
                    l_parameter_deserializer.next_field("val");
                    MaterialRessource::Asset::Value::Parameters::add_parameter_texture(l_material_parameters, HashSlice(l_parameter_deserializer.get_currentfield().value));
                }
                break;
                case ShaderParameter::Type::UNIFORM_HOST:
                {
                    JSONDeserializer l_uniform_host_obj_deserializer = JSONDeserializer::allocate_default();
                    if (l_parameter_deserializer.next_object("val", &l_uniform_host_obj_deserializer))
                    {
                        if (l_uniform_host_obj_deserializer.next_field("type"))
                        {
                            PrimitiveSerializedTypes::Type l_parameter_type = PrimitiveSerializedTypes::get_type_from_string(l_uniform_host_obj_deserializer.get_currentfield().value);
                            switch (l_parameter_type)
                            {
                            case PrimitiveSerializedTypes::Type::FLOAT32:
                            {
                                l_uniform_host_obj_deserializer.next_field("val");
                                float32 l_value = FromString::afloat32(l_uniform_host_obj_deserializer.get_currentfield().value);
                                MaterialRessource::Asset::Value::Parameters::add_parameter_hostbuffer(l_material_parameters, Slice_build_asint8_memory_singleelement<float32>(&l_value));
                            }
                            break;
                            case PrimitiveSerializedTypes::Type::FLOAT32_2:
                            {
                                JSONDeserializer l_value_deserializer = JSONDeserializer::allocate_default();
                                l_uniform_host_obj_deserializer.next_object("val", &l_value_deserializer);
                                v2f l_value = MathJSONDeserialization::_v2f(&l_value_deserializer);
                                MaterialRessource::Asset::Value::Parameters::add_parameter_hostbuffer(l_material_parameters, Slice_build_asint8_memory_singleelement<v2f>(&l_value));
                                l_value_deserializer.free();
                            }
                            break;
                            case PrimitiveSerializedTypes::Type::FLOAT32_3:
                            {
                                JSONDeserializer l_value_deserializer = JSONDeserializer::allocate_default();
                                l_uniform_host_obj_deserializer.next_object("val", &l_value_deserializer);
                                v3f l_value = MathJSONDeserialization::_v3f(&l_value_deserializer);
                                MaterialRessource::Asset::Value::Parameters::add_parameter_hostbuffer(l_material_parameters, Slice_build_asint8_memory_singleelement<v3f>(&l_value));
                                l_value_deserializer.free();
                            }
                            break;
                            case PrimitiveSerializedTypes::Type::FLOAT32_4:
                            {
                                JSONDeserializer l_value_deserializer = JSONDeserializer::allocate_default();
                                l_uniform_host_obj_deserializer.next_object("val", &l_value_deserializer);
                                v4f l_value = MathJSONDeserialization::_v4f(&l_value_deserializer);
                                MaterialRessource::Asset::Value::Parameters::add_parameter_hostbuffer(l_material_parameters, Slice_build_asint8_memory_singleelement<v4f>(&l_value));
                                l_value_deserializer.free();
                            }
                            break;
                            default:
                                abort();
                            }
                        }
                    }
                    l_uniform_host_obj_deserializer.free();
                }
                break;
                default:
                    abort();
                }
            }
            l_parameter_deserializer.free();
        }

        Span<int8> l_asset_allocated_binary = MaterialRessource::Asset::allocate_from_values(MaterialRessource::Asset::Value{l_material_parameters.to_varying_slice()}).allocated_binary;

        l_parameter_array_deserializer.free();
        l_material_parameters.free();

        return l_asset_allocated_binary;
    };

    inline static MaterialRessource::AssetDependencies allocate_dependencies_from_json(JSONDeserializer* p_json_deserializer, const Slice<int8>& p_root_path)
    {
        Vector<int8> l_binary = Vector<int8>::allocate(0);

        p_json_deserializer->next_field("shader");
        BinarySerializer::type(&l_binary, HashSlice(p_json_deserializer->get_currentfield().value));

        {
            Span<int8> l_shader_file_content = AssetCompiler_open_and_read_asset_file(p_root_path, p_json_deserializer->get_currentfield().value);
            Vector<int8> l_shader_file_content_vector = Vector<int8>{l_shader_file_content.Capacity, l_shader_file_content};
            JSONDeserializer l_shader_asset_deserializer = JSONDeserializer::start(l_shader_file_content_vector);
            JSONDeserializer l_shader_asset_value_deserializer;
            AssetJSON::move_json_deserializer_to_value(&l_shader_asset_deserializer, &l_shader_asset_value_deserializer);
            ShaderAssetJSON::push_dependencies_from_json_to_buffer(&l_shader_asset_value_deserializer, &l_binary);

            l_shader_asset_deserializer.free();
            l_shader_asset_value_deserializer.free();
            l_shader_file_content.free();
        }

        Vector<hash_t> l_textures = Vector<hash_t>::allocate(0);
        JSONDeserializer l_parameter_array_deserializer = JSONDeserializer::allocate_default();
        if (p_json_deserializer->next_array("parameters", &l_parameter_array_deserializer))
        {
            JSONDeserializer l_parameter_deserializer = JSONDeserializer::allocate_default();
            while (l_parameter_array_deserializer.next_array_object(&l_parameter_deserializer))
            {
                l_parameter_deserializer.next_field("type");
                Slice<int8> l_material_parameter_type = l_parameter_deserializer.get_currentfield().value;
                if (Slice_compare(&l_material_parameter_type, &MaterialAssetJSON_Const::TEXTURE_GPU))
                {
                    l_parameter_deserializer.next_field("val");
                    l_textures.push_back_element(HashSlice(l_parameter_deserializer.get_currentfield().value));
                }
            }
            l_parameter_deserializer.free();
        }
        l_parameter_array_deserializer.free();

        BinarySerializer::slice(&l_binary, Slice_build_asint8(&l_textures.Memory.slice));

        l_textures.free();

        return MaterialRessource::AssetDependencies{l_binary.Memory};
    };
};