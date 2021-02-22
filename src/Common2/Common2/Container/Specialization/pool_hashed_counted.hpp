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

	struct IncrementOrAllocateState
	{
		enum class State
		{
			UNDEFINED = 0, WAITING_FOR_ELEMENT = 1, INCREMENTED = 2, ALLOCATED = 3, END = ALLOCATED + 1
		} state;
		KeyType key;
		ElementType element_to_allocate;
		Token(ElementType) allocated_token;

		inline static IncrementOrAllocateState build(const KeyType& p_key)
		{
			return IncrementOrAllocateState{
					State::UNDEFINED, p_key
			};
		};
	};

	inline void increment_or_allocate_2(IncrementOrAllocateState& in_out_state)
	{
		switch (in_out_state.state)
		{
		case IncrementOrAllocateState::State::UNDEFINED:
		{
			if (this->CountMap.has_key_nothashed(in_out_state.key))
			{
				CountElement* l_counted_element = this->CountMap.get_value_nothashed(in_out_state.key);
				l_counted_element->counter += 1;
				in_out_state.allocated_token = l_counted_element->token;
				in_out_state.state = IncrementOrAllocateState::State::INCREMENTED;
			}
			else
			{
				in_out_state.state = IncrementOrAllocateState::State::WAITING_FOR_ELEMENT;
			}
		}
			break;
		case IncrementOrAllocateState::State::WAITING_FOR_ELEMENT:
		{
			in_out_state.allocated_token = this->pool.alloc_element(in_out_state.element_to_allocate);
			this->CountMap.push_key_value_nothashed(in_out_state.key, CountElement{ in_out_state.allocated_token, 1 });
			in_out_state.state = IncrementOrAllocateState::State::ALLOCATED;
		}
			break;
		}
	};

	struct DecrementOrDeallocateReturn
	{
		int8 id_deallocated;
		Token(ElementType) deallocated_token;
	};

	inline DecrementOrDeallocateReturn decrement_or_deallocate_pool_not_modified(const KeyType& p_key)
	{
		CountElement* l_counted_element = this->CountMap.get_value_nothashed(p_key);
		l_counted_element->counter -= 1;
		if (l_counted_element->counter == 0)
		{
			// p_element_deallocator(this->pool.get(l_counted_element->token));
			// this->pool.release_element(l_counted_element->token);
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