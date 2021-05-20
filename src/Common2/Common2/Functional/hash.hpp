#pragma once

using hash_t = uimax;

struct strlen_compile
{
    static constexpr uimax get_size(const int8* p_memory)
    {
        return get_size_step(p_memory, 0);
    };

  private:
    static constexpr uimax get_size_step(const int8* p_memory, const uimax p_index)
    {
        return ((*(p_memory + p_index)) == (int8)NULL) ? p_index : get_size_step(p_memory, p_index + 1);
    };
};

struct HashFunctions
{
    static hash_t hash(const Slice<int8>& p_memory)
    {
        hash_t l_hash = hash_begin;
        for (loop(i, 0, p_memory.Size))
        {
            l_hash = hash_step(&p_memory.get(i), l_hash);
        }
        return l_hash;
    };

    template <uimax _Size> static constexpr hash_t hash_compile(const char* p_memory)
    {
        return HashCompileInitialization<_Size>{}.exec(p_memory);
    };

  private:
    static constexpr uimax hash_begin = 5381;

    // http://www.cse.yorku.ca/~oz/hash.html
    static constexpr uimax hash_step(const int8* p_memory, const uimax p_hash)
    {
        return ((p_hash << 5) + p_hash) + (*(p_memory));
    };

    static constexpr uimax hash_step_indexed(const int8* p_memory, const uimax p_index, const uimax p_hash)
    {
        return hash_step(p_memory + p_index, p_hash);
    };

    template <uimax _index, uimax _Size> struct HashCompileFragment
    {
        constexpr uimax exec(const char* p_memory, const uimax p_hash)
        {
            return HashCompileFragment<_index + 1, _Size>{}.exec(p_memory, HashFunctions::hash_step_indexed(p_memory, _index, p_hash));
        };
    };

    template <uimax _Size> struct HashCompileFragment<_Size, _Size>
    {
        constexpr uimax exec(const char* p_memory, const uimax p_hash)
        {
            return p_hash;
        };
    };

    template <uimax _Size> struct HashCompileInitialization
    {
        constexpr uimax exec(const char* p_memory)
        {
            return HashCompileFragment<0, _Size>{}.exec(p_memory, hash_begin);
        };
    };
};
