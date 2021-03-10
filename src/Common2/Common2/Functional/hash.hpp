#pragma once

using hash_t = uimax;

// http://www.cse.yorku.ca/~oz/hash.html
inline constexpr uimax HashFunctionRaw(const int8* p_value, const uimax p_size)
{
    hash_t hash = 5381;

    for (loop(i, 0, p_size))
    {
        int8 c = *(p_value + i);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

// http://www.cse.yorku.ca/~oz/hash.html
inline uimax HashFunctionRawChecked(const int8* p_value, const uimax p_size)
{
    hash_t hash = 5381;

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

template <class TYPE> inline hash_t HashFunction(const TYPE& p_value)
{
    return HashFunctionRaw((int8*)&p_value, sizeof(TYPE));
};

template <class TYPE> inline hash_t HashCombineFunction(const uimax p_hash, const TYPE& p_value)
{
    return p_hash ^ (HashFunction<TYPE>(p_value) + 0x9e3779b9 + (p_hash << 6) + (p_hash >> 2));
};

template <class TYPE> inline hash_t HashSlice(const Slice<TYPE>& p_value)
{
    return HashFunctionRaw((int8*)p_value.Begin, p_value.Size * sizeof(TYPE));
};

inline hash_t HashRaw(const int8* p_str)
{
    return HashFunctionRaw((int8*)p_str, strlen(p_str));
};

#define HASHRAW(s) HashFunctionRaw((int8*)s, STRLEN(s))