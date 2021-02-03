#pragma once

namespace v2
{


	enum class ShaderLayoutParameterType
	{
		UNDEFINED = 0,
		UNIFORM_BUFFER_VERTEX = 1,
		UNIFORM_BUFFER_VERTEX_FRAGMENT = 2,
		TEXTURE_FRAGMENT = 3,
	};

	struct ShaderLayoutParameters
	{
		VkDescriptorSetLayout uniformbuffer_vertex_layout;
		VkDescriptorSetLayout uniformbuffer_fragment_vertex_layout;
		VkDescriptorSetLayout texture_fragment_layout;

		static ShaderLayoutParameters allocate(gc_t const p_device);

		void free(gc_t const p_device);

		VkDescriptorSetLayout get_descriptorset_layout(const ShaderLayoutParameterType p_shader_layout_parameter_type) const;

	private:
		VkDescriptorSetLayout create_layout(gc_t const p_device, const ShaderLayoutParameterType p_shader_layout_parameter_type);

		VkDescriptorSetLayoutBinding create_binding(const uint32 p_binding, const VkDescriptorType p_descriptor_type, const VkShaderStageFlags p_shader_stage);
	};


	struct GraphicsDevice
	{
		GraphicsCard graphics_card;
		gc_t device;
		gcqueue_t graphics_queue;

		CommandPool command_pool;
		CommandBuffer command_buffer;

		ShaderLayoutParameters shaderlayout_parameters;

		static GraphicsDevice allocate(GPUInstance& p_instance);

		void free();
	};

	typedef VkSampler TextureSampler;

	struct TextureSamplers
	{
		TextureSampler Default;

		static TextureSamplers build(const gc_t p_device);
	};

	typedef VkImageView ImageView_t;

	struct TextureGPU
	{
		Token(ImageGPU) Image;
		ImageView_t ImageView;

		static TextureGPU allocate(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format);

		void free(BufferAllocator& p_buffer_allocator);

	};

	typedef VkRenderPass RenderPass_t;

	enum class AttachmentType
	{
		COLOR, DEPTH
	};

	struct RenderPass
	{
		RenderPass_t render_pass;

		template<uint8 AttachmentCount>
		static RenderPass allocate(const GraphicsDevice& p_device, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount]);

		void free(const GraphicsDevice& p_device);

	};

	typedef VkFramebuffer FrameBuffer_t;

	struct GraphicsPass
	{
		RenderPass render_pass;
		Token(Slice<Token(TextureGPU)>) attachment_textures;
		FrameBuffer_t frame_buffer;
	};

	typedef VkPipelineLayout ShaderLayout_t;

	struct ShaderLayout
	{
		ShaderLayout_t layout;
		//TODO -> this Span will be used for validation
		Span<ShaderLayoutParameterType> shader_layout_parameter_types;
		Span<PrimitiveSerializedTypes::Type> vertex_input_layout;

		static ShaderLayout allocate(const GraphicsDevice& p_device, const Span<ShaderLayoutParameterType>& in_shaderlayout_parameter_types, const Span<PrimitiveSerializedTypes::Type>& in_vertex_input_layout);

		void free(const GraphicsDevice& p_device);
	};

	typedef VkShaderModule ShaderModule_t;

	struct ShaderModule
	{
		ShaderModule_t module;

		static ShaderModule allocate(const GraphicsDevice& p_graphics_device, const Slice<int8>& p_shader_compiled_str);

		void free(const GraphicsDevice& p_graphics_device);
	};

	struct ShaderConfiguration
	{
		enum class CompareOp
		{
			Never = 0,
			Less = 1,
			Equal = 2,
			LessOrEqual = 3,
			Greater = 4,
			NotEqual = 5,
			GreaterOrEqual = 6,
			Always = 7,
			Invalid = 8
		};

		char zwrite;
		CompareOp ztest;
	};

	struct ShaderAllocateInfo
	{
		GraphicsPass& p_graphics_pass;
		ShaderConfiguration& shader_configuration;
		ShaderLayout& shader_layout;
		ShaderModule vertex_shader;
		ShaderModule fragment_shader;
	};

	struct Shader
	{
		VkPipeline shader;
		ShaderLayout layout;

		static Shader allocate(const GraphicsDevice& p_device, const ShaderAllocateInfo& p_shader_allocate_info);

		void free(const GraphicsDevice& p_device);

	private:
		static VkFormat get_primitivetype_format(const PrimitiveSerializedTypes::Type p_primitive_type);
	};

	struct GraphicsAllocator
	{
		GraphicsDevice graphicsDevice;

		Pool <TextureGPU> textures_gpu;
		PoolOfVector <Token(TextureGPU)> renderpass_attachment_textures;
		Pool <GraphicsPass> graphics_pass;
		Pool <ShaderLayout> shader_layouts;
		Pool <ShaderModule> shader_modules;

		static GraphicsAllocator allocate_default(GPUInstance& p_instance);

		void free();

		Token(TextureGPU) allocate_texturegpu(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format);

		void free_texturegpu(BufferAllocator& p_buffer_allocator, const Token(TextureGPU) p_texture_gpu);

		template<uint8 AttachmentCount>
		Token(GraphicsPass) allocate_graphicspass(BufferAllocator& p_buffer_allocator, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount]);

		void free_graphicspass(BufferAllocator& p_buffer_allocator, const Token(GraphicsPass) p_graphics_pass);

		void cmd_beginRenderPass2(BufferAllocator& p_buffer_allocator, const GraphicsPass& p_graphics_pass, const Slice<v4f>& p_clear_values);

		void cmd_endRenderPass(const GraphicsPass& p_graphics_pass);

		Token(ShaderLayout) allocate_shader_layout(Span<ShaderLayoutParameterType>& in_shaderlayout_parameter_types, Span<PrimitiveSerializedTypes::Type>& in_vertex_input_layout);

		void free_shader_layout(const Token(ShaderLayout) p_shader_layout);

		Token(ShaderModule) allocate_shader_module(const Slice<int8>& p_shader_compiled_str);

		void free_shader_module(const Token(ShaderModule) p_shader_module);

	private:

		template<uint8 AttachmentCount>
		GraphicsPass _allocate_graphics_pass(BufferAllocator& p_buffer_allocator, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount]);

		void _free_graphics_pass(BufferAllocator& p_buffer_allocator, GraphicsPass& p_graphics_pass);
	};

	inline ShaderLayoutParameters ShaderLayoutParameters::allocate(gc_t const p_device)
	{
		ShaderLayoutParameters l_shader_layout_parameters;
		l_shader_layout_parameters.uniformbuffer_vertex_layout = l_shader_layout_parameters.create_layout(p_device, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX);
		l_shader_layout_parameters.uniformbuffer_fragment_vertex_layout = l_shader_layout_parameters.create_layout(p_device, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT);
		l_shader_layout_parameters.texture_fragment_layout = l_shader_layout_parameters.create_layout(p_device, ShaderLayoutParameterType::TEXTURE_FRAGMENT);
		return l_shader_layout_parameters;
	};

	inline void ShaderLayoutParameters::free(gc_t const p_device)
	{
		vkDestroyDescriptorSetLayout(p_device, this->uniformbuffer_vertex_layout, NULL);
		vkDestroyDescriptorSetLayout(p_device, this->uniformbuffer_fragment_vertex_layout, NULL);
		vkDestroyDescriptorSetLayout(p_device, this->texture_fragment_layout, NULL);

	};

	inline VkDescriptorSetLayout ShaderLayoutParameters::get_descriptorset_layout(const ShaderLayoutParameterType p_shader_layout_parameter_type) const
	{
		switch (p_shader_layout_parameter_type)
		{
		case ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX:
			return this->uniformbuffer_vertex_layout;
		case ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT:
			return this->uniformbuffer_fragment_vertex_layout;
		case ShaderLayoutParameterType::TEXTURE_FRAGMENT:
			return this->texture_fragment_layout;
		default:
			abort();
		}
	};

	inline VkDescriptorSetLayout ShaderLayoutParameters::create_layout(gc_t const p_device, const ShaderLayoutParameterType p_shader_layout_parameter_type)
	{
		VkDescriptorType l_descriptor_type;
		VkShaderStageFlags l_shader_stage;
		switch (p_shader_layout_parameter_type)
		{
		case ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX:
			l_descriptor_type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			l_shader_stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT:
			l_descriptor_type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			l_shader_stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case ShaderLayoutParameterType::TEXTURE_FRAGMENT:
			l_descriptor_type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			l_shader_stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		default:
			abort();
		}

		VkDescriptorSetLayoutBinding l_binding = this->create_binding(0, l_descriptor_type, l_shader_stage);

		VkDescriptorSetLayoutCreateInfo l_descriptorset_layot_create{};
		l_descriptorset_layot_create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		l_descriptorset_layot_create.bindingCount = 1;
		l_descriptorset_layot_create.pBindings = &l_binding;

		VkDescriptorSetLayout l_layout;
		vk_handle_result(vkCreateDescriptorSetLayout(p_device, &l_descriptorset_layot_create, NULL, &l_layout));
		return l_layout;
	};

	inline VkDescriptorSetLayoutBinding ShaderLayoutParameters::create_binding(const uint32 p_binding, const VkDescriptorType p_descriptor_type, const VkShaderStageFlags p_shader_stage)
	{
		VkDescriptorSetLayoutBinding l_camera_matrices_layout_binding;
		l_camera_matrices_layout_binding.binding = p_binding;
		l_camera_matrices_layout_binding.descriptorCount = 1;
		l_camera_matrices_layout_binding.descriptorType = p_descriptor_type;
		l_camera_matrices_layout_binding.stageFlags = p_shader_stage;
		l_camera_matrices_layout_binding.pImmutableSamplers = nullptr;
		return l_camera_matrices_layout_binding;
	};

	inline GraphicsDevice GraphicsDevice::allocate(GPUInstance& p_instance)
	{
		GraphicsDevice l_graphics_device;
		l_graphics_device.graphics_card = p_instance.graphics_card;
		l_graphics_device.device = p_instance.logical_device;

		vkGetDeviceQueue(l_graphics_device.device, p_instance.graphics_card.graphics_queue_family, 0, &l_graphics_device.graphics_queue);

		l_graphics_device.command_pool = CommandPool::allocate(l_graphics_device.device, p_instance.graphics_card.graphics_queue_family);
		l_graphics_device.command_buffer = l_graphics_device.command_pool.allocate_command_buffer(l_graphics_device.device, l_graphics_device.graphics_queue);

		l_graphics_device.shaderlayout_parameters = ShaderLayoutParameters::allocate(l_graphics_device.device);

		return l_graphics_device;
	};

	inline void GraphicsDevice::free()
	{
		this->command_pool.free_command_buffer(this->device, this->command_buffer);
		this->command_pool.free(this->device);

		this->shaderlayout_parameters.free(this->device);
	};

	inline TextureSamplers TextureSamplers::build(const gc_t p_device)
	{
		TextureSamplers l_samplers;

		VkSamplerCreateInfo l_sampler_create_info;
		l_sampler_create_info.magFilter = VkFilter::VK_FILTER_NEAREST;
		l_sampler_create_info.minFilter = VkFilter::VK_FILTER_NEAREST;
		l_sampler_create_info.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		l_sampler_create_info.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		l_sampler_create_info.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		l_sampler_create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		l_sampler_create_info.unnormalizedCoordinates = 0;
		l_sampler_create_info.compareEnable = false;
		l_sampler_create_info.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
		l_sampler_create_info.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
		l_sampler_create_info.mipLodBias = 0.0f;
		l_sampler_create_info.maxLod = 0.0f;
		l_sampler_create_info.minLod = 0.0f;

		vk_handle_result(vkCreateSampler(p_device, &l_sampler_create_info, NULL, &l_samplers.Default));
		return l_samplers;
	};

	inline TextureGPU TextureGPU::allocate(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format)
	{
		TextureGPU l_texture_gpu;
		l_texture_gpu.Image = p_buffer_allocator.allocate_imagegpu(p_image_format);

		ImageGPU& l_image_gpu = p_buffer_allocator.gpu_images.get(l_texture_gpu.Image);

		VkImageViewCreateInfo l_imageview_create_info{};
		l_imageview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		l_imageview_create_info.image = l_image_gpu.image;
		switch (p_image_format.imageType)
		{
		case VkImageType::VK_IMAGE_TYPE_2D:
			l_imageview_create_info.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
			break;
		default:
			abort();
		}
		l_imageview_create_info.format = p_image_format.format;
		l_imageview_create_info.subresourceRange = VkImageSubresourceRange{
				p_image_format.imageAspect,
				0,
				(uint32_t)p_image_format.mipLevels,
				0,
				(uint32_t)p_image_format.arrayLayers
		};

		vk_handle_result(vkCreateImageView(p_buffer_allocator.device.device, &l_imageview_create_info, NULL, &l_texture_gpu.ImageView));

		return l_texture_gpu;
	};

	inline void TextureGPU::free(BufferAllocator& p_buffer_allocator)
	{
		vkDestroyImageView(p_buffer_allocator.device.device, this->ImageView, NULL);
		p_buffer_allocator.free_imagegpu(this->Image);
	};

	template<uint8 AttachmentCount>
	inline RenderPass RenderPass::allocate(const GraphicsDevice& p_device, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount])
	{

#if GPU_DEBUG
		assert_true(AttachmentCount >= 1);
#endif

		VkAttachmentDescription l_attachments_raw[AttachmentCount];
		Slice<VkAttachmentDescription> l_attachments = Slice<VkAttachmentDescription>::build_memory_elementnb(l_attachments_raw, AttachmentCount);

		VkAttachmentReference l_color_attachments_ref_raw[AttachmentCount];
		Slice<VkAttachmentReference> l_color_attachments_ref = Slice<VkAttachmentReference>::build_memory_elementnb(l_color_attachments_ref_raw, AttachmentCount);
		uint8 l_color_attachments_ref_count = 0;

		VkAttachmentReference l_depth_attachment_reference;

		VkSubpassDescription l_subpass = VkSubpassDescription{};

		for (loop(i, 0, AttachmentCount))
		{
			switch (p_attachment_types[i])
			{
			case AttachmentType::COLOR:
			{
				VkAttachmentDescription& l_color_attachment = l_attachments.get(i);
				l_color_attachment = VkAttachmentDescription{};
				l_color_attachment.format = p_attachment_formats[i].format;
				l_color_attachment.samples = p_attachment_formats[i].samples;
				l_color_attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
				l_color_attachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
				l_color_attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
				l_color_attachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
				l_color_attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				l_color_attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkAttachmentReference& l_attachment_reference = l_color_attachments_ref.get(l_color_attachments_ref_count);
				l_attachment_reference.attachment = (uint32)i;
				l_attachment_reference.layout = l_color_attachment.finalLayout;
				l_color_attachments_ref_count += 1;
			}
				break;
			case AttachmentType::DEPTH:
			{
				VkAttachmentDescription& l_color_attachment = l_attachments.get(i);
				l_color_attachment = VkAttachmentDescription{};
				l_color_attachment.format = p_attachment_formats[i].format;
				l_color_attachment.samples = p_attachment_formats[i].samples;
				l_color_attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
				l_color_attachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
				l_color_attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
				l_color_attachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
				l_color_attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				l_color_attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				l_depth_attachment_reference = VkAttachmentReference{};
				l_depth_attachment_reference.attachment = (uint32)i;
				l_depth_attachment_reference.layout = l_color_attachment.finalLayout;
				l_subpass.pDepthStencilAttachment = &l_depth_attachment_reference;
			}
				break;
			}
		}

		l_subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		l_subpass.colorAttachmentCount = l_color_attachments_ref_count;
		l_subpass.pColorAttachments = l_color_attachments_ref.Begin;


		VkRenderPassCreateInfo l_renderpass_create_info{};
		l_renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		l_renderpass_create_info.attachmentCount = (uint32)l_attachments.Size;
		l_renderpass_create_info.pAttachments = l_attachments.Begin;
		l_renderpass_create_info.subpassCount = 1;
		l_renderpass_create_info.pSubpasses = &l_subpass;

		RenderPass l_render_pass_object;

		vk_handle_result(vkCreateRenderPass(p_device.device, &l_renderpass_create_info, NULL, &l_render_pass_object.render_pass));
		return l_render_pass_object;

	};

	inline void RenderPass::free(const GraphicsDevice& p_device)
	{
		vkDestroyRenderPass(p_device.device, this->render_pass, NULL);
	};

	inline ShaderLayout ShaderLayout::allocate(const GraphicsDevice& p_device, const Span<ShaderLayoutParameterType>& in_shaderlayout_parameter_types, const Span<PrimitiveSerializedTypes::Type>& in_vertex_input_layout)
	{
		VkPipelineLayoutCreateInfo l_pipeline_create_info{};
		l_pipeline_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		Span<VkDescriptorSetLayout> l_descriptor_set_layouts = Span<VkDescriptorSetLayout>::allocate(in_shaderlayout_parameter_types.Capacity);

		for (loop(i, 0, in_shaderlayout_parameter_types.Capacity))
		{
			l_descriptor_set_layouts.get(i) = p_device.shaderlayout_parameters.get_descriptorset_layout(in_shaderlayout_parameter_types.get(i));
		}

		l_pipeline_create_info.setLayoutCount = (uint32_t)l_descriptor_set_layouts.Capacity;
		l_pipeline_create_info.pSetLayouts = l_descriptor_set_layouts.Memory;

		ShaderLayout l_shader_layout;
		l_shader_layout.shader_layout_parameter_types = in_shaderlayout_parameter_types;
		l_shader_layout.vertex_input_layout = in_vertex_input_layout;
		vk_handle_result(vkCreatePipelineLayout(p_device.device, &l_pipeline_create_info, NULL, &l_shader_layout.layout));

		l_descriptor_set_layouts.free();

		return l_shader_layout;
	};

	inline void ShaderLayout::free(const GraphicsDevice& p_device)
	{
		vkDestroyPipelineLayout(p_device.device, this->layout, NULL);
		this->shader_layout_parameter_types.free();
		this->vertex_input_layout.free();
	};

	inline ShaderModule ShaderModule::allocate(const GraphicsDevice& p_graphics_device, const Slice<int8>& p_shader_compiled_str)
	{
		VkShaderModuleCreateInfo l_shader_module_create_info{};
		l_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		l_shader_module_create_info.codeSize = p_shader_compiled_str.Size;
		l_shader_module_create_info.pCode = (uint32_t*)p_shader_compiled_str.Begin;

		ShaderModule l_shader_module;
		vk_handle_result(vkCreateShaderModule(p_graphics_device.device, &l_shader_module_create_info, NULL, &l_shader_module.module));
		return l_shader_module;
	};

	inline void ShaderModule::free(const GraphicsDevice& p_graphics_device)
	{
		vkDestroyShaderModule(p_graphics_device.device, this->module, NULL);
	};

	inline Shader Shader::allocate(const GraphicsDevice& p_device, const ShaderAllocateInfo& p_shader_allocate_info)
	{
		Shader l_shader;
		l_shader.layout = p_shader_allocate_info.shader_layout;

		Span<VkVertexInputAttributeDescription> l_vertex_input_attributes = Span<VkVertexInputAttributeDescription>::allocate(p_shader_allocate_info.shader_layout.vertex_input_layout.Capacity);

		VkGraphicsPipelineCreateInfo l_pipeline_graphcis_create_info{};
		l_pipeline_graphcis_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		l_pipeline_graphcis_create_info.layout = p_shader_allocate_info.shader_layout.layout;
		l_pipeline_graphcis_create_info.renderPass = p_shader_allocate_info.p_graphics_pass.render_pass.render_pass;

		VkPipelineInputAssemblyStateCreateInfo l_inputassembly_state{};
		l_inputassembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		l_inputassembly_state.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		l_inputassembly_state.primitiveRestartEnable = 0;

		VkPipelineRasterizationStateCreateInfo l_rasterization_state;
		l_rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		l_rasterization_state.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
		l_rasterization_state.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
		l_rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		l_rasterization_state.lineWidth = 1.0f;
		l_rasterization_state.depthClampEnable = 0;
		l_rasterization_state.rasterizerDiscardEnable = 0;
		l_rasterization_state.depthClampEnable = 0;


		VkPipelineColorBlendAttachmentState l_blendattachment_state;
		l_blendattachment_state.colorWriteMask = 0xf;
		l_blendattachment_state.blendEnable = 0;
		VkPipelineColorBlendStateCreateInfo l_blendattachment_state_create{};
		l_blendattachment_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		l_blendattachment_state_create.attachmentCount = 1;
		l_blendattachment_state_create.pAttachments = &l_blendattachment_state;

		VkPipelineViewportStateCreateInfo l_viewport_state{};
		l_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		l_viewport_state.viewportCount = 1;
		l_viewport_state.scissorCount = 1;

		VkDynamicState l_dynamicstates_enabled[2];
		l_dynamicstates_enabled[0] = VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT;
		l_dynamicstates_enabled[1] = VkDynamicState::VK_DYNAMIC_STATE_SCISSOR;
		VkPipelineDynamicStateCreateInfo l_dynamicstates{};
		l_dynamicstates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		l_dynamicstates.dynamicStateCount = 2;
		l_dynamicstates.pDynamicStates = l_dynamicstates_enabled;


		VkPipelineDepthStencilStateCreateInfo l_depthstencil_state{};
		l_depthstencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		if (p_shader_allocate_info.shader_configuration.ztest != ShaderConfiguration::CompareOp::Invalid)
		{
			l_depthstencil_state.depthTestEnable = 1;
			l_depthstencil_state.depthWriteEnable = p_shader_allocate_info.shader_configuration.zwrite;
			l_depthstencil_state.depthCompareOp = (VkCompareOp)p_shader_allocate_info.shader_configuration.ztest;
			l_depthstencil_state.depthBoundsTestEnable = 0;

			VkStencilOpState l_back{};
			l_back.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
			l_back.failOp = VkStencilOp::VK_STENCIL_OP_KEEP;
			l_back.passOp = VkStencilOp::VK_STENCIL_OP_KEEP;
			l_depthstencil_state.back = l_back;
			l_depthstencil_state.front = l_back;
			l_depthstencil_state.stencilTestEnable = 0;

			VkPipelineMultisampleStateCreateInfo l_multisample_state{};
			l_multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			l_multisample_state.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;


			uimax l_vertex_input_total_size = 0;
			for (loop(i, 0, l_vertex_input_attributes.Capacity))
			{
				l_vertex_input_attributes.get(i) = VkVertexInputAttributeDescription{
						0, (uint32_t)i, get_primitivetype_format(p_shader_allocate_info.shader_layout.vertex_input_layout.get(i)), (uint32_t)l_vertex_input_total_size
				};

				l_vertex_input_total_size += PrimitiveSerializedTypes::get_size(p_shader_allocate_info.shader_layout.vertex_input_layout.get(i));


			}

			VkVertexInputBindingDescription l_vertex_input_binding{ 0, (uint32_t)l_vertex_input_total_size, VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX };

			VkPipelineVertexInputStateCreateInfo l_vertex_input_create{};
			l_vertex_input_create.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			l_vertex_input_create.vertexBindingDescriptionCount = 1;
			l_vertex_input_create.pVertexBindingDescriptions = &l_vertex_input_binding;
			l_vertex_input_create.pVertexAttributeDescriptions = l_vertex_input_attributes.Memory;
			l_vertex_input_create.vertexAttributeDescriptionCount = (uint32_t)l_vertex_input_attributes.Capacity;


			VkPipelineShaderStageCreateInfo l_shader_stages[2];

			VkPipelineShaderStageCreateInfo l_vertex_stage{};
			l_vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			l_vertex_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			l_vertex_stage.module = p_shader_allocate_info.vertex_shader.module;
			l_vertex_stage.pName = "main";


			VkPipelineShaderStageCreateInfo l_fragment_stage{};
			l_fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			l_fragment_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			l_fragment_stage.module = p_shader_allocate_info.fragment_shader.module;
			l_fragment_stage.pName = "main";

			l_shader_stages[0] = l_vertex_stage;
			l_shader_stages[1] = l_fragment_stage;

			l_pipeline_graphcis_create_info.stageCount = 2;
			l_pipeline_graphcis_create_info.pStages = l_shader_stages;


			l_pipeline_graphcis_create_info.pVertexInputState = &l_vertex_input_create;
			l_pipeline_graphcis_create_info.pInputAssemblyState = &l_inputassembly_state;
			l_pipeline_graphcis_create_info.pRasterizationState = &l_rasterization_state;
			l_pipeline_graphcis_create_info.pColorBlendState = &l_blendattachment_state_create;
			l_pipeline_graphcis_create_info.pMultisampleState = &l_multisample_state;
			l_pipeline_graphcis_create_info.pViewportState = &l_viewport_state;

			if (p_shader_allocate_info.shader_configuration.ztest != ShaderConfiguration::CompareOp::Invalid)
			{
				l_pipeline_graphcis_create_info.pDepthStencilState = &l_depthstencil_state;
			}
			l_pipeline_graphcis_create_info.pDynamicState = &l_dynamicstates;

			vk_handle_result(vkCreateGraphicsPipelines(p_device.device, VkPipelineCache{}, 1, &l_pipeline_graphcis_create_info, NULL, &l_shader.shader));
		}

		return l_shader;
	};

	inline void Shader::free(const GraphicsDevice& p_device)
	{
		this->layout.free(p_device);
		vkDestroyPipeline(p_device.device, this->shader, NULL);
	};

	inline VkFormat Shader::get_primitivetype_format(const PrimitiveSerializedTypes::Type p_primitive_type)
	{
		switch (p_primitive_type)
		{
		case PrimitiveSerializedTypes::Type::FLOAT32:
			return VkFormat::VK_FORMAT_R32_SFLOAT;
		case PrimitiveSerializedTypes::Type::FLOAT32_2:
			return VkFormat::VK_FORMAT_R32G32_SFLOAT;
		case PrimitiveSerializedTypes::Type::FLOAT32_3:
			return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		case PrimitiveSerializedTypes::Type::FLOAT32_4:
			return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			abort();
		}

	};

	inline GraphicsAllocator GraphicsAllocator::allocate_default(GPUInstance& p_instance)
	{
		return GraphicsAllocator{
				GraphicsDevice::allocate(p_instance),
				Pool<TextureGPU>::allocate(0),
				PoolOfVector<Token(TextureGPU) >::allocate_default(),
				Pool<GraphicsPass>::allocate(0),
				Pool<ShaderLayout>::allocate(0),
				Pool<ShaderModule>::allocate(0)
		};
	};

	inline void GraphicsAllocator::free()
	{
#if GPU_DEBUG
		assert_true(!this->textures_gpu.has_allocated_elements());
		assert_true(!this->renderpass_attachment_textures.has_allocated_elements());
		assert_true(!this->graphics_pass.has_allocated_elements());
		assert_true(!this->shader_layouts.has_allocated_elements());
		assert_true(!this->shader_modules.has_allocated_elements());
#endif

		this->textures_gpu.free();
		this->renderpass_attachment_textures.free();
		this->graphicsDevice.free();
		this->graphics_pass.free();
		this->shader_layouts.free();
		this->shader_modules.free();
	};

	inline Token(TextureGPU) GraphicsAllocator::allocate_texturegpu(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format)
	{
		return this->textures_gpu.alloc_element(TextureGPU::allocate(p_buffer_allocator, p_image_format));
	};

	inline void GraphicsAllocator::free_texturegpu(BufferAllocator& p_buffer_allocator, const Token(TextureGPU) p_texture_gpu)
	{
		TextureGPU& l_texture_gpu = this->textures_gpu.get(p_texture_gpu);
		l_texture_gpu.free(p_buffer_allocator);
		this->textures_gpu.release_element(p_texture_gpu);
	};

	template<uint8 AttachmentCount>
	inline Token(GraphicsPass) GraphicsAllocator::allocate_graphicspass(BufferAllocator& p_buffer_allocator, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount])
	{
		return this->graphics_pass.alloc_element(
				_allocate_graphics_pass<AttachmentCount>(p_buffer_allocator, p_attachment_types, p_attachment_formats)
		);
	};

	inline void GraphicsAllocator::free_graphicspass(BufferAllocator& p_buffer_allocator, const Token(GraphicsPass) p_graphics_pass)
	{
		GraphicsPass& l_graphics_pass = this->graphics_pass.get(p_graphics_pass);
		_free_graphics_pass(p_buffer_allocator, l_graphics_pass);
		this->graphics_pass.release_element(p_graphics_pass);
	};

	inline void GraphicsAllocator::cmd_beginRenderPass2(BufferAllocator& p_buffer_allocator, const GraphicsPass& p_graphics_pass, const Slice<v4f>& p_clear_values)
	{
		VkRenderPassBeginInfo l_renderpass_begin{};
		l_renderpass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		l_renderpass_begin.renderPass = p_graphics_pass.render_pass.render_pass;

		Slice<Token(TextureGPU) > l_attachments = this->renderpass_attachment_textures.get_vector(p_graphics_pass.attachment_textures);
		ImageFormat& l_target_format = p_buffer_allocator.gpu_images.get(this->textures_gpu.get(l_attachments.get(0)).Image).format;

		l_renderpass_begin.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, VkExtent2D{ (uint32_t)l_target_format.extent.x, (uint32_t)l_target_format.extent.y }};
		l_renderpass_begin.clearValueCount = (uint32)p_clear_values.Size;
		l_renderpass_begin.pClearValues = (VkClearValue*)p_clear_values.Begin;

		l_renderpass_begin.framebuffer = p_graphics_pass.frame_buffer;

		VkViewport l_viewport{};
		l_viewport.width = (float)l_target_format.extent.x;
		l_viewport.height = (float)l_target_format.extent.y;
		l_viewport.minDepth = 0.0f;
		l_viewport.maxDepth = 1.0f;

		VkRect2D l_windowarea = VkRect2D{ VkOffset2D{ 0, 0 }, VkExtent2D{ (uint32_t)l_target_format.extent.x, (uint32_t)l_target_format.extent.y }};
		vkCmdSetViewport(this->graphicsDevice.command_buffer.command_buffer, 0, 1, &l_viewport);
		vkCmdSetScissor(this->graphicsDevice.command_buffer.command_buffer, 0, 1, &l_windowarea);

		vkCmdBeginRenderPass(this->graphicsDevice.command_buffer.command_buffer, &l_renderpass_begin, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
	};

	inline void GraphicsAllocator::cmd_endRenderPass(const GraphicsPass& p_graphics_pass)
	{
		vkCmdEndRenderPass(this->graphicsDevice.command_buffer.command_buffer);
	};

	inline Token(ShaderLayout) GraphicsAllocator::allocate_shader_layout(Span<ShaderLayoutParameterType>& in_shaderlayout_parameter_types, Span<PrimitiveSerializedTypes::Type>& in_vertex_input_layout)
	{
		return this->shader_layouts.alloc_element(ShaderLayout::allocate(this->graphicsDevice, in_shaderlayout_parameter_types.move_to_value(), in_vertex_input_layout.move_to_value()));
	};

	inline void GraphicsAllocator::free_shader_layout(const Token(ShaderLayout) p_shader_layout)
	{
		ShaderLayout& l_shader_layout = this->shader_layouts.get(p_shader_layout);
		l_shader_layout.free(this->graphicsDevice);
		this->shader_layouts.release_element(p_shader_layout);
	};

	inline Token(ShaderModule) GraphicsAllocator::allocate_shader_module(const Slice<int8>& p_shader_compiled_str)
	{
		return this->shader_modules.alloc_element(ShaderModule::allocate(this->graphicsDevice, p_shader_compiled_str));
	};

	inline void GraphicsAllocator::free_shader_module(const Token(ShaderModule) p_shader_module)
	{
		ShaderModule& l_shader_module = this->shader_modules.get(p_shader_module);
		l_shader_module.free(this->graphicsDevice);
		this->shader_modules.release_element(p_shader_module);
	};

	template<uint8 AttachmentCount>
	inline GraphicsPass GraphicsAllocator::_allocate_graphics_pass(BufferAllocator& p_buffer_allocator,
			const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount])
	{

#if GPU_DEBUG
		v3ui l_extend = p_attachment_formats[0].extent;
		for (loop(i, 1, AttachmentCount))
		{
			assert_true(p_attachment_formats[i].extent == l_extend);
		}
#endif

		RenderPass l_render_pass = RenderPass::allocate<2>(this->graphicsDevice, p_attachment_types, p_attachment_formats);
		TextureGPU l_attachment_textures[AttachmentCount];
		ImageView_t l_attachment_image_views[AttachmentCount];
		Token(TextureGPU) l_tokenized_textures[AttachmentCount];

		for (loop(i, 0, AttachmentCount))
		{
			l_attachment_textures[i] = TextureGPU::allocate(p_buffer_allocator, p_attachment_formats[i]);
			l_attachment_image_views[i] = l_attachment_textures[i].ImageView;
			l_tokenized_textures[i] = this->textures_gpu.alloc_element(l_attachment_textures[i]);
		};

		VkFramebufferCreateInfo l_framebuffer_create{};
		l_framebuffer_create.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		l_framebuffer_create.attachmentCount = AttachmentCount;
		l_framebuffer_create.pAttachments = l_attachment_image_views;
		l_framebuffer_create.layers = 1;
		l_framebuffer_create.height = p_attachment_formats[0].extent.x;
		l_framebuffer_create.width = p_attachment_formats[0].extent.y;
		l_framebuffer_create.renderPass = l_render_pass.render_pass;

		GraphicsPass l_graphics_pass;
		vk_handle_result(vkCreateFramebuffer(p_buffer_allocator.device.device, &l_framebuffer_create, NULL, &l_graphics_pass.frame_buffer));

		l_graphics_pass.attachment_textures = this->renderpass_attachment_textures.alloc_vector_with_values(
				Slice<Token(TextureGPU) >::build_memory_elementnb(l_tokenized_textures, AttachmentCount));
		l_graphics_pass.render_pass = l_render_pass;

		return l_graphics_pass;
	};

	inline void GraphicsAllocator::_free_graphics_pass(BufferAllocator& p_buffer_allocator, GraphicsPass& p_graphics_pass)
	{
		p_graphics_pass.render_pass.free(this->graphicsDevice);
		Slice<Token(TextureGPU) > l_attached_textures = this->renderpass_attachment_textures.get_vector(p_graphics_pass.attachment_textures);
		for (loop(i, 0, l_attached_textures.Size))
		{
			this->textures_gpu.get(l_attached_textures.get(i)).free(p_buffer_allocator);
			this->textures_gpu.release_element(l_attached_textures.get(i));
		}

		this->renderpass_attachment_textures.release_vector(p_graphics_pass.attachment_textures);

		vkDestroyFramebuffer(this->graphicsDevice.device, p_graphics_pass.frame_buffer, NULL);
	};


};