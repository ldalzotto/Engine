#pragma once

namespace v2
{
	struct ShaderModuleRessource
	{
		RessourceIdentifiedHeader header;
		Token(ShaderModule) shader_module;

		inline static ShaderModuleRessource build_inline_from_id(const hash_t p_id)
		{
			return ShaderModuleRessource{
					RessourceIdentifiedHeader::build_inline_with_id(p_id),
					tk_bd(ShaderModule)
			};
		};


		inline static ShaderModuleRessource build_database_from_id(const hash_t p_id)
		{
			return ShaderModuleRessource{
					RessourceIdentifiedHeader::build_database_with_id(p_id),
					tk_bd(ShaderModule)
			};
		};

		struct Asset
		{
			Span<int8> allocated_binary;

			inline void free()
			{
				this->allocated_binary.free();
			};

			struct Value
			{
				Slice<int8> compiled_shader;

				inline static Value build_from_asset(const Asset& p_asset)
				{
					return Value{ p_asset.allocated_binary.slice };
				};
			};

			inline static Asset allocate_from_binary(const Span<int8>& p_allocated_binary)
			{
				return Asset{ p_allocated_binary };
			};

			inline static Asset allocate_from_values(const Value& p_value)
			{
				return Asset{ Span<int8>::allocate_slice(p_value.compiled_shader) };
			};
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
		};

		struct DatabaseAllocationInput
		{
			hash_t id;
		};

		struct AllocationEvent
		{
			Asset RessourceAllocationEvent_member_asset;
			Token(ShaderModuleRessource)RessourceAllocationEvent_member_allocated_ressource;

			inline static AllocationEvent build_inline(const Asset& p_asset, const Token(ShaderModuleRessource) p_allocated_ressource)
			{
				return AllocationEvent{ p_asset, p_allocated_ressource };
			};
		};

		struct FreeEvent
		{
			Token(ShaderModuleRessource) ressource;

			inline static FreeEvent build_from_token(const Token(ShaderModuleRessource) p_token)
			{
				return FreeEvent{ p_token };
			};
		};
	};

	struct MeshRessource
	{
		RessourceIdentifiedHeader header;
		Token(Mesh) mesh;

		inline static MeshRessource build_inline_from_id(const hash_t p_id)
		{
			return MeshRessource{
					RessourceIdentifiedHeader::build_inline_with_id(p_id),
					tk_bd(Mesh)
			};
		};

		inline static MeshRessource build_database_from_id(const hash_t p_id)
		{
			return MeshRessource{
					RessourceIdentifiedHeader::build_database_with_id(p_id),
					tk_bd(Mesh)
			};
		};

		struct Asset
		{
			Span<int8> allocated_binary;

			inline void free()
			{
				this->allocated_binary.free();
			}

			struct Value
			{
				Slice<Vertex> initial_vertices;
				Slice<uint32> initial_indices;

				inline static Value build_from_asset(const Asset& p_asset)
				{
					Value l_value;
					BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
					l_value.initial_vertices = slice_cast<Vertex>(l_deserializer.slice());
					l_value.initial_indices = slice_cast<uint32>(l_deserializer.slice());
					return l_value;
				};
			};

			inline static Asset allocate_from_binary(const Span<int8>& p_allocated_binary)
			{
				return Asset{ p_allocated_binary };
			};

			inline static Asset allocate_from_values(const Value& p_values)
			{
				Vector<int8> l_binary = Vector<int8>::allocate(0);
				BinarySerializer::slice(&l_binary, p_values.initial_vertices.build_asint8());
				BinarySerializer::slice(&l_binary, p_values.initial_indices.build_asint8());
				return allocate_from_binary(l_binary.Memory);
			};
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
		};

		struct DatabaseAllocationInput
		{
			hash_t id;
		};

		struct AllocationEvent
		{
			MeshRessource::Asset RessourceAllocationEvent_member_asset;
			Token(MeshRessource)RessourceAllocationEvent_member_allocated_ressource;

			inline static AllocationEvent build_inline(const MeshRessource::Asset& p_asset, const Token(MeshRessource) p_allocated_ressource)
			{
				return AllocationEvent{ p_asset, p_allocated_ressource };
			};
		};

		struct FreeEvent
		{
			Token(MeshRessource) ressource;

			inline static FreeEvent build_from_token(const Token(MeshRessource) p_ressource)
			{
				return FreeEvent{ p_ressource };
			};
		};
	};

	struct ShaderRessource
	{
		struct Dependencies
		{
			Token(ShaderModuleRessource) vertex_shader;
			Token(ShaderModuleRessource) fragment_shader;
		};

		RessourceIdentifiedHeader header;
		Token(ShaderIndex) shader;
		Dependencies dependencies;

		inline static ShaderRessource build_from_id(const hash_t p_id)
		{
			return ShaderRessource{
					RessourceIdentifiedHeader::build_inline_with_id(p_id),
					tk_bd(ShaderIndex),
					Dependencies{ tk_bd(ShaderModuleRessource), tk_bd(ShaderModuleRessource)}
			};
		};

		struct Asset
		{
			Span<int8> allocated_binary;

			struct Value
			{
				Slice<ShaderLayoutParameterType> specific_parameters;
				uimax execution_order;
				ShaderConfiguration shader_configuration;

				inline static Value build_from_asset(const Asset& p_asset)
				{
					Value l_value;
					BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
					l_value.specific_parameters = slice_cast<ShaderLayoutParameterType>(l_deserializer.slice());
					l_value.execution_order = *l_deserializer.type<uimax>();
					l_value.shader_configuration = *l_deserializer.type<ShaderConfiguration>();
					return l_value;
				};
			};

			inline static Asset allocate_from_binary(const Span<int8>& p_allocated_binary)
			{
				return Asset{ p_allocated_binary };
			};

			inline static Asset allocate_from_values(const Value& p_values)
			{
				Vector<int8> l_binary = Vector<int8>::allocate(0);
				BinarySerializer::slice(&l_binary, p_values.specific_parameters.build_asint8());
				BinarySerializer::type(&l_binary, p_values.execution_order);
				BinarySerializer::type(&l_binary, p_values.shader_configuration);
				return allocate_from_binary(l_binary.Memory);
			};

			inline void free()
			{
				this->allocated_binary.free();
			}
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
		};

		struct DatabaseAllocationInput
		{
			hash_t id;
		};

		struct AllocationEvent
		{
			ShaderRessource::Asset RessourceAllocationEvent_member_asset;
			Token(ShaderRessource)RessourceAllocationEvent_member_allocated_ressource;

			inline static AllocationEvent build_inline(const ShaderRessource::Asset& p_asset, const Token(ShaderRessource) p_allocated_ressource)
			{
				return AllocationEvent{ p_asset, p_allocated_ressource };
			};
		};

		struct FreeEvent
		{
			Token(ShaderRessource) ressource;

			inline static ShaderRessource::FreeEvent build_from_token(const Token(ShaderRessource) p_token)
			{
				return ShaderRessource::FreeEvent{ p_token };
			};
		};
	};

	struct TextureRessource
	{
		RessourceIdentifiedHeader header;
		Token(TextureGPU) texture;

		struct Asset
		{
			Span<int8> allocated_binary;

			struct Value
			{
				v3ui size;
				Slice<int8> pixels;

				inline static Value build_from_asset(const Asset& p_asset)
				{
					Value l_value;
					BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
					l_value.size = *l_deserializer.type<v3ui>();
					l_value.pixels = l_deserializer.slice();
					return l_value;
				};
			};

			inline static Asset allocate_from_binary(const Span<int8>& p_allocated_binary)
			{
				return Asset{ p_allocated_binary };
			};

			inline static Asset allocate_from_values(const Value& p_value)
			{
				Vector<int8> l_binary = Vector<int8>::allocate(0);
				BinarySerializer::type(&l_binary, p_value.size);
				BinarySerializer::slice(&l_binary, p_value.pixels);
				return allocate_from_binary(l_binary.Memory);
			};

			inline void free()
			{
				this->allocated_binary.free();
			};
		};

		struct InlineRessourceInput
		{
			hash_t id;
			Asset asset;
		};

		struct DatabaseRessourceInput
		{
			hash_t id;
		};

		struct AllocationEvent
		{
			Asset RessourceAllocationEvent_member_asset;
			Token(TextureRessource)RessourceAllocationEvent_member_allocated_ressource;
		};

		struct FreeEvent
		{
			Token(TextureRessource) ressource;
		};
	};

	struct MaterialRessource
	{
		struct DynamicDependency
		{
			Token(TextureRessource) dependency;
		};

		struct Dependencies
		{
			Token(ShaderRessource) shader;
			Token(Slice<MaterialRessource::DynamicDependency>) dynamic_dependencies;
		};

		RessourceIdentifiedHeader header;
		Token(Material) material;
		Dependencies dependencies;

		static MaterialRessource build_from_id(const hash_t p_id)
		{
			return MaterialRessource{
					RessourceIdentifiedHeader::build_inline_with_id(p_id),
					tk_bd(Material)
			};
		};

		struct Asset
		{
			Span<int8> allocated_binary;

			struct Value
			{
				VaryingVector parameters; //TODO -> implementing a "header" version of the VaryingVector. Or can't we build a VaryingSlice ? instead of using vectors

				inline static Value build_from_asset(const Asset& p_asset)
				{
					Value l_value;
					BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_asset.allocated_binary.slice);
					l_value.parameters = l_deserializer.varying_vector();
					return l_value;
				};
			};

			inline void free()
			{
				this->allocated_binary.free();
			};

			inline static Asset allocate_from_binary(const Span<int8>& p_allocated_binary)
			{
				return Asset{ p_allocated_binary };
			};

			inline static Asset allocate_from_values(const Value& p_value)
			{
				Vector<int8> l_binary = Vector<int8>::allocate(0);
				BinarySerializer::varying_vector(&l_binary, p_value.parameters);
				return allocate_from_binary(l_binary.Memory);
			};
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
			Slice<TextureRessource::InlineRessourceInput> texture_dependencies_input;
		};

		struct DatabaseAllocationInput
		{
			hash_t id;
			Slice<TextureRessource::DatabaseRessourceInput> texture_dependencies_input;
		};

		struct AllocationEvent
		{
			Asset RessourceAllocationEvent_member_asset;
			Token(MaterialRessource)RessourceAllocationEvent_member_allocated_ressource;

			inline static AllocationEvent build_inline(const Asset& p_asset,
					const Token(MaterialRessource) p_allocated_ressource)
			{
				return AllocationEvent{ p_asset, p_allocated_ressource };
			};
		};

		struct FreeEvent
		{
			Token(MaterialRessource) ressource;
		};

	};

	struct CameraComponent
	{
		static constexpr component_t Type = HashRaw_constexpr(STR(CameraComponent));

		struct Asset
		{
			float32 Near;
			float32 Far;
			float32 Fov;
		};

		int8 allocated;
		int8 force_update;
		Token(Node) scene_node;
		Asset asset;

		inline static CameraComponent build_default()
		{
			return CameraComponent{ 0, 0, tk_bd(Node)};
		};
	};

	struct MeshRendererComponent
	{
		struct Dependencies
		{
			Token(MaterialRessource) material;
			Token(MeshRessource) mesh;
		};

		static constexpr component_t Type = HashRaw_constexpr(STR(MeshRendererComponent));
		int8 allocated;
		int8 force_update;
		Token(Node) scene_node;
		Token(RenderableObject) renderable_object;
		Dependencies dependencies;

		inline static MeshRendererComponent build(const Token(Node) p_scene_node, const Dependencies& p_dependencies)
		{
			return MeshRendererComponent
					{
							0, 1, p_scene_node, tk_bd(RenderableObject), p_dependencies
					};
		};

		struct FreeEvent
		{
			Token(MeshRendererComponent) component;
		};
	};

}