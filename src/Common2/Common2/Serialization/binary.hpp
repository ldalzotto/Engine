#pragma once

namespace BinarySerializer
{
template <class ElementType> inline static void type(Vector<int8>* in_out_serialization_target, const ElementType& p_value)
{
    in_out_serialization_target->push_back_array(Slice_build_asint8_memory_singleelement<ElementType>(&p_value));
};

inline static void slice(Vector<int8>* in_out_serialization_target, const Slice<int8>& p_slice)
{
    type(in_out_serialization_target, p_slice.Size);
    in_out_serialization_target->push_back_array(p_slice);
};

inline static void varying_slice(Vector<int8>* in_out_serialization_target, const VaryingSlice& p_varying_slice)
{
    slice(in_out_serialization_target, p_varying_slice.memory);
    slice(in_out_serialization_target, Slice_build_asint8(&p_varying_slice.chunks));
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
        Slice_slide(&this->memory, sizeof(ElementType));
        return l_element;
    };

    inline Slice<int8> slice()
    {
        Slice<int8> l_slice;
        l_slice.Size = *type<uimax>();
        l_slice.Begin = this->memory.Begin;
        Slice_slide(&this->memory, l_slice.Size);
        return l_slice;
    };

    inline VaryingSlice varying_slice()
    {
        VaryingSlice l_verying_vector;
        l_verying_vector.memory = slice();
        Slice<int8> l_chunks_slice = slice();
        l_verying_vector.chunks = Slice_cast<SliceIndex>(&l_chunks_slice);
        return l_verying_vector;
    };
};