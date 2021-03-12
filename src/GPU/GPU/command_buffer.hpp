#pragma once

struct Semafore
{
    VkSemaphore semaphore;

    static Semafore allocate(const gc_t p_device);
    void free(const gc_t p_device);
};

struct CommandBuffer
{
    VkCommandBuffer command_buffer;
    // VkSemaphore semaphore;
    gcqueue_t queue;
    int8 has_begun;

    static CommandBuffer build_default();

    void begin();
    void end();
    void submit();
    void submit_and_notity(const Semafore p_notify);
    void submit_after(const Semafore p_wait_for, const VkPipelineStageFlags p_wait_stage);
    void submit_after_and_notify(const Semafore p_wait_for, const VkPipelineStageFlags p_wait_stage, const Semafore p_notify);
    void wait_for_completion();
    void flush();

    void force_sync_execution();
};

struct CommandPool
{
    VkCommandPool pool;

    static CommandPool allocate(const gc_t p_device, const uint32 p_queue_family);
    void free(const gc_t p_device);

    CommandBuffer allocate_command_buffer(const gc_t p_device, const gcqueue_t p_queue);
    void free_command_buffer(const gc_t p_device, const CommandBuffer& p_command_buffer);
};


inline Semafore Semafore::allocate(const gc_t p_device)
{
    Semafore l_semaphore;
    VkSemaphoreCreateInfo l_semaphore_create_info{};
    l_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk_handle_result(vkCreateSemaphore(p_device, &l_semaphore_create_info, NULL, &l_semaphore.semaphore));
    return l_semaphore;
};

inline void Semafore::free(const gc_t p_device)
{
    vkDestroySemaphore(p_device, this->semaphore, NULL);
};

inline CommandBuffer CommandBuffer::build_default()
{
    return CommandBuffer{NULL, NULL, 0};
};

inline void CommandBuffer::begin()
{
    if (!this->has_begun)
    {
        VkCommandBufferBeginInfo l_command_buffer_begin_info{};
        l_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk_handle_result(vkBeginCommandBuffer(this->command_buffer, &l_command_buffer_begin_info));
        this->has_begun = 1;
    }
};

inline void CommandBuffer::end()
{
    if (this->has_begun)
    {
        vk_handle_result(vkEndCommandBuffer(this->command_buffer));
        this->has_begun = 0;
    }
};

inline void CommandBuffer::submit()
{
    this->end();
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = &this->command_buffer;
    /*
    l_wait_for_end_submit.signalSemaphoreCount = 1;
    l_wait_for_end_submit.pSignalSemaphores = &this->semaphore;
    */
    vk_handle_result(vkQueueSubmit(this->queue, 1, &l_wait_for_end_submit, NULL));
};

inline void CommandBuffer::submit_and_notity(const Semafore p_notify)
{
    this->end();
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = &this->command_buffer;
    l_wait_for_end_submit.signalSemaphoreCount = 1;
    l_wait_for_end_submit.pSignalSemaphores = &p_notify.semaphore;
    vk_handle_result(vkQueueSubmit(this->queue, 1, &l_wait_for_end_submit, NULL));
};

// VK_PIPELINE_STAGE_HOST_BIT

inline void CommandBuffer::submit_after(const Semafore p_wait_for, const VkPipelineStageFlags p_wait_stage)
{
    this->end();
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = &this->command_buffer;
    l_wait_for_end_submit.waitSemaphoreCount = 1;
    l_wait_for_end_submit.pWaitSemaphores = &p_wait_for.semaphore;
    l_wait_for_end_submit.pWaitDstStageMask = &p_wait_stage;
    vk_handle_result(vkQueueSubmit(this->queue, 1, &l_wait_for_end_submit, NULL));
};

inline void CommandBuffer::submit_after_and_notify(const Semafore p_wait_for, const VkPipelineStageFlags p_wait_stage, const Semafore p_notify){
    this->end();
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = &this->command_buffer;
    l_wait_for_end_submit.waitSemaphoreCount = 1;
    l_wait_for_end_submit.pWaitSemaphores = &p_wait_for.semaphore;
    l_wait_for_end_submit.pWaitDstStageMask = &p_wait_stage;
    l_wait_for_end_submit.signalSemaphoreCount = 1;
    l_wait_for_end_submit.pSignalSemaphores = &p_notify.semaphore;
    vk_handle_result(vkQueueSubmit(this->queue, 1, &l_wait_for_end_submit, NULL));
};

inline void CommandBuffer::wait_for_completion()
{
    vk_handle_result(vkQueueWaitIdle(this->queue));
};

inline void CommandBuffer::flush()
{
    this->submit();
    this->wait_for_completion();
};

inline void CommandBuffer::force_sync_execution()
{
    this->submit();
    this->wait_for_completion();
};

inline CommandPool CommandPool::allocate(const gc_t p_device, const uint32 p_queue_family)
{
    CommandPool l_pool;

    VkCommandPoolCreateInfo l_command_pool_create_info{};
    l_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    l_command_pool_create_info.queueFamilyIndex = p_queue_family;
    l_command_pool_create_info.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_handle_result(vkCreateCommandPool(p_device, &l_command_pool_create_info, NULL, &l_pool.pool));
    return l_pool;
};

inline void CommandPool::free(const gc_t p_device)
{
    vkDestroyCommandPool(p_device, this->pool, NULL);
};

inline CommandBuffer CommandPool::allocate_command_buffer(const gc_t p_device, const gcqueue_t p_queue)
{
    CommandBuffer l_command_buffer = CommandBuffer::build_default();

    VkCommandBufferAllocateInfo l_command_buffer_allocate_info{};
    l_command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    l_command_buffer_allocate_info.commandPool = this->pool;
    l_command_buffer_allocate_info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    l_command_buffer_allocate_info.commandBufferCount = 1;
    vk_handle_result(vkAllocateCommandBuffers(p_device, &l_command_buffer_allocate_info, &l_command_buffer.command_buffer));

    l_command_buffer.queue = p_queue;

    /*
    VkSemaphoreCreateInfo l_semaphore_create_info{};
    l_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk_handle_result(vkCreateSemaphore(p_device, &l_semaphore_create_info, NULL, &l_command_buffer.semaphore));
    */
    // vkDestroySemaphore()
    // This is to avoid errors if we try to submit the command buffer if there is no previous recording.
    l_command_buffer.begin();
    l_command_buffer.end();

    return l_command_buffer;
};

inline void CommandPool::free_command_buffer(const gc_t p_device, const CommandBuffer& p_command_buffer){
    //  vkDestroySemaphore(p_device, p_command_buffer.semaphore, NULL);
};