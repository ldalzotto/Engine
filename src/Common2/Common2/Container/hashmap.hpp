#pragma once

#define Hashmap_HashFn_Hash(p_key, p_modulo) hash(p_key, p_modulo)

template <class KeyType_t> struct HashMap_HashFn
{
    inline static hash_t Hashmap_HashFn_Hash(const KeyType_t& p_key, const uimax p_modulo)
    {
        return HashSlice(Slice<KeyType_t>::build_memory_elementnb((KeyType_t*)&p_key, 1)) % p_modulo;
    };
};

// We specialize hash only for hash_t because it's formula is simpler
template <> struct HashMap_HashFn<hash_t>
{
    inline static hash_t Hashmap_HashFn_Hash(const hash_t p_key, const uimax p_modulo)
    {
        return p_key % p_modulo;
    };
};

template <class KeyType, class ElementType> struct HashMap
{
    Span<ElementType> Memory;
    Span<KeyType> StoredKeys; // for rehash
    Span<int8> Slots;

    inline static HashMap<KeyType, ElementType> allocate(const uimax p_initial_size)
    {
#if __DEBUG
        if (p_initial_size < 2)
        {
            abort(); // minimal allocation size is 2
        }
#endif
        return HashMap<KeyType, ElementType>{Span<ElementType>::allocate(p_initial_size), Span<KeyType>::allocate(p_initial_size), Span<int8>::callocate(p_initial_size)};
    };

    inline static HashMap<KeyType, ElementType> allocate_default()
    {
        return allocate(8);
    };

    inline void resize(const uimax p_new_size)
    {
        this->Memory.resize(p_new_size);
        this->StoredKeys.resize(p_new_size);
        this->Slots.resize(p_new_size);
    };

    inline void zero()
    {
        this->Slots.slice.zero();
    };

    inline void free()
    {
        this->Memory.free();
        this->StoredKeys.free();
        this->Slots.free();
    };

    inline uimax get_capacity()
    {
        return this->Memory.Capacity;
    };

    inline int8 empty()
    {
        for (loop(i, 0, this->Slots.Capacity))
        {
            if (this->Slots.get(i))
            {
                return 0;
            }
        }

        return 1;
    };

    inline int8 has_key(const hash_t p_hash)
    {
        return this->Slots.get(p_hash);
    };

    inline int8 has_key_nothashed(const KeyType& p_key)
    {
#if __DEBUG
        hash_t l_hash = this->hash_key(p_key);
        if (this->has_key(l_hash))
        {
            // TODO check_key_equality should not be an assertion here because l_hash can be the same as an other key that hashed to the same result.
            assert_true(this->check_key_equality(p_key, l_hash));
            return 1;
        }
        return 0;
#else
        return this->has_key(this->hash_key(p_key));
#endif
    };

    inline ElementType* get_value(const hash_t p_hash)
    {
        if (this->has_key(p_hash))
        {
            return &this->Memory.get(p_hash);
        }

        return NULL;
    };

    inline ElementType* get_value_nothashed(const KeyType& p_key)
    {
        hash_t l_hash = this->hash_key(p_key);
        if (this->has_key(l_hash))
        {
#if __DEBUG
            assert_true(this->check_key_equality(p_key, l_hash));
#endif
            return this->get_value(l_hash);
        }

        return NULL;
    };

    inline void put_key_value(const hash_t p_hash, const KeyType& p_key, const ElementType& p_value)
    {
#if __DEBUG
        assert_true(!this->has_key(p_hash)); // use has_key before
#endif

        this->Memory.get(p_hash) = p_value;
        this->StoredKeys.get(p_hash) = p_key;
        this->Slots.get(p_hash) = 1;
    };

    inline void put_value(const hash_t p_hash, const ElementType& p_value)
    {
#if __DEBUG
        assert_true(this->has_key(p_hash)); // use has_key before
#endif

        this->Memory.get(p_hash) = p_value;
    };

    inline void put_value_nothashed(const KeyType& p_key, const ElementType& p_value)
    {
        hash_t l_hash = this->hash_key(p_key);
#if __DEBUG
        assert_true(this->has_key(l_hash) && this->check_key_equality(p_key, l_hash)); // use has_key before
#endif
        this->Memory.get(l_hash) = p_value;
    };

    inline void push_key_value_nothashed(const KeyType& p_key, const ElementType& p_value)
    {
        hash_t l_hash = this->hash_key(p_key);
        if (this->has_key(l_hash))
        {
            HashMap<KeyType, ElementType> l_new_map = HashMap<KeyType, ElementType>::allocate(this->Memory.Capacity * 2);
            this->rehash_to(&l_new_map);
            l_hash = l_new_map.hash_key(p_key);
            while (l_new_map.has_key(l_hash))
            {
                l_new_map.resize(l_new_map.Memory.Capacity * 2);
                l_new_map.zero();
                this->rehash_to(&l_new_map);
                l_hash = l_new_map.hash_key(p_key);
            }

            this->free();
            *this = l_new_map;
        }

        this->put_key_value(l_hash, p_key, p_value);
    };

    inline void erase_key(const hash_t p_hash)
    {
        this->Slots.get(p_hash) = 0;
    };

    inline void erase_key_nothashed(const KeyType& p_key)
    {
        hash_t l_hash = this->hash_key(p_key);
#if __DEBUG
        assert_true(this->check_key_equality(p_key, l_hash));
#endif
        this->Slots.get(l_hash) = 0;
    };

  private:
    inline void rehash_to(HashMap<KeyType, ElementType>* in_out_new_map)
    {
        for (loop(i, 0, this->Slots.Capacity))
        {
            if (this->Slots.get(i))
            {
                KeyType& l_key = this->StoredKeys.get(i);
                in_out_new_map->put_key_value(in_out_new_map->hash_key(l_key), l_key, this->Memory.get(i));
            }
        }
    };

    inline hash_t hash_key(const KeyType& p_key)
    {
        return HashMap_HashFn<KeyType>::Hashmap_HashFn_Hash(p_key, this->Memory.Capacity);
    };

    inline hash_t mod_key(const hash_t p_hash)
    {
        return p_hash % this->Memory.Capacity;
    };

    inline int8 check_key_equality(const KeyType& p_key, const hash_t p_hash)
    {
        return memory_compare((int8*)&this->StoredKeys.get(p_hash), (int8*)&p_key, sizeof(KeyType));
    };
};

#undef Hashmap_HashFn_Hash