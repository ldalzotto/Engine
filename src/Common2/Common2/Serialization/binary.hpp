#pragma once

namespace BinarySerializer
{
	template<class ElementType>
	inline static void type(Vector<int8>* in_out_serialization_target, const ElementType& p_value)
	{
		in_out_serialization_target->push_back_array(Slice<ElementType>::build_asint8_memory_singleelement(&p_value));
	};

	inline static void slice(Vector<int8>* in_out_serialization_target, const Slice<int8>& p_slice)
	{
		type(in_out_serialization_target, p_slice.Size);
		in_out_serialization_target->push_back_array(p_slice);
	};

	inline static void span(Vector<int8>* in_out_serialization_target, const Span<int8>& p_span)
	{
		slice(in_out_serialization_target, p_span.slice);
	};

	//TODO -> delete, we don't want to serialize a resizable container
	inline static void varying_vector(Vector<int8>* in_out_serialization_target, const VaryingVector& p_varying_vector)
	{
		slice(in_out_serialization_target, p_varying_vector.memory.to_slice());
		slice(in_out_serialization_target, p_varying_vector.chunks.to_slice().build_asint8());
	};
};

struct BinaryDeserializer
{
	Slice<int8> memory;

	inline static BinaryDeserializer build(const Slice<int8>& p_memory)
	{
		return BinaryDeserializer{ p_memory };
	};

	template<class ElementType>
	inline ElementType* type()
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

	inline Span<int8> span()
	{
		Span<int8> l_span;
		l_span.slice = slice();
		return l_span;
	};

	//TODO -> delete, we don't want to deserialize to a resizable container
	template<class ElementType>
	inline Vector<ElementType> vector()
	{
		Vector<ElementType> l_vector;
		l_vector.Memory.slice = slice_cast<ElementType>(span().slice);
		l_vector.Size = l_vector.Memory.Capacity;
		return l_vector;
	};

	//TODO -> delete, we don't want to deserialize to a resizable container
	inline VaryingVector varying_vector()
	{
		VaryingVector l_verying_vector;
		l_verying_vector.memory = vector<int8>();
		l_verying_vector.chunks = vector<SliceIndex>();
		return l_verying_vector;
	};

};