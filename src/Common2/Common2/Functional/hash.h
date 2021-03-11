#pragma once

typedef uimax hash_t;

#define HASH_SEED 5381

// http://www.cse.yorku.ca/~oz/hash.html
inline static uimax HashFunctionRawChecked(const int8* p_value, const uimax p_size)
{
    hash_t hash = HASH_SEED;

    for (loop(i, 0, p_size))
    {
        int8 c = *(p_value + i);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

#if CONTAINER_BOUND_TEST
    // -1 is usually the default value of hash_t, we we prevent in debuf mode any value that can clash
    assert_true(hash != (hash_t)-1);
#endif

    return hash;
}

inline hash_t HashSlice_(const Slice_* p_value, const uimax t_value_elementtype)
{
    return HashFunctionRawChecked(p_value->Begin, p_value->Size * t_value_elementtype);
};

inline hash_t HashSlice__0v(const Slice_ p_value, const uimax t_value_elementtype)
{
    return HashSlice_(&p_value, t_value_elementtype);
};

inline hash_t HashRaw(const int8* p_str)
{
    return HashFunctionRawChecked((int8*)p_str, strlen(p_str));
};