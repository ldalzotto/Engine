#pragma once

#define Hashmap_HashFn_Hash(p_key, p_modulo) hash(p_key, p_modulo)

template <class KeyType_t> struct HashMap_HashFn
{
    inline static hash_t Hashmap_HashFn_Hash(const KeyType_t& p_key, const uimax p_modulo)
    {
        return HashSlice(Slice_build_memory_elementnb<KeyType_t>((KeyType_t*)&p_key, 1)) % p_modulo;
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
        return HashMap<KeyType, ElementType>{Span_allocate<ElementType>(p_initial_size), Span_allocate<KeyType>(p_initial_size), Span_callocate<int8>(p_initial_size)};
    };

    inline static HashMap<KeyType, ElementType> allocate_default()
    {
        return allocate(8);
    };

    inline void resize(const uimax p_new_size)
    {
        Span_resize(&this->Memory, p_new_size);
        Span_resize(&this->StoredKeys, p_new_size);
        Span_resize(&this->Slots, p_new_size);
    };

    inline void zero()
    {
        Slice_zero(&this->Slots.slice);
    };

    inline void free()
    {
        Span_free(&this->Memory);
        Span_free(&this->StoredKeys);
        Span_free(&this->Slots);
    };

    inline uimax get_capacity()
    {
        return this->Memory.Capacity;
    };

    inline int8 empty()
    {
        for (loop(i, 0, this->Slots.Capacity))
        {
            if (*Span_get(&this->Slots, i))
            {
                return 0;
            }
        }

        return 1;
    };

    inline int8 has_key(const hash_t p_hash)
    {
        return *Span_get(&this->Slots, p_hash);
    };

    inline int8 has_key_nothashed(const KeyType& p_key)
    {
        hash_t l_hash = this->hash_key(p_key);
        if (this->has_key(l_hash))
        {
            if (this->check_key_equality(p_key, l_hash))
            {
                return 1;
            }
        }
        return 0;
    };

    inline ElementType* get_value(const hash_t p_hash)
    {
        if (this->has_key(p_hash))
        {
            return Span_get(&this->Memory, p_hash);
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

        *Span_get(&this->Memory, p_hash) = p_value;
        *Span_get(&this->StoredKeys, p_hash) = p_key;
        *Span_get(&this->Slots, p_hash) = 1;
    };

    inline void put_value(const hash_t p_hash, const ElementType& p_value)
    {
#if __DEBUG
        assert_true(this->has_key(p_hash)); // use has_key before
#endif

        *Span_get(&this->Memory, p_hash) = p_value;
    };

    inline void put_value_nothashed(const KeyType& p_key, const ElementType& p_value)
    {
        hash_t l_hash = this->hash_key(p_key);
#if __DEBUG
        assert_true(this->has_key(l_hash) && this->check_key_equality(p_key, l_hash)); // use has_key before
#endif
        *Span_get(&this->Memory, l_hash) = p_value;
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
        this->Slots.Span_get(p_hash) = 0;
    };

    inline void erase_key_nothashed(const KeyType& p_key)
    {
        hash_t l_hash = this->hash_key(p_key);
#if __DEBUG
        assert_true(this->check_key_equality(p_key, l_hash));
#endif
        *Span_get(&this->Slots, l_hash) = 0;
    };

  private:
    inline void rehash_to(HashMap<KeyType, ElementType>* in_out_new_map)
    {
        for (loop(i, 0, this->Slots.Capacity))
        {
            if (*Span_get(&this->Slots, i))
            {
                KeyType& l_key = *Span_get(&this->StoredKeys, i);
                in_out_new_map->put_key_value(in_out_new_map->hash_key(l_key), l_key, *Span_get(&this->Memory, i));
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
        return memory_compare((int8*)Span_get(&this->StoredKeys, p_hash), (int8*)&p_key, sizeof(KeyType));
    };
};

#undef Hashmap_HashFn_Hash