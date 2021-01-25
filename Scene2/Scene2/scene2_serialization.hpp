#pragma once

namespace v2
{

	struct SceneAssetComponent
	{
		component_t type;
		uimax value_size;
		Slice<int8> value;

		inline static SceneAssetComponent build_from_memory(const Slice<int8>& p_memory)
		{
			return SceneAssetComponent{
				*slice_cast_singleelement<component_t>(p_memory),
				*slice_cast_singleelement<uimax>(p_memory.slide_rv(sizeof(SceneAssetComponent::type))),
				p_memory.slide_rv(sizeof(SceneAssetComponent::type) + sizeof(SceneAssetComponent::value_size))
			};
		};
	};

	/*
		The SceneTreeAsset is the representation of the JSON Scene as inserted in the Asset database.
	*/
	struct SceneAsset
	{
		NTree<transform> nodes;
		VaryingVector component_assets;
		VectorOfVector<Token(SliceIndex)> node_to_components;

		inline static SceneAsset allocate_default()
		{
			return SceneAsset{
				NTree<transform>::allocate_default(),
				VaryingVector::allocate_default(),
				VectorOfVector<Token(SliceIndex)>::allocate_default()
			};
		};

		inline void free()
		{
			this->nodes.free();
			this->component_assets.free();
			this->node_to_components.free();
		};

		inline Token(transform) add_node_without_parent(const transform& p_node_local_transform)
		{
			Token(transform) l_node = this->nodes.push_root_value(p_node_local_transform);
			this->node_to_components.push_back();
			return l_node;
		};

		inline Token(transform) add_node(const transform& p_node_local_transform, const Token(transform) p_parent)
		{
			Token(transform) l_node = this->nodes.push_value(p_node_local_transform, p_parent);
			this->node_to_components.push_back();
			return l_node;
		};

		inline void add_component(const Token(transform) p_node, const component_t p_component_type, const Slice<int8>& p_component_value_memory)
		{
			this->component_assets.push_back_empty(sizeof(SceneAssetComponent::type) + sizeof(SceneAssetComponent::value_size) + p_component_value_memory.Size);
			Slice<int8> l_element = this->component_assets.get_last_element();

			slice_memcpy(l_element, Slice<component_t>::build_asint8_memory_singleelement((component_t*)&p_component_type));
			l_element.slide(sizeof(SceneAssetComponent::type));
			slice_memcpy(l_element, Slice<uimax>::build_asint8_memory_singleelement((uimax*)&p_component_value_memory.Size));
			l_element.slide(sizeof(SceneAssetComponent::value_size));
			slice_memcpy(l_element, p_component_value_memory);

			this->node_to_components.element_push_back_element(tk_v(p_node), tk_b(SliceIndex, this->component_assets.get_size() - 1));
		};

		template<class ComponentType>
		inline void add_component_typed(const Token(transform) p_node, const ComponentType& p_component)
		{
			this->add_component(p_node, ComponentType::Type, Slice<ComponentType>::build_asint8_memory_singleelement(&p_component));
		};

		inline Slice<Token(SliceIndex)> get_components(const NTree<transform>::Resolve& p_node)
		{
			return this->node_to_components.get(tk_v(p_node.Node->index));
		};

		template<class ComponentType>
		inline ComponentType* get_component_typed(const Token(SliceIndex) p_component)
		{
			Slice<int8> l_component_element = this->component_assets.get_element(tk_v(p_component));
#if SCENE_BOUND_TEST
			assert_true(*(component_t*)l_component_element.Begin == ComponentType::Type);
#endif
			return (ComponentType*)l_component_element.slide_rv(sizeof(SceneAssetComponent::type) + sizeof(SceneAssetComponent::value_size)).Begin;
		};

		inline SceneAssetComponent get_component(const Token(SliceIndex) p_component)
		{
			return SceneAssetComponent::build_from_memory(this->component_assets.get_element(tk_v(p_component)));
		};

		/*
			Insert the SceneAsset tree to the p_target_scene at the p_parent Node;
			Component ressource allocation is ensured by calling the ComponentResourceAllocatorFunc.

			# token_t ComponentResourceAllocatorFunc(const SceneAssetComponent& p_asset_component);
		*/
		template<class ComponentResourceAllocatorFunc>
		inline void merge_to_scene(Scene* p_target_scene, const Token(Node) p_parent, const ComponentResourceAllocatorFunc& p_component_resource_allocator)
		{
			Span<Token(Node)> l_allocated_nodes = Span<Token(Node)>::allocate(this->nodes.Indices.get_size());
			{
				this->nodes.traverse3(tk_b(NTreeNode, 0), [&](const NTree<transform>::Resolve& p_node) {
					Token(Node) l_parent;
					if (p_node.has_parent())
					{
						l_parent = l_allocated_nodes.get(tk_v(p_node.Node->parent));
					}
					else
					{
						l_parent = p_parent;
					}

					Token(Node) l_allocated_node = p_target_scene->add_node(*p_node.Element, l_parent);
					l_allocated_nodes.get(tk_v(p_node.Node->index)) = l_allocated_node;

					Slice<Token(SliceIndex)> l_components = this->get_components(p_node);
					for (loop(i, 0, l_components.Size))
					{
						SceneAssetComponent l_scene_asset_component = this->get_component(l_components.get(i));
						p_target_scene->add_node_component_by_value(l_allocated_node,
							NodeComponent::build(l_scene_asset_component.type, p_component_resource_allocator(l_scene_asset_component)));;
					}

					});
			}
			l_allocated_nodes.free();
		};

		struct AddComponentAssetToSceneAsset
		{
			SceneAsset* thiz;
			Token(transform) p_scene_asset_node;

			template<class ComponentType>
			inline void add(const ComponentType& p_component) const
			{
				thiz->add_component_typed(p_scene_asset_node, p_component);
			};
		};

		/*
			The inverse of merge_to_scene.
			Copy the scene subtree defined by the p_start_node_included to a fresh new SceneAsset.
		*/
		template<class ComponentResourceDeconstrcutorFunc>
		inline void scene_copied_to(Scene* p_scene, const Token(Node) p_start_node_included, const ComponentResourceDeconstrcutorFunc& p_component_resrouce_deconstructor)
		{
			Span<Token(transform)> l_allocated_nodes = Span<Token(transform)>::allocate(p_scene->tree.node_tree.Indices.get_size());

			{
				NodeEntry& l_node = p_scene->tree.node_tree.get(p_start_node_included);
				Token(transform) l_allocated_node = this->add_node_without_parent(l_node.Element->local_transform);
				l_allocated_nodes.get(tk_v(l_node.Node->index)) = l_allocated_node;
				Slice<NodeComponent> l_components = p_scene->get_node_components(tk_bf(Node, l_node.Node->index));
				for (loop(i, 0, l_components.Size))
				{
					p_component_resrouce_deconstructor(l_components.get(i), AddComponentAssetToSceneAsset{ this, l_allocated_node });
				}

			}


			p_scene->tree.node_tree.traverse3_excluded(tk_bf(NTreeNode, p_start_node_included), [&](const NTree<Node>::Resolve& p_node) {
				Token(transform) l_allocated_node = this->add_node(p_node.Element->local_transform, l_allocated_nodes.get(tk_v(p_node.Node->parent)));
				l_allocated_nodes.get(tk_v(p_node.Node->index)) = l_allocated_node;

				Slice<NodeComponent> l_components = p_scene->get_node_components(tk_bf(Node, p_node.Node->index));
				for (loop(i, 0, l_components.Size))
				{
					p_component_resrouce_deconstructor(l_components.get(i), AddComponentAssetToSceneAsset{ this, l_allocated_node });
				}

				});

			l_allocated_nodes.free();
		};
	};

	namespace SceneSerialization_const
	{
		const Slice<int8> scene_json_type = slice_int8_build_rawstr("scene");
		const int8* scene_nodes_field = "nodes";
		const int8* node_local_position_field = "local_position";
		const int8* node_local_rotation_field = "local_rotation";
		const int8* node_local_scale_field = "local_scale";
		const int8* node_components_field = "components";
		const int8* node_childs_field = "childs";

		const int8* node_component_type_field = "type";
		const int8* node_component_object_field = "object";
	};

	struct SceneJSON_TO_SceneAsset
	{
		/*
			Converts a JSON Scene to a SceneAsset.
			The ComponentDeserializationFunc is used to convert a component JSON to it's ComponentAsset value. So that
			it can be stored in memory.
		*/
		template<class ComponentDeserializationFunc>
		inline static void json_to_SceneAsset(JSONDeserializer& p_scene_deserializer, SceneAsset* out_SceneAssetTree)
		{
			json_get_type_checked(p_scene_deserializer, SceneSerialization_const::scene_json_type);
			build_SceneAsset_from_JSONNodes<ComponentDeserializationFunc>(p_scene_deserializer, out_SceneAssetTree);
			p_scene_deserializer.free();
		};

	private:

		template<class ComponentDeserializationFunc>
		inline static void build_SceneAsset_from_JSONNodes(JSONDeserializer& p_nodes, SceneAsset* in_out_SceneAssetTree)
		{
			struct Stack
			{
				Vector<JSONDeserializer> node_object_iterator;
				Vector<Token(transform)> allocated_nodes;
				Vector<JSONDeserializer> node_childs_serializers;

				inline static Stack allocate_default()
				{
					return Stack{
					 Vector<JSONDeserializer>::allocate(0),
					 Vector<Token(transform)>::allocate(0),
					 Vector<JSONDeserializer>::allocate(0)
					};
				};

				inline void free()
				{
					this->node_object_iterator.free();
					this->allocated_nodes.free();
					this->node_childs_serializers.free();
				};

				inline void push_stack(const Token(transform) p_allocated_node, const JSONDeserializer& p_node_object_iterator)
				{
					this->node_object_iterator.push_back_element(p_node_object_iterator);
					this->allocated_nodes.push_back_element(p_allocated_node);
					this->node_childs_serializers.push_back_element_empty();
				};

				inline void pop_stack()
				{
					this->node_object_iterator.pop_back();
					this->allocated_nodes.pop_back();
					this->node_childs_serializers.pop_back();
				};

				inline JSONDeserializer& get_node_object()
				{
					return this->node_object_iterator.get(this->node_object_iterator.Size - 1);
				};

				inline Token(transform) get_allocated_node()
				{
					return this->allocated_nodes.get(this->allocated_nodes.Size - 1);
				};
			};

			Stack l_stack = Stack::allocate_default();

			in_out_SceneAssetTree->add_node_without_parent(transform_const::ORIGIN);

			json_deser_iterate_array_start(SceneSerialization_const::scene_nodes_field, &p_nodes);
			{

				transform l_transform;
				Token(transform) l_allocated_node;
				JSONDeserializer l_object_iterator = JSONDeserializer::allocate_default();

				l_transform = deserialize_node_transform(l_object, l_object_iterator);
				l_allocated_node = in_out_SceneAssetTree->add_node(l_transform, tk_b(transform, 0));
				deserialize_components<ComponentDeserializationFunc>(l_object, l_object_iterator, l_allocated_node, in_out_SceneAssetTree);

				l_object.next_array(SceneSerialization_const::node_childs_field, &l_object_iterator);

				l_stack.push_stack(l_allocated_node, l_object_iterator.clone());

				while (l_stack.node_object_iterator.Size != 0)
				{

					JSONDeserializer& l_current_node = l_stack.get_node_object();

					JSONDeserializer l_child_node_iterator = JSONDeserializer::allocate_default();
					if (l_current_node.next_array_object(&l_child_node_iterator))
					{
						l_transform = deserialize_node_transform(l_child_node_iterator, l_object_iterator);
						l_allocated_node = in_out_SceneAssetTree->add_node(l_transform, l_stack.get_allocated_node());
						deserialize_components<ComponentDeserializationFunc>(l_child_node_iterator, l_object_iterator, l_allocated_node, in_out_SceneAssetTree);

						l_child_node_iterator.next_array(SceneSerialization_const::node_childs_field, &l_object_iterator);
						l_stack.push_stack(l_allocated_node, l_object_iterator.clone());
					}
					else
					{
						l_stack.pop_stack();
					};
					l_child_node_iterator.free();
				}
			}
			json_deser_iterate_array_end();

			l_stack.free();
		};

		inline static transform deserialize_node_transform(JSONDeserializer& p_node, JSONDeserializer& p_tmp_iterator)
		{
			transform l_tranfsorm;
			json_deser_object(SceneSerialization_const::node_local_position_field, &p_node, &p_tmp_iterator, l_tranfsorm.position, MathJSONDeserialization::_v3f);
			json_deser_object(SceneSerialization_const::node_local_rotation_field, &p_node, &p_tmp_iterator, l_tranfsorm.rotation, MathJSONDeserialization::_quat);
			json_deser_object(SceneSerialization_const::node_local_scale_field, &p_node, &p_tmp_iterator, l_tranfsorm.scale, MathJSONDeserialization::_v3f);
			return l_tranfsorm;
		};

		template<class ComponentDeserializationFunc>
		inline static void deserialize_components(JSONDeserializer& p_node, JSONDeserializer& p_tmp_iterator, const Token(transform) p_node_token, SceneAsset* in_in_out_SceneAssetTreeout_scene)
		{
			json_deser_iterate_array_start(SceneSerialization_const::node_components_field, &p_node);
			{
				json_deser_iterate_array_object.next_field(SceneSerialization_const::node_component_type_field);
				Slice<int8> l_type = json_deser_iterate_array_object.get_currentfield().value;
				JSONDeserializer l_component_object_iterator = JSONDeserializer::allocate_default();
				json_deser_iterate_array_object.next_object(SceneSerialization_const::node_component_object_field, &l_component_object_iterator);
				ComponentDeserializationFunc::push_json_to_sceneassettree(l_component_object_iterator, HashSlice(l_type), p_node_token, in_in_out_SceneAssetTreeout_scene);
				l_component_object_iterator.free();
			}
			json_deser_iterate_array_end();
		};
	};

};