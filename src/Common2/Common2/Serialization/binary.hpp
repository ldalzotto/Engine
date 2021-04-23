#pragma once

namespace BinarySerializer
{

template <class ElementType> inline static void type(Vector<int8>* in_out_serialization_target, const ElementType& p_value)
{
    in_out_serialization_target->push_back_array(Slice<ElementType>::build_asint8_memory_singleelement(&p_value));
};

template <class ElementType> inline static void type(Slice<int8>* in_out_serialization_target, const ElementType& p_value)
{
#if __DEBUG
    assert_true(in_out_serialization_target->Size >= sizeof(ElementType));
#endif
    *(ElementType*)in_out_serialization_target->Begin = p_value;
    in_out_serialization_target->slide(sizeof(ElementType));
};

inline static void slice(Vector<int8>* in_out_serialization_target, const Slice<int8>& p_slice)
{
    type(in_out_serialization_target, p_slice.Size);
    in_out_serialization_target->push_back_array(p_slice);
};

inline static void slice(Slice<int8>* in_out_serialization_target, const Slice<int8>& p_slice)
{
    type(in_out_serialization_target, p_slice.Size);
    in_out_serialization_target->copy_memory(p_slice);
    in_out_serialization_target->slide(p_slice.Size);
};

inline static uimax slice_ret_bytesnb(Slice<int8>* in_out_serialization_target, const Slice<int8>& p_slice)
{
    slice(in_out_serialization_target, p_slice);
    return sizeof(p_slice.Size) + p_slice.Size;
};

inline static void varying_slice(Vector<int8>* in_out_serialization_target, const VaryingSlice& p_varying_slice)
{
    slice(in_out_serialization_target, p_varying_slice.memory);
    slice(in_out_serialization_target, p_varying_slice.chunks.build_asint8());
};
inline static void varying_slice(Slice<int8>* in_out_serialization_target, const VaryingSlice& p_varying_slice)
{
    slice(in_out_serialization_target, p_varying_slice.memory);
    slice(in_out_serialization_target, p_varying_slice.chunks.build_asint8());
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