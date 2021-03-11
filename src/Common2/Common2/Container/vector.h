#pragma once

/*
    A Vector is a Span with an imaginary boundary (Size).
    Vector memory is continuous, there is no "hole" between items.
    Vector is reallocated on need.
    Any memory access outside of this imaginary boundary will be in error.
    The Vector expose some safe way to insert/erase data (array or single element).
*/
typedef struct s_Vector_
{
    uimax Size;
    Span_ Memory;
} Vector_;

inline Vector_ Vector__build(const uimax p_size, Span_ p_span)
{
#if __cplusplus
    return Vector_{.Size = p_size, .Memory = p_span};
#else
    return (Vector_){.Size = p_size, .Memory = p_span};
#endif
};

inline Vector_ Vector__allocate(const uimax p_initial_capacity, const uimax t_elementtype)
{
    return Vector__build(0, Span__allocate(t_elementtype, p_initial_capacity));
};

inline Vector_ Vector__allocate_slice(const uimax t_elementtype, const Slice_* p_initial_elements)
{
    return Vector__build(0, Span__allocate_slice(t_elementtype, p_initial_elements));
};

/*
inline Vector_ Vector__allocate_capacity_slice(const uimax t_elementtype, const uimax p_inital_capacity, const Slice_* p_initial_elements)
{
    Vector_ l_vector = Vector__allocate(p_inital_capacity, t_elementtype);
    l_vector.push_back_array(p_initial_elements);
    return l_vector;
};
*/