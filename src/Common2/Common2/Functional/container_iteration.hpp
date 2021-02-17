#pragma once

#define slice_foreach_begin(SliceVariable, IteratorName, SliceElementVariableName) \
for (loop(IteratorName, 0, (SliceVariable)->Size)) \
{\
auto* SliceElementVariableName = (SliceVariable)->get(IteratorName); \

#define slice_foreach_end() \
}

#define vector_foreach_begin(VectorVariable, IteratorName, VectorElementVariableName) \
for (loop(IteratorName, 0, (VectorVariable)->Size)) \
{\
auto& VectorElementVariableName = (VectorVariable)->get(IteratorName);

#define vector_foreach_end() \
}

#define vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
for (vector_loop_reverse((VectorVariable), IteratorName)) \
{ \
auto& VectorElementVariableName = (VectorVariable)->get(IteratorName);

#define vector_erase_if_2_end(VectorVariable, IteratorName, IfConditionVariableName) \
if ((IfConditionVariableName))\
{\
	(VectorVariable)->erase_element_at_always(IteratorName);\
};\
}


#define vector_erase_if_2_break_end(VectorVariable, IteratorName, IfConditionVariableName) \
if ((IfConditionVariableName))\
{\
	(VectorVariable)->erase_element_at_always(IteratorName);\
	break; \
};\
}

#define vector_erase_if_2_single_line(VectorVariable, IteratorName, VectorElementVariableName, IfConditionCode) \
vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
vector_erase_if_2_end(VectorVariable, IteratorName, IfConditionCode)

#define vector_erase_if_2_break_single_line(VectorVariable, IteratorName, VectorElementVariableName, IfConditionCode) \
vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
vector_erase_if_2_break_end(VectorVariable, IteratorName, IfConditionCode)
