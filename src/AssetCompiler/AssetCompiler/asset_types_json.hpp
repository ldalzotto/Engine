#pragma once

struct ShaderAssetJSON
{
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
                        Slice<int8> l_type_slice = l_layout_parameter_deserializer.get_currentfield().value;
                        if (l_type_slice.compare(slice_int8_build_rawstr("TEXTURE_FRAGMENT")))
                        {
                            l_shader_layout_parameter_types_deserialized.push_back_element(v2::ShaderLayoutParameterType::TEXTURE_FRAGMENT);
                        }
                        // TODO map to enum
                    };
                }
                l_layout_parameter_deserializer.free();
            }
            l_layout_parameters_deserializer.free();
        }
        l_shader_ressource_value.specific_parameters = l_shader_layout_parameter_types_deserialized.to_slice();

        if (p_json_deserializer->next_field("ztest"))
        {
            Slice<int8> l_z_test_string = p_json_deserializer->get_currentfield().value;
            if (l_z_test_string.compare(slice_int8_build_rawstr("Always")))
            {
                l_shader_ressource_value.shader_configuration.ztest = v2::ShaderConfiguration::CompareOp::Always;
            }
            // TODO -> others
        }

        p_json_deserializer->next_field("zwrite");
        l_shader_ressource_value.shader_configuration.zwrite = FromString::auint8(p_json_deserializer->get_currentfield().value);

        v2::ShaderRessource::Asset l_asset = v2::ShaderRessource::Asset::allocate_from_values(l_shader_ressource_value);

        l_layout_deserializer.free();
        l_shader_layout_parameter_types_deserialized.free();

        return l_asset.allocated_binary;
    };

    inline static v2::ShaderRessource::AssetDependencies::Value build_dependencies_from_json(JSONDeserializer* p_json_deserializer)
    {
        v2::ShaderRessource::AssetDependencies::Value l_shader_dependencies_value{};

        p_json_deserializer->next_field("execution_order");
        p_json_deserializer->next_field("vertex");
        l_shader_dependencies_value.vertex_module = HashSlice(p_json_deserializer->get_currentfield().value);
        p_json_deserializer->next_field("fragment");
        l_shader_dependencies_value.fragment_module = HashSlice(p_json_deserializer->get_currentfield().value);

        return l_shader_dependencies_value;
    };

    inline static Span<int8> allocate_dependencies_from_json(JSONDeserializer* p_json_deserializer)
    {
        return v2::ShaderRessource::AssetDependencies::allocate_from_values(build_dependencies_from_json(p_json_deserializer)).allocated_binary;
    };
};

/*
    "execution_order": "1001",
    "vertex": "/shad.vert",
    "fragment": "/shad.frag",
    "layout": {
      "parameters": [
        { "type": "TEXTURE_FRAGMENT" }
      ]
    },
    "ztest": "Always",
    "zwrite": "false"

 */