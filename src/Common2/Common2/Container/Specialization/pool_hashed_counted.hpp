#pragma once

template<class KeyType, class ElementType>
struct PoolHashedCounted
{
	struct CountElement
	{
		Token(ElementType) token;
		uimax counter;
	};

	Pool<ElementType> pool;
	HashMap<KeyType, CountElement> CountMap;

	inline static PoolHashedCounted<KeyType, ElementType> allocate_default()
	{
		return PoolHashedCounted<KeyType, ElementType>
				{
						Pool<ElementType>::allocate(0),
						HashMap<KeyType, CountElement>::allocate_default()
				};
	};

	inline void free()
	{
		this->pool.free();
		this->CountMap.free();
	};

	inline int8 empty()
	{
		return !this->pool.has_allocated_elements() && this->CountMap.empty();
	};

	//TODO -> simplify these templated functions to smaller ones and composition
	//TODO -> add test :)
	template<class AllocatedElementBuilderFunc, class OnAllocatedFunc>
	inline Token(ElementType) increment_or_allocate_3(const KeyType& p_key, const AllocatedElementBuilderFunc& p_allocated_element_builder_func,
			const OnAllocatedFunc& p_on_allocated_func)
	{
		if (this->CountMap.has_key_nothashed(p_key))
		{
			CountElement* l_counted_element = this->CountMap.get_value_nothashed(p_key);
			l_counted_element->counter += 1;
			return l_counted_element->token;
		}
		else
		{
			Token(ElementType) l_allocated_token = this->pool.alloc_element(p_allocated_element_builder_func(p_key));
			this->CountMap.push_key_value_nothashed(p_key, CountElement{ l_allocated_token, 1 });
			p_on_allocated_func(l_allocated_token);
			return l_allocated_token;
		}
	};

	struct DecrementOrDeallocateReturn
	{
		int8 is_deallocated;
		Token(ElementType) deallocated_token;
	};

	inline DecrementOrDeallocateReturn decrement_or_deallocate_pool_not_modified(const KeyType& p_key)
	{
		CountElement* l_counted_element = this->CountMap.get_value_nothashed(p_key);
		l_counted_element->counter -= 1;
		if (l_counted_element->counter == 0)
		{
			this->CountMap.erase_key_nothashed(p_key);
			return DecrementOrDeallocateReturn{
					1, l_counted_element->token
			};
		}
		return DecrementOrDeallocateReturn{
				0
		};
	};
};

#define ShadowPoolHashedCounted(KeyType, ElementType) ShadowPoolHashedCounted_##KeyType_##ElementType
#define ShadowPoolHashedCounted_member_pool pool
#define ShadowPoolHashedCounted_decrement_or_deallocate_pool_not_modified(p_key) decrement_or_deallocate_pool_not_modified(p_key)