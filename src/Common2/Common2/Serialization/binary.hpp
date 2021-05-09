#pragma once

namespace BinarySerializer
{
template <class ShadowVector(int8), class ElementType> inline static void type(ShadowVector(int8) * in_out_serialization_target, const ElementType& p_value)
{
    sv_c_push_back_array(*in_out_serialization_target, Slice<ElementType>::build_asint8_memory_singleelement(&p_value));
};

template <class ShadowVector(int8)> inline static void slice(ShadowVector(int8) * in_out_serialization_target, const Slice<int8>& p_slice)
{
    BinarySerializer::type(in_out_serialization_target, p_slice.Size);
    sv_c_push_back_array(*in_out_serialization_target, p_slice);
};

template <class ShadowVector(int8)> inline static uimax slice_ret_bytesnb(ShadowVector(int8) * in_out_serialization_target, const Slice<int8>& p_slice)
{
    BinarySerializer::slice(in_out_serialization_target, p_slice);
    return sizeof(p_slice.Size) + p_slice.Size;
};

template <class ShadowVector(int8)> inline static void varying_slice(ShadowVector(int8) * in_out_serialization_target, const VaryingSlice& p_varying_slice)
{
    BinarySerializer::slice(in_out_serialization_target, p_varying_slice.memory);
    BinarySerializer::slice(in_out_serialization_target, p_varying_slice.chunks.build_asint8());
};

}; // namespace BinarySerializer_v2

struct BinaryDeserializer
{
    Slice<int8> memory;

    inline static BinaryDeserializer build(const Slice<int8>& p_memory)
    {
        return BinaryDeserializer{p_memory};
    };

    template <class ElementType> inline ElementType* type()
    {
        ElementType* l_element = (ElementType*)this->memory.Begin;
        this->memory.slide(sizeof(ElementType));
        return l_element;
    };

    inline Slice<int8> slice()
    {
        Slice<int8> l_slice;
        l_slice.Size = *type<uimax>();
        l_slice.Begin = this->memory.Begin;
        this->memory.slide(l_slice.Size);
        return l_slice;
    };

    inline VaryingSlice varying_slice()
    {
        VaryingSlice l_verying_vector;
        l_verying_vector.memory = slice();
        l_verying_vector.chunks = slice_cast<SliceIndex>(slice());
        return l_verying_vector;
    };
};