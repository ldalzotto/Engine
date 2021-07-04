#pragma once

namespace BinarySize
{
inline static uimax slice(const Slice<int8>& p_slice)
{
    return sizeof(p_slice.Size) + p_slice.Size;
};
inline static uimax varying_slice(const VaryingSlice& p_slice)
{
    return BinarySize::slice(p_slice.memory) + BinarySize::slice(p_slice.chunks.build_asint8());
};
}; // namespace BinarySize

namespace BinarySerializer
{
template <class Vector, class ElementType> inline static void type(iVector<Vector> in_out_serialization_target, const ElementType& p_value)
{
    iVector<Vector>::Assert::element_type<int8>();

    in_out_serialization_target.push_back_array(Slice<ElementType>::build_asint8_memory_singleelement(&p_value));
};

template <class Vector> inline static void slice(iVector<Vector> in_out_serialization_target, const Slice<int8>& p_slice)
{
    iVector<Vector>::Assert::element_type<int8>();

    BinarySerializer::type(in_out_serialization_target, p_slice.Size);
    in_out_serialization_target.push_back_array(p_slice);
};

template <class Vector> inline static uimax slice_ret_bytesnb(iVector<Vector> in_out_serialization_target, const Slice<int8>& p_slice)
{
    iVector<Vector>::Assert::element_type<int8>();

    BinarySerializer::slice(in_out_serialization_target, p_slice);
    return sizeof(p_slice.Size) + p_slice.Size;
};

template <class Vector> inline static void varying_slice(iVector<Vector> in_out_serialization_target, const VaryingSlice& p_varying_slice)
{
    iVector<Vector>::Assert::element_type<int8>();

    BinarySerializer::slice(in_out_serialization_target, p_varying_slice.memory);
    BinarySerializer::slice(in_out_serialization_target, p_varying_slice.chunks.build_asint8());
};

}; // namespace BinarySerializer

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