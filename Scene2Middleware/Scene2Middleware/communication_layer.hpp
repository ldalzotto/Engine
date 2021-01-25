#pragma once

namespace v2
{
	namespace BoxColliderComponentAsset_SceneCommunication
	{

		/*
			struct BoxColliderComponentAsset
			{
				v3f half_extend;
			};
		*/

		inline static BoxColliderComponentAsset from_json(JSONDeserializer& p_json_deserialiazer)
		{
			BoxColliderComponentAsset l_return;
			JSONDeserializer l_deserializer;
			p_json_deserialiazer.next_object("half_extend", &l_deserializer);
			l_return.half_extend = MathJSONDeserialization::_v3f(&l_deserializer);
			l_deserializer.free();
			return l_return;
		};

		inline static void to_json(const BoxColliderComponentAsset& p_component, JSONSerializer* in_out_json_serializer)
		{
			in_out_json_serializer->start_object(slice_int8_build_rawstr("half_extend"));
			MathJSONSerialization::_v3f(p_component.half_extend, in_out_json_serializer);
			in_out_json_serializer->end_object();
		};

		inline static NodeComponent construct_nodecomponent(const Token(BoxColliderComponent) p_ressource)
		{
			return NodeComponent{ BoxColliderComponent::Type, tk_v(p_ressource) };
		};

		inline static BoxColliderComponentAsset desconstruct_nodecomponent(SceneMiddleware& p_scene_middleware, const NodeComponent& p_node_component)
		{
			return BoxColliderComponentAsset
			{
				p_scene_middleware.collision_middleware.allocator.box_collider_get_world_half_extend(tk_b(BoxColliderComponent, p_node_component.resource))
			};
		};

		inline static void on_node_component_removed(SceneMiddleware* p_scene_middleware, const NodeComponent& p_node_component)
		{
			p_scene_middleware->collision_middleware.allocator.free_box_collider_component(tk_b(BoxColliderComponent, p_node_component.resource));
		};
	};

#define LIST_OF_COMPONENT \
	X(BoxColliderComponent)

	//global
	inline void on_node_component_removed(SceneMiddleware* p_scene_middleware, const NodeComponent& p_node_component)
	{
		switch (p_node_component.type)
		{

#define X(ComponentType) \
		case ComponentType::Type:\
			ComponentType##Asset_SceneCommunication::on_node_component_removed(p_scene_middleware, p_node_component);\
			break;

			LIST_OF_COMPONENT
#undef X

		default:
			abort();
			break;
		}
	};

#undef LIST_OF_COMPONENT

}