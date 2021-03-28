#pragma once

struct PrimitiveSerializedTypes
{
    enum class Type
    {
        UNDEFINED = 0,
        FLOAT32 = UNDEFINED + 1,
        FLOAT32_2 = FLOAT32 + 1,
        FLOAT32_3 = FLOAT32_2 + 1,
        FLOAT32_4 = FLOAT32_3 + 1,
    };

    inline static uimax get_size(const Type p_type)
    {
        switch (p_type)
        {
        case Type::FLOAT32:
            return sizeof(float32);
        case Type::FLOAT32_2:
            return sizeof(float32) * 2;
        case Type::FLOAT32_3:
            return sizeof(float32) * 3;
        case Type::FLOAT32_4:
            return sizeof(float32) * 4;
        default:
            abort();
        }
    };

    inline static PrimitiveSerializedTypes::Type get_type_from_string(const Slice<int8>& p_str)
    {
        Slice<int8> l_float_32 = Slice_int8_build_rawstr("FLOAT32");
        if (Slice_compare(&l_float_32, &p_str))
        {
            return PrimitiveSerializedTypes::Type::FLOAT32;
        }
        else
        {
            Slice<int8> l_float_32_2 = Slice_int8_build_rawstr("FLOAT32_2");
            if (Slice_compare(&l_float_32_2, &p_str))
            {
                return PrimitiveSerializedTypes::Type::FLOAT32_2;
            }
            else
            {
                Slice<int8> l_float_32_3 = Slice_int8_build_rawstr("FLOAT32_3");
                if (Slice_compare(&l_float_32_3, &p_str))
                {
                    return PrimitiveSerializedTypes::Type::FLOAT32_3;
                }
                else
                {
                    Slice<int8> l_float_32_4 = Slice_int8_build_rawstr("FLOAT32_4");
                    if (Slice_compare(&l_float_32_4, &p_str))
                    {
                        return PrimitiveSerializedTypes::Type::FLOAT32_4;
                    }
                    else
                    {
                        abort();
                    }
                }
            }
        }
    };
};
