#pragma once

struct MathJSONDeserialization
{
    inline static v2f _v2f(JSONDeserializer* p_json_object)
    {
        v2f l_return;
        p_json_object->next_field("x");
        l_return.x = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("y");
        l_return.y = FromString::afloat32(p_json_object->get_currentfield().value);
        return l_return;
    };

    inline static v3f _v3f(JSONDeserializer* p_json_object)
    {
        v3f l_return;
        p_json_object->next_field("x");
        l_return.x = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("y");
        l_return.y = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("z");
        l_return.z = FromString::afloat32(p_json_object->get_currentfield().value);
        return l_return;
    };

    inline static v4f _v4f(JSONDeserializer* p_json_object)
    {
        v4f l_return;
        p_json_object->next_field("x");
        l_return.x = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("y");
        l_return.y = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("z");
        l_return.z = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("w");
        l_return.w = FromString::afloat32(p_json_object->get_currentfield().value);
        return l_return;
    };

    inline static quat _quat(JSONDeserializer* p_json_object)
    {
        quat l_return;
        p_json_object->next_field("x");
        l_return.x = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("y");
        l_return.y = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("z");
        l_return.z = FromString::afloat32(p_json_object->get_currentfield().value);
        p_json_object->next_field("w");
        l_return.w = FromString::afloat32(p_json_object->get_currentfield().value);
        return l_return;
    };
};

struct MathJSONSerialization
{
    inline static void _v3f(const v3f& p_value, JSONSerializer* p_serializer)
    {
        p_serializer->push_field(slice_int8_build_rawstr("x"), Slice<float32>::build_asint8_memory_singleelement(&p_value.x));
        p_serializer->coma();
        p_serializer->push_field(slice_int8_build_rawstr("y"), Slice<float32>::build_asint8_memory_singleelement(&p_value.y));
        p_serializer->coma();
        p_serializer->push_field(slice_int8_build_rawstr("z"), Slice<float32>::build_asint8_memory_singleelement(&p_value.z));
    };

    inline static void _quat(const quat& p_value, JSONSerializer* p_serializer)
    {
        p_serializer->push_field(slice_int8_build_rawstr("x"), Slice<float32>::build_asint8_memory_singleelement(&p_value.x));
        p_serializer->coma();
        p_serializer->push_field(slice_int8_build_rawstr("y"), Slice<float32>::build_asint8_memory_singleelement(&p_value.y));
        p_serializer->coma();
        p_serializer->push_field(slice_int8_build_rawstr("z"), Slice<float32>::build_asint8_memory_singleelement(&p_value.z));
        p_serializer->coma();
        p_serializer->push_field(slice_int8_build_rawstr("w"), Slice<float32>::build_asint8_memory_singleelement(&p_value.w));
    };
};

struct MathBinaryDeserialization
{
    inline static transform _transform(BinaryDeserializer& p_deserializer)
    {
        return transform{*p_deserializer.type<v3f>(), *p_deserializer.type<quat>(), *p_deserializer.type<v3f>()};
    };
};