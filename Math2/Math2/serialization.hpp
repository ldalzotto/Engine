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
}