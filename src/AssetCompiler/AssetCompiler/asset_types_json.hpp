#pragma once

inline File AssetCompiler_open_asset_file(const Slice<int8>& p_asset_root_path, const Slice<int8>& p_relative_asset_path)
{
    Span<int8> l_asset_full_path = Span<int8>::allocate_slice_3(p_asset_root_path, p_relative_asset_path, Slice<int8>::build("\0", 0, 1));
    File l_asset_file = File::open(l_asset_full_path.slice);
    return l_asset_file;
};

inline Span<int8> AssetCompiler_open_and_read_asset_file(const Slice<int8>& p_asset_root_path, const Slice<int8>& p_relative_asset_path)
{
    Span<int8> l_asset_full_path = Span<int8>::allocate_slice_3(p_asset_root_path, p_relative_asset_path, Slice<int8>::build("\0", 0, 1));
    File l_asset_file = File::open(l_asset_full_path.slice);
    Span<int8> l_asset_file_content = l_asset_file.read_file_allocate();
    l_asset_full_path.free();
    l_asset_file.free();
    return l_asset_file_content;
};

enum class AssetJSONTypes
{
    UNDEFINED = 0,
    SHADER = UNDEFINED + 1,
    MATERIAL = SHADER + 1
};

struct AssetJSON
{
    inline static AssetJSONTypes get_value_of_asset_json(JSONDeserializer* p_json_deserializer, JSONDeserializer* out_val_deserializer)
    {
        p_json_deserializer->next_field("type");
        AssetJSONTypes l_type = AssetJSONTypes::UNDEFINED;
        if (p_json_deserializer->get_currentfield().value.compare(slice_int8_build_rawstr("SHADER")))
        {
            l_type = AssetJSONTypes::SHADER;
        }
        else if (p_json_deserializer->get_currentfield().value.compare(slice_int8_build_rawstr("MATERIAL")))
        {
            l_type = AssetJSONTypes::MATERIAL;
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

struct ShaderAssetJSON
{
    inline static v2::ShaderLayoutParameterType slice_to_shaderlayoutparamtertype(const Slice<int8>& p_slice)
    {
        if (p_slice.compare(slice_int8_build_rawstr("TEXTURE_FRAGMENT")))
        {
            return v2::ShaderLayoutParameterType::TEXTURE_FRAGMENT;
        }
        else if(p_slice.compare(slice_int8_build_rawstr("UNIFORM_BUFFER_VERTEX")))
        {
            return v2::ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX;
        }
        else if(p_slice.compare(slice_int8_build_rawstr("UNIFORM_BUFFER_VERTEX_FRAGMENT")))
        {
            return v2::ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
        }
        else
        {
            abort();
        }
    };

    inline static v2::ShaderConfiguration::CompareOp slice_to_compareop(const Slice<int8>& p_slice)
    {
        if (p_slice.compare(slice_int8_build_rawstr("Always")))
        {
            return v2::ShaderConfiguration::CompareOp::Always;
        }
        else if (p_slice.compare(slice_int8_build_rawstr("LessOrEqual")))
        {
            return v2::ShaderConfiguration::CompareOp::LessOrEqual;
        }
        else if (p_slice.compare(slice_int8_build_rawstr("Greater")))
        {
            return v2::ShaderConfiguration::CompareOp::Greater;
        }
        else if (p_slice.compare(slice_int8_build_rawstr("NotEqual")))
        {
            return v2::ShaderConfiguration::CompareOp::NotEqual;
        }
        else if (p_slice.compare(slice_int8_build_rawstr("GreaterOrEqual")))
        {
            return v2::ShaderConfiguration::CompareOp::GreaterOrEqual;
        }
        else if (p_slice.compare(slice_int8_build_rawstr("Always")))
        {
            return v2::ShaderConfiguration::CompareOp::Always;
        }
        else if (p_slice.compare(slice_int8_build_rawstr("Invalid")))
        {
            return v2::ShaderConfiguration::CompareOp::Invalid;
        }
        else
        {
            abort();
        }
    };

    inline static Span<int8> allocate_asset_from_json(JSONDeserializer* p_json_deserializer)
    {
        v2::ShaderRessource::Asset::Value l_shader_ressource_value{};
        Vector<v2::ShaderLayoutParameterType> l_shader_layout_parameter_types_deserialized = Vector<v2::ShaderLayoutParameterType>::allocate(0);

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

        v2::ShaderRessource::Asset l_asset = v2::ShaderRessource::Asset::allocate_from_values(l_shader_ressource_value);

        l_layout_deserializer.free();
        l_shader_layout_parameter_types_deserialized.free();

        return l_asset.allocated_binary;
    };

    inline static void push_dependencies_from_json_to_buffer(JSONDeserializer* p_json_deserializer, Vector<int8>* in_out_buffer)
    {
        v2::ShaderRessource::AssetDependencies::Value l_shader_dependencies_value{};

        p_json_deserializer->next_field("execution_order");
        p_json_deserializer->next_field("vertex");
        l_shader_dependencies_value.vertex_module = HashSlice(p_json_deserializer->get_currentfield().value);
        p_json_deserializer->next_field("fragment");
        l_shader_dependencies_value.fragment_module = HashSlice(p_json_deserializer->get_currentfield().value);

        l_shader_dependencies_value.push_to_binary_buffer(in_out_buffer);
    };

    inline static v2::ShaderRessource::AssetDependencies allocate_dependencies_from_json(JSONDeserializer* p_json_deserializer)
    {
        Vector<int8> l_binary = Vector<int8>::allocate(0);
        push_dependencies_from_json_to_buffer(p_json_deserializer, &l_binary);
        return v2::ShaderRessource::AssetDependencies{l_binary.Memory};
    };
};

struct MaterialAssetJSON
{
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
                if (l_material_parameter_type.compare(slice_int8_build_rawstr("TEXTURE_GPU")))
                {
                    l_parameter_deserializer.next_field("val");
                    v2::MaterialRessource::Asset::Value::Parameters::add_parameter_texture(l_material_parameters, HashSlice(l_parameter_deserializer.get_currentfield().value));
                }
            }
            l_parameter_deserializer.free();
        }

        Span<int8> l_asset_allocated_binary = v2::MaterialRessource::Asset::allocate_from_values(v2::MaterialRessource::Asset::Value{l_material_parameters.to_varying_slice()}).allocated_binary;

        l_parameter_array_deserializer.free();
        l_material_parameters.free();

        return l_asset_allocated_binary;
    };

    inline static v2::MaterialRessource::AssetDependencies allocate_dependencies_from_json(JSONDeserializer* p_json_deserializer, const Slice<int8>& p_root_path)
    {
        Vector<int8> l_binary = Vector<int8>::allocate(0);

        p_json_deserializer->next_field("shader");
        BinarySerializer::type(&l_binary, HashSlice(p_json_deserializer->get_currentfield().value));

        {
            Span<int8> l_shader_file_content = AssetCompiler_open_and_read_asset_file(p_root_path, p_json_deserializer->get_currentfield().value);
            String l_shader_file_str = String::build_from_raw_span(l_shader_file_content);
            JSONDeserializer l_shader_asset_deserializer = JSONDeserializer::start(l_shader_file_str);
            JSONDeserializer l_shader_asset_value_deserializer;
            AssetJSON::move_json_deserializer_to_value(&l_shader_asset_deserializer, &l_shader_asset_value_deserializer);
            ShaderAssetJSON::push_dependencies_from_json_to_buffer(&l_shader_asset_value_deserializer, &l_binary);

            l_shader_asset_deserializer.free();
            l_shader_asset_value_deserializer.free();
            l_shader_file_str.free();
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
                if (l_material_parameter_type.compare(slice_int8_build_rawstr("TEXTURE_GPU")))
                {
                    l_parameter_deserializer.next_field("val");
                    l_textures.push_back_element(HashSlice(l_parameter_deserializer.get_currentfield().value));
                }
            }
            l_parameter_deserializer.free();
        }
        l_parameter_array_deserializer.free();

        BinarySerializer::slice(&l_binary, l_textures.Memory.slice.build_asint8());

        l_textures.free();

        return v2::MaterialRessource::AssetDependencies{l_binary.Memory};
    };
};