#pragma once

#define RessourceAllocationEvent_member_allocated_ressource allocated_ressource
#define RessourceAllocationEvent_member_asset asset

struct RessourceIdentifiedHeader
{
	int8 allocated;
	hash_t id;

	inline static RessourceIdentifiedHeader build_default()
	{
		return RessourceIdentifiedHeader{
				0, 0
		};
	};

	inline static RessourceIdentifiedHeader build_with_id(const hash_t p_id)
	{
		return RessourceIdentifiedHeader{
				0, p_id
		};
	};
};

/*
	Utility functions that handle the execution flow of ressource allocation/free event creation
*/
struct RessourceComposition
{

	struct AllocationEventFoundSlot
	{
		struct Nil
		{
			template<class RessourceAllocationEvent_t>
			inline static void slot(const RessourceAllocationEvent_t& p_allocation_event)
			{
			}
		};

		struct FreeAsset
		{
			template<class RessourceAllocationEvent_t>
			inline static void slot(RessourceAllocationEvent_t& p_allocation_event)
			{
				p_allocation_event.asset.free();
			};
		};
	};

	/*
		Iterates over the vector of allocation events and remove those whom ressource token is the same as input,
	 	calling AllocationEventFoundSlot_t on the way.
	*/
	template<class ShadowVector(RessourceAllocationEvent), class ShadowToken(Ressource), class AllocationEventFoundSlot_t>
	inline static void remove_reference_from_allocation_events_explicit(ShadowVector(RessourceAllocationEvent)& p_allocation_events,
			const ShadowToken(Ressource) p_ressource, const AllocationEventFoundSlot_t& p_allocation_event_found_slot)
	{
		for (loop_reverse(i, 0, p_allocation_events.sv_get_size()))
		{
			auto& l_event = p_allocation_events.sv_get(i);
			if (tk_eq(l_event.RessourceAllocationEvent_member_allocated_ressource, p_ressource))
			{
				p_allocation_event_found_slot.slot(l_event);
				p_allocation_events.sv_erase_element_at_always(i);
			}
		}
	};

	/*
		Default version of remove_reference_from_allocation_events_explicit
	*/
	template<class ShadowVector(RessourceAllocationEvent), class ShadowToken(Ressource)>
	inline static void remove_reference_from_allocation_events(ShadowVector(RessourceAllocationEvent)& p_allocation_events,
			const ShadowToken(Ressource) p_ressource)
	{
		remove_reference_from_allocation_events_explicit(p_allocation_events, p_ressource, AllocationEventFoundSlot::Nil{});
	};

	template<class ShadowPoolHashedCounted(RessourceId, Ressource), class ShadowVector(RessourceAllocationEvent), class RessourceBuilderFunc_t, class RessourceAllocationEventBuilderFunc_t>
	inline static auto allocate_ressource_composition_explicit(
			ShadowPoolHashedCounted(RessourceId, Ressource)& p_pool_hashed_counted,
			ShadowVector(RessourceAllocationEvent)& p_ressource_allocation_events,
			hash_t p_ressource_id,
			const RessourceBuilderFunc_t& p_ressource_builder_func,
			const RessourceAllocationEventBuilderFunc_t& p_ressource_allocation_event_builder_func
	)
	{
		return p_pool_hashed_counted.increment_or_allocate_explicit(p_ressource_id, p_ressource_builder_func,
				[&p_ressource_allocation_events, &p_ressource_allocation_event_builder_func](auto p_allocated_ressource)
				{
					p_ressource_allocation_events.push_back_element(p_ressource_allocation_event_builder_func(p_allocated_ressource));
				});
	};

	/*
	 	The entry point of Ressource deallocation execution flow.
	*/
	template<class ShadowPoolHashedCounted(RessourceId, Ressource), class ShadowVector(RessourceAllocationEvent), class ShadowVector(RessourceFreeEvent), class RessourceFreeEventBuilderFunc_t,
			class AllocationEventFoundSlot_t>
	inline static void free_ressource_composition_explicit(
			ShadowPoolHashedCounted(RessourceId, Ressource)& p_pool_hashed_counted,
			ShadowVector(RessourceAllocationEvent)& p_ressource_allocation_events,
			ShadowVector(RessourceFreeEvent)& p_ressource_free_events,
			const RessourceIdentifiedHeader& p_ressource_header,
			const RessourceFreeEventBuilderFunc_t& p_free_event_builder, // Builds the RessourceFreeEvent if an event has to be pushed
			const AllocationEventFoundSlot_t& p_allocation_event_found_slot // Slot executed when an existing RessourceAllocationEvent with Ressource reference has been found
	)
	{
		if (p_ressource_header.allocated)
		{
			p_pool_hashed_counted.decrement_and_deallocate_pool_not_modified_explicit(p_ressource_header.id,
					[&p_ressource_free_events, &p_free_event_builder](auto p_deallocated_token)
					{
						p_ressource_free_events.sv_push_back_element(p_free_event_builder(p_deallocated_token));
					});
		}
		else
		{
			p_pool_hashed_counted.decrement_and_deallocate_pool_not_modified_explicit(p_ressource_header.id,
					[&p_ressource_allocation_events, &p_pool_hashed_counted, &p_allocation_event_found_slot](auto p_deallocated_token)
					{
						RessourceComposition::remove_reference_from_allocation_events_explicit(p_ressource_allocation_events, p_deallocated_token, p_allocation_event_found_slot);
						p_pool_hashed_counted.pool.sp_release_element(p_deallocated_token);
					});
		}
	};

	/*
		Default version of  free_ressource_composition_explicit
	*/
	template<class ShadowPoolHashedCounted(RessourceId, Ressource), class ShadowVector(RessourceAllocationEvent), class ShadowVector(RessourceFreeEvent), class RessourceFreeEventBuilder>
	inline static void free_ressource_composition(
			ShadowPoolHashedCounted(RessourceId, Ressource)& p_pool_hashed_counted,
			ShadowVector(RessourceAllocationEvent)& p_ressource_allocation_events,
			ShadowVector(RessourceFreeEvent)& p_ressource_free_events,
			const RessourceIdentifiedHeader& p_ressource_header,
			const RessourceFreeEventBuilder& p_free_event_builder)
	{
		RessourceComposition::free_ressource_composition_explicit(p_pool_hashed_counted, p_ressource_allocation_events, p_ressource_free_events, p_ressource_header,
				p_free_event_builder,
				AllocationEventFoundSlot::Nil{});
	};
};