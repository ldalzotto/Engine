#pragma once

namespace v2
{
    struct MathJSONDeserialization
    {
        inline static v3f _v3f(JSONDeserializer* p_json_object)
        {
            v3f l_return;
            json_deser_object_field("x", p_json_object, l_return.x, FromString::afloat32);
            json_deser_object_field("y", p_json_object, l_return.y, FromString::afloat32);
            json_deser_object_field("z", p_json_object, l_return.z, FromString::afloat32);
            return l_return;
        };

        inline static quat _quat(JSONDeserializer* p_json_object)
        {
            quat l_return;
            json_deser_object_field("x", p_json_object, l_return.x, FromString::afloat32);
            json_deser_object_field("y", p_json_object, l_return.y, FromString::afloat32);
            json_deser_object_field("z", p_json_object, l_return.z, FromString::afloat32);
            json_deser_object_field("w", p_json_object, l_return.w, FromString::afloat32);
            return l_return;
        };
    };

    struct MathJSONSerialization
    {
        inline static void _v3f(const v3f& p_value, JSONSerializer* p_serializer)
        {
            p_serializer->push_field(slice_int8_build_rawstr("x"), Slice<float32>::build_asint8_memory_singleelement(&p_value.x));
            p_serializer->push_field(slice_int8_build_rawstr("y"), Slice<float32>::build_asint8_memory_singleelement(&p_value.y));
            p_serializer->push_field(slice_int8_build_rawstr("z"), Slice<float32>::build_asint8_memory_singleelement(&p_value.z));
        };

        inline static void _quat(const quat& p_value, JSONSerializer* p_serializer)
        {
            p_serializer->push_field(slice_int8_build_rawstr("x"), Slice<float32>::build_asint8_memory_singleelement(&p_value.x));
            p_serializer->push_field(slice_int8_build_rawstr("y"), Slice<float32>::build_asint8_memory_singleelement(&p_value.y));
            p_serializer->push_field(slice_int8_build_rawstr("z"), Slice<float32>::build_asint8_memory_singleelement(&p_value.z));
            p_serializer->push_field(slice_int8_build_rawstr("w"), Slice<float32>::build_asint8_memory_singleelement(&p_value.w));
        };
    };
} 