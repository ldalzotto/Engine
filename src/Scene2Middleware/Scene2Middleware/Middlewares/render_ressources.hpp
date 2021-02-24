#pragma once

namespace v2
{
	struct ShaderModuleRessource
	{
		RessourceIdentifiedHeader header;
		Token(ShaderModule) shader_module;

		inline static ShaderModuleRessource build_default()
		{
			return ShaderModuleRessource{
					RessourceIdentifiedHeader::build_default(),
					tk_bd(ShaderModule)
			};
		};

		inline static ShaderModuleRessource build_from_id(const hash_t p_id)
		{
			return ShaderModuleRessource{
					RessourceIdentifiedHeader::build_with_id(p_id),
					tk_bd(ShaderModule)
			};
		};


		struct Asset
		{
			Span<int8> compiled_shader;

			inline void free()
			{
				this->compiled_shader.free();
			};
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
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

		inline static MeshRessource build_default()
		{
			return MeshRessource{
					RessourceIdentifiedHeader::build_default(),
					tk_bd(Mesh)
			};
		};

		inline static MeshRessource build_from_id(const hash_t p_id)
		{
			return MeshRessource{
					RessourceIdentifiedHeader::build_with_id(p_id),
					tk_bd(Mesh)
			};
		};

		struct Asset
		{
			Span<Vertex> initial_vertices;
			Span<uint32> initial_indices;

			inline void free()
			{
				this->initial_vertices.free();
				this->initial_indices.free();
			}
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
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
					RessourceIdentifiedHeader::build_with_id(p_id),
					tk_bd(ShaderIndex),
					Dependencies{ tk_bd(ShaderModuleRessource), tk_bd(ShaderModuleRessource)}
			};
		};

		struct Asset
		{
			Span<ShaderLayoutParameterType> specific_parameters;
			uimax execution_order;
			ShaderConfiguration shader_configuration;

			inline void free()
			{
				this->specific_parameters.free();
			}
		};

		struct InlineAllocationInput
		{
			hash_t id;
			Asset asset;
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

	struct MaterialRessource
	{
		struct Dependencies
		{
			Token(ShaderRessource) shader;
		};

		RessourceIdentifiedHeader header;
		Token(Material) material;
		Dependencies dependencies;

		inline static MaterialRessource build_default()
		{
			return MaterialRessource{
					RessourceIdentifiedHeader::build_default(),
					tk_bd(Material),
					Dependencies{ tk_bd(ShaderRessource)}
			};
		};

		static MaterialRessource build_from_id(const hash_t p_id)
		{
			return MaterialRessource{
					RessourceIdentifiedHeader::build_with_id(p_id),
					tk_bd(Material)
			};
		};

		struct Asset
		{
			//TODO adding parameters
			// Material allocation can be done by accepting an array of input parameters. This array will be in this structure to be used.
		};

		struct InlineRessourceInput
		{
			hash_t id;
			Asset asset;
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

	struct CameraComponentAsset
	{
		float32 Near;
		float32 Far;
		float32 Fov;
	};

	struct CameraComponent
	{
		static constexpr component_t Type = HashRaw_constexpr(STR(CameraComponent));
		int8 force_update;
		Token(Node) scene_node;
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