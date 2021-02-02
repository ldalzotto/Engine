#pragma once

namespace v2
{

	struct GraphicsDevice
	{
		GraphicsCard graphics_card;
		gc_t device;
		gcqueue_t graphics_queue;

		CommandPool command_pool;
		CommandBuffer command_buffer;

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
	};

	typedef VkRenderPass RenderPass_t;

	enum class AttachmentType
	{
		COLOR, DEPTH
	};

	struct RenderPass
	{
		RenderPass_t render_pass;
	};

	typedef VkFramebuffer FrameBuffer_t;

	struct GraphicsPass
	{
		RenderPass render_pass;
		Token(Slice<Token(TextureGPU)>) attachment_textures;
		FrameBuffer_t frame_buffer;
	};

	struct GraphicsAllocator
	{
		GraphicsDevice graphicsDevice;

		Pool<TextureGPU> textures_gpu;
		PoolOfVector<Token(TextureGPU)> renderpass_attachment_textures;
		Pool<GraphicsPass> graphics_pass;

		static GraphicsAllocator GraphicsAllocator::allocate_default(GPUInstance& p_instance);
		void free();

		Token(TextureGPU) allocate_texturegpu(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format);
		void free_texturegpu(BufferAllocator& p_buffer_allocator, const Token(TextureGPU) p_texture_gpu);

		template<uint8 AttachmentCount>
		Token(GraphicsPass) allocate_graphicspass(BufferAllocator& p_buffer_allocator, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount]);
		void free_graphicspass(BufferAllocator& p_buffer_allocator, const Token(GraphicsPass) p_graphics_pass);

		void cmd_beginRenderPass2(BufferAllocator& p_buffer_allocator, const GraphicsPass& p_graphics_pass, const Slice<v4f>& p_clear_values);
		void cmd_endRenderPass(const GraphicsPass& p_graphics_pass);

	private:

		TextureGPU _allocate_texture_gpu(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format);
		void _free_texture_gpu(BufferAllocator& p_buffer_allocator, TextureGPU& p_texture_gpu);

		template<uint8 AttachmentCount>
		RenderPass _allocate_renderpass(const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount]);
		void _free_renderpass(RenderPass p_render_pass);

		template<uint8 AttachmentCount>
		GraphicsPass _allocate_graphics_pass(BufferAllocator& p_buffer_allocator, const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount]);
		void _free_graphics_pass(BufferAllocator& p_buffer_allocator, GraphicsPass& p_graphics_pass);
	};


	inline GraphicsDevice GraphicsDevice::allocate(GPUInstance& p_instance)
	{
		GraphicsDevice l_graphics_device;
		l_graphics_device.graphics_card = p_instance.graphics_card;
		l_graphics_device.device = p_instance.logical_device;

		vkGetDeviceQueue(l_graphics_device.device, p_instance.graphics_card.graphics_queue_family, 0, &l_graphics_device.graphics_queue);

		l_graphics_device.command_pool = CommandPool::allocate(l_graphics_device.device, p_instance.graphics_card.graphics_queue_family);
		l_graphics_device.command_buffer = l_graphics_device.command_pool.allocate_command_buffer(l_graphics_device.device, l_graphics_device.graphics_queue);

		return l_graphics_device;
	};

	inline void GraphicsDevice::free()
	{
		// this->command_buffer.flush();
		this->command_pool.free_command_buffer(this->device, this->command_buffer);
		this->command_pool.free(this->device);
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

	inline GraphicsAllocator GraphicsAllocator::allocate_default(GPUInstance& p_instance)
	{
		return GraphicsAllocator{
			GraphicsDevice::allocate(p_instance),
			Pool<TextureGPU>::allocate(0),
			PoolOfVector<Token(TextureGPU)>::allocate_default()
		};
	};

	inline void GraphicsAllocator::free()
	{
#if GPU_DEBUG
		assert_true(!this->textures_gpu.has_allocated_elements());
		assert_true(!this->renderpass_attachment_textures.has_allocated_elements());
#endif

		this->textures_gpu.free();
		this->renderpass_attachment_textures.free();
		this->graphicsDevice.free();
	};

	inline Token(TextureGPU) GraphicsAllocator::allocate_texturegpu(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format)
	{
		return this->textures_gpu.alloc_element(_allocate_texture_gpu(p_buffer_allocator, p_image_format));
	};

	inline void GraphicsAllocator::free_texturegpu(BufferAllocator& p_buffer_allocator, const Token(TextureGPU) p_texture_gpu)
	{
		TextureGPU& l_texture_gpu = this->textures_gpu.get(p_texture_gpu);
		_free_texture_gpu(p_buffer_allocator, l_texture_gpu);
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

		Slice<Token(TextureGPU)> l_attachments = this->renderpass_attachment_textures.get_vector(p_graphics_pass.attachment_textures);
		ImageFormat& l_target_format = p_buffer_allocator.gpu_images.get(this->textures_gpu.get(l_attachments.get(0)).Image).format;

		l_renderpass_begin.renderArea = VkRect2D{ VkOffset2D{0,0}, VkExtent2D{l_target_format.extent.x, l_target_format.extent.y} };
		l_renderpass_begin.clearValueCount = (uint32)p_clear_values.Size;
		l_renderpass_begin.pClearValues = (VkClearValue*)p_clear_values.Begin;

		l_renderpass_begin.framebuffer = p_graphics_pass.frame_buffer;

		VkViewport l_viewport{};
		l_viewport.width = (float)l_target_format.extent.x;
		l_viewport.height = (float)l_target_format.extent.y;
		l_viewport.minDepth = 0.0f;
		l_viewport.maxDepth = 1.0f;

		VkRect2D l_windowarea = VkRect2D{ VkOffset2D{0,0}, VkExtent2D{l_target_format.extent.x, l_target_format.extent.y} };
		vkCmdSetViewport(this->graphicsDevice.command_buffer.command_buffer, 0, 1, &l_viewport);
		vkCmdSetScissor(this->graphicsDevice.command_buffer.command_buffer, 0, 1, &l_windowarea);

		vkCmdBeginRenderPass(this->graphicsDevice.command_buffer.command_buffer, &l_renderpass_begin, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
	};

	inline void GraphicsAllocator::cmd_endRenderPass(const GraphicsPass& p_graphics_pass)
	{
		vkCmdEndRenderPass(this->graphicsDevice.command_buffer.command_buffer);
	};

	inline TextureGPU GraphicsAllocator::_allocate_texture_gpu(BufferAllocator& p_buffer_allocator, const ImageFormat& p_image_format)
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
		}
		l_imageview_create_info.format = p_image_format.format;
		l_imageview_create_info.subresourceRange = VkImageSubresourceRange{
			p_image_format.imageAspect,
			0,
			p_image_format.mipLevels,
			0,
			p_image_format.arrayLayers
		};

		vk_handle_result(vkCreateImageView(p_buffer_allocator.device.device, &l_imageview_create_info, NULL, &l_texture_gpu.ImageView));

		return l_texture_gpu;
	};

	inline void GraphicsAllocator::_free_texture_gpu(BufferAllocator& p_buffer_allocator, TextureGPU& p_texture_gpu)
	{
		vkDestroyImageView(p_buffer_allocator.device.device, p_texture_gpu.ImageView, NULL);
		p_buffer_allocator.free_imagegpu(p_texture_gpu.Image);
	};


	template<uint8 AttachmentCount>
	inline RenderPass GraphicsAllocator::_allocate_renderpass(const AttachmentType p_attachment_types[AttachmentCount], const ImageFormat p_attachment_formats[AttachmentCount])
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

		vk_handle_result(vkCreateRenderPass(this->graphicsDevice.device, &l_renderpass_create_info, NULL, &l_render_pass_object.render_pass));
		return l_render_pass_object;
	};

	inline void GraphicsAllocator::_free_renderpass(RenderPass p_render_pass)
	{
		vkDestroyRenderPass(this->graphicsDevice.device, p_render_pass.render_pass, NULL);
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

		RenderPass l_render_pass = _allocate_renderpass<2>(p_attachment_types, p_attachment_formats);
		TextureGPU l_attachment_textures[AttachmentCount];
		ImageView_t l_attachment_image_views[AttachmentCount];
		Token(TextureGPU) l_tokenized_textures[AttachmentCount];

		for (loop(i, 0, AttachmentCount))
		{
			l_attachment_textures[i] = _allocate_texture_gpu(p_buffer_allocator, p_attachment_formats[i]);
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
					Slice<Token(TextureGPU)>::build_memory_elementnb(l_tokenized_textures, AttachmentCount));
		l_graphics_pass.render_pass = l_render_pass;

		return l_graphics_pass;
	};

	inline void GraphicsAllocator::_free_graphics_pass(BufferAllocator& p_buffer_allocator, GraphicsPass& p_graphics_pass)
	{
		_free_renderpass(p_graphics_pass.render_pass);
		Slice<Token(TextureGPU)> l_attached_textures = this->renderpass_attachment_textures.get_vector(p_graphics_pass.attachment_textures);
		for (loop(i, 0, l_attached_textures.Size))
		{
			_free_texture_gpu(p_buffer_allocator, this->textures_gpu.get(l_attached_textures.get(i)));
			this->textures_gpu.release_element(l_attached_textures.get(i));
		}

		this->renderpass_attachment_textures.release_vector(p_graphics_pass.attachment_textures);

		vkDestroyFramebuffer(this->graphicsDevice.device, p_graphics_pass.frame_buffer, NULL);
	};


};