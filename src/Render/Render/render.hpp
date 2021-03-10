#pragma once

#include "GPU/gpu.hpp"

namespace v2
{
struct Camera
{
    m44f view;
    m44f projection;
};

struct Vertex
{
    v3f position;
    v2f uv;
};

struct ShaderIndex
{
    uimax execution_order;
    TokenT(Shader) shader_index;
    TokenT(ShaderLayout) shader_layout;
};

struct Mesh
{
    TokenT(BufferGPU) vertices_buffer;
    TokenT(BufferGPU) indices_buffer;
    uimax indices_count;
};

struct RenderableObject
{
    TokenT(ShaderUniformBufferHostParameter) model;
    TokenT(Mesh) mesh;
};

struct D3RendererHeap
{
    Pool<ShaderIndex> shaders;
    Pool<Material> materials;
    Pool<Mesh> meshes;
    Pool<RenderableObject> renderable_objects;

    Vector<TokenT(ShaderIndex)> shaders_indexed;
    PoolOfVector<TokenT(Material)> shaders_to_materials;
    PoolOfVector<TokenT(RenderableObject)> material_to_renderable_objects;

    struct RenderableObject_ModelUpdateEvent
    {
        TokenT(RenderableObject) renderable_object;
        m44f model_matrix;
    };

    Vector<RenderableObject_ModelUpdateEvent> model_update_events;

    static D3RendererHeap allocate();

    void free();

    void link_shader_with_material(const TokenT(ShaderIndex) p_shader, const TokenT(Material) p_material);

    void unlink_shader_with_material(const TokenT(ShaderIndex) p_shader, const TokenT(Material) p_material);

    void get_materials_and_renderableobject_linked_to_shader(const TokenT(ShaderIndex) p_shader, Vector<TokenT(Material)>* in_out_materials, Vector<TokenT(RenderableObject)>* in_out_renderableobject);

    PoolOfVector<TokenT(Material)>::Element_ShadowVector get_materials_from_shader(const TokenT(ShaderIndex) p_shader);

    void link_material_with_renderable_object(const TokenT(Material) p_material, const TokenT(RenderableObject) p_renderable_object);

    void unlink_material_with_renderable_object(const TokenT(Material) p_material, const TokenT(RenderableObject) p_renderable_object);

    PoolOfVector<TokenT(RenderableObject)>::Element_ShadowVector get_renderableobjects_from_material(const TokenT(Material) p_material);

    void push_modelupdateevent(const RenderableObject_ModelUpdateEvent& p_modelupdateevent);
};

struct D3RendererAllocator
{
    D3RendererHeap heap;

    static D3RendererAllocator allocate();

    void free();

    TokenT(ShaderIndex) allocate_shader(const ShaderIndex& p_shader);

    void free_shader(const TokenT(ShaderIndex) p_shader);

    TokenT(Material) allocate_material(const Material& p_material);

    void free_material(const TokenT(Material) p_material);

    TokenT(Mesh) allocate_mesh(const Mesh& p_mesh);

    void free_mesh(const TokenT(Mesh) p_mesh);

    TokenT(RenderableObject) allocate_renderable_object(const RenderableObject& p_renderable_object);

    void free_renderable_object(const TokenT(RenderableObject) p_rendereable_object);
};

namespace ColorStep_const
{
SliceN<ShaderLayoutParameterType, 1> shaderlayout_before = SliceN<ShaderLayoutParameterType, 1>{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX};
SliceN<ShaderLayoutParameterType, 1> shaderlayout_after = SliceN<ShaderLayoutParameterType, 1>{ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX};
SliceN<ShaderLayout::VertexInputParameter, 2> shaderlayout_vertex_input = SliceN<ShaderLayout::VertexInputParameter, 2>{
    ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, 0}, ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_2, offsetof(Vertex, uv)}};
}; // namespace ColorStep_const

struct ColorStep
{
    v3ui render_target_dimensions;

    TokenT(GraphicsPass) pass;
    Span<v4f> clear_values;

    TokenT(ShaderLayout) global_buffer_layout;
    Material global_material;

    struct AllocateInfo
    {
        v3ui render_target_dimensions;
        int8 attachment_host_read;
        int8 color_attachment_sample;
    };

    static ColorStep allocate(GPUContext& p_gpu_context, const AllocateInfo& p_allocate_info);

    void free(GPUContext& p_gpu_context);

    void set_camera(GPUContext& p_gpu_context, const Camera& p_camera);

    void set_camera_projection(GPUContext& p_gpu_context, const float32 p_near, const float32 p_far, const float32 p_fov);

    void set_camera_view(GPUContext& p_gpu_context, const v3f& p_world_position, const v3f& p_forward, const v3f& p_up);

    Slice<Camera> get_camera(GPUContext& p_gpu_context);
};

/*
    The D3Renderer is a structure that organize GPU graphics allocated data in a hierarchical way (Shader -> Material -> RenderableObject).
*/
struct D3Renderer
{
    D3RendererAllocator allocator;
    ColorStep color_step;

    static D3Renderer allocate(GPUContext& p_gpu_context, const ColorStep::AllocateInfo& p_allocation_info);

    void free(GPUContext& p_gpu_context);

    D3RendererHeap& heap();

    void buffer_step(GPUContext& p_gpu_context);

    void graphics_step(GraphicsBinder& p_graphics_binder);
};

inline D3RendererHeap D3RendererHeap::allocate()
{
    D3RendererHeap l_heap;

    l_heap.shaders = Pool<ShaderIndex>::allocate(0);
    l_heap.materials = Pool<Material>::allocate(0);
    l_heap.meshes = Pool<Mesh>::allocate(0);
    l_heap.renderable_objects = Pool<RenderableObject>::allocate(0);
    l_heap.shaders_indexed = Vector<TokenT(ShaderIndex)>::allocate(0);
    l_heap.shaders_to_materials = PoolOfVector<TokenT(Material)>::allocate_default();
    l_heap.material_to_renderable_objects = PoolOfVector<TokenT(RenderableObject)>::allocate_default();
    l_heap.model_update_events = Vector<RenderableObject_ModelUpdateEvent>::allocate(0);
    return l_heap;
};

inline void D3RendererHeap::free()
{
#if RENDER_BOUND_TEST
    assert_true(!this->shaders.has_allocated_elements());
    assert_true(!this->materials.has_allocated_elements());
    assert_true(!this->meshes.has_allocated_elements());
    assert_true(!this->renderable_objects.has_allocated_elements());
    assert_true(this->shaders_indexed.empty());
    assert_true(!this->shaders_to_materials.has_allocated_elements());
    assert_true(!this->material_to_renderable_objects.has_allocated_elements());
    assert_true(this->model_update_events.empty());
#endif

    this->shaders.free();
    this->materials.free();
    this->meshes.free();
    this->renderable_objects.free();
    this->shaders_indexed.free();
    this->shaders_to_materials.free();
    this->material_to_renderable_objects.free();
    this->model_update_events.free();
};

inline void D3RendererHeap::link_shader_with_material(const TokenT(ShaderIndex) p_shader, const TokenT(Material) p_material)
{
    this->shaders_to_materials.element_push_back_element(tk_bfT(Slice<TokenT(Material)>, p_shader), p_material);
};

inline void D3RendererHeap::unlink_shader_with_material(const TokenT(ShaderIndex) p_shader, const TokenT(Material) p_material)
{
    auto l_materials = this->get_materials_from_shader(p_shader);
    for (loop(i, 0, l_materials.get_size()))
    {
        if (tk_eq(l_materials.get(i), p_material))
        {
            l_materials.erase_element_at_always(i);
            return;
        }
    }
};

inline void D3RendererHeap::get_materials_and_renderableobject_linked_to_shader(const TokenT(ShaderIndex) p_shader, Vector<TokenT(Material)>* in_out_materials,
                                                                                Vector<TokenT(RenderableObject)>* in_out_renderableobject)
{
    auto l_materials = this->get_materials_from_shader(p_shader);
    for (loop(i, 0, l_materials.get_size()))
    {
        in_out_renderableobject->push_back_array(this->get_renderableobjects_from_material(l_materials.get(i)).to_slice());
    }
    in_out_materials->push_back_array(l_materials.to_slice());
};

inline PoolOfVector<TokenT(Material)>::Element_ShadowVector D3RendererHeap::get_materials_from_shader(const TokenT(ShaderIndex) p_shader)
{
    return this->shaders_to_materials.get_element_as_shadow_vector(tk_bfT(Slice<TokenT(Material)>, p_shader));
};

inline void D3RendererHeap::link_material_with_renderable_object(const TokenT(Material) p_material, const TokenT(RenderableObject) p_renderable_object)
{
    this->material_to_renderable_objects.element_push_back_element(tk_bfT(Slice<TokenT(RenderableObject)>, p_material), p_renderable_object);
};

inline void D3RendererHeap::unlink_material_with_renderable_object(const TokenT(Material) p_material, const TokenT(RenderableObject) p_renderable_object)
{
    auto l_linked_renderable_objects = this->get_renderableobjects_from_material(p_material);
    for (loop(i, 0, l_linked_renderable_objects.get_size()))
    {
        if (tk_eq(l_linked_renderable_objects.get(i), p_renderable_object))
        {
            l_linked_renderable_objects.erase_element_at_always(i);
            return;
        }
    }
};

inline PoolOfVector<TokenT(RenderableObject)>::Element_ShadowVector D3RendererHeap::get_renderableobjects_from_material(const TokenT(Material) p_material)
{
    return this->material_to_renderable_objects.get_element_as_shadow_vector(tk_bfT(Slice<TokenT(RenderableObject)>, p_material));
};

inline void D3RendererHeap::push_modelupdateevent(const RenderableObject_ModelUpdateEvent& p_modelupdateevent)
{
    this->model_update_events.push_back_element(p_modelupdateevent);
};

inline D3RendererAllocator D3RendererAllocator::allocate()
{
    return D3RendererAllocator{D3RendererHeap::allocate()};
};

inline void D3RendererAllocator::free()
{
    this->heap.free();
};

inline TokenT(ShaderIndex) D3RendererAllocator::allocate_shader(const ShaderIndex& p_shader)
{
    uimax l_insertion_index = 0;
    for (loop(i, 0, this->heap.shaders_indexed.Size))
    {
        if (this->heap.shaders.get(this->heap.shaders_indexed.get(i)).execution_order >= l_insertion_index)
        {
            break;
        };
    };

    TokenT(ShaderIndex) l_shader = this->heap.shaders.alloc_element(p_shader);
    this->heap.shaders_to_materials.alloc_vector();

    this->heap.shaders_indexed.insert_element_at_always(l_shader, l_insertion_index);
    return l_shader;
};

inline void D3RendererAllocator::free_shader(const TokenT(ShaderIndex) p_shader)
{
    ShaderIndex& l_shader = this->heap.shaders.get(p_shader);

    for (loop_reverse(j, 0, this->heap.shaders_indexed.Size))
    {
        if (tk_eq(this->heap.shaders_indexed.get(j), p_shader))
        {
            this->heap.shaders_indexed.erase_element_at_always(j);
            break;
        }
    }
    
    this->heap.shaders_to_materials.release_vector(tk_bfT(Slice<TokenT(Material)>, p_shader));
    this->heap.shaders.release_element(p_shader);
};

inline TokenT(Material) D3RendererAllocator::allocate_material(const Material& p_material)
{
    this->heap.material_to_renderable_objects.alloc_vector();
    return this->heap.materials.alloc_element(p_material);
};

inline void D3RendererAllocator::free_material(const TokenT(Material) p_material)
{
    this->heap.material_to_renderable_objects.release_vector(tk_bfT(Slice<TokenT(RenderableObject)>, p_material));
    this->heap.materials.release_element(p_material);
};

inline TokenT(Mesh) D3RendererAllocator::allocate_mesh(const Mesh& p_mesh)
{
    return this->heap.meshes.alloc_element(p_mesh);
};

inline void D3RendererAllocator::free_mesh(const TokenT(Mesh) p_mesh)
{
    this->heap.meshes.release_element(p_mesh);
};

inline TokenT(RenderableObject) D3RendererAllocator::allocate_renderable_object(const RenderableObject& p_renderable_object)
{
    return this->heap.renderable_objects.alloc_element(p_renderable_object);
};

inline void D3RendererAllocator::free_renderable_object(const TokenT(RenderableObject) p_rendereable_object)
{
    for (loop_reverse(i, 0, this->heap.model_update_events.Size))
    {
        if (tk_eq(this->heap.model_update_events.get(i).renderable_object, p_rendereable_object))
        {
            this->heap.model_update_events.erase_element_at_always(i);
        }
    }

    this->heap.renderable_objects.release_element(p_rendereable_object);
};

struct D3RendererAllocatorComposition
{

    inline static TokenT(Mesh)
        allocate_mesh_with_buffers(BufferMemory& p_buffer_memory, D3RendererAllocator& p_render_allocator, const Slice<Vertex>& p_initial_vertices, const Slice<uint32>& p_initial_indices)
    {
        Mesh l_mesh;
        Slice<int8> l_initial_vertices_binary = p_initial_vertices.build_asint8();
        l_mesh.vertices_buffer = p_buffer_memory.allocator.allocate_buffergpu(l_initial_vertices_binary.Size,
                                                                              (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::VERTEX | (BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_mesh.vertices_buffer, l_initial_vertices_binary);

        Slice<int8> l_initial_indices_binary = p_initial_indices.build_asint8();
        l_mesh.indices_buffer = p_buffer_memory.allocator.allocate_buffergpu(l_initial_indices_binary.Size,
                                                                             (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::INDEX | (BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_mesh.indices_buffer, l_initial_indices_binary);

        l_mesh.indices_count = p_initial_indices.Size;

        return p_render_allocator.allocate_mesh(l_mesh);
    };

    inline static void free_mesh_with_buffers(BufferMemory& p_buffer_memory, D3RendererAllocator& p_render_allocator, const TokenT(Mesh) p_mesh)
    {
        Mesh& l_mesh = p_render_allocator.heap.meshes.get(p_mesh);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, l_mesh.vertices_buffer);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, l_mesh.indices_buffer);
        p_render_allocator.free_mesh(p_mesh);
    };

    inline static TokenT(Material) allocate_emptymaterial_with_parameters(GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator)
    {
        Material l_material = Material::allocate_empty(p_graphics_allocator, 1);
        return p_render_allocator.allocate_material(l_material);
    };

    inline static void free_material_with_parameters_and_textures(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator,
                                                                  const TokenT(Material) p_material)
    {
        p_render_allocator.heap.materials.get(p_material).free_with_textures(p_graphics_allocator, p_buffer_memory);
        p_render_allocator.free_material(p_material);
    };

    inline static void free_material_with_parameters(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator, const TokenT(Material) p_material)
    {
        p_render_allocator.heap.materials.get(p_material).free(p_graphics_allocator, p_buffer_memory);
        p_render_allocator.free_material(p_material);
    };

    inline static TokenT(RenderableObject)
        allocate_renderable_object_with_buffers(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator, const TokenT(Mesh) p_mesh)
    {
        RenderableObject l_renderable_object;
        l_renderable_object.mesh = p_mesh;

        TokenT(BufferHost) l_buffer = p_buffer_memory.allocator.allocate_bufferhost_empty(sizeof(m44f), BufferUsageFlag::UNIFORM);
        l_renderable_object.model =
            p_graphics_allocator.allocate_shaderuniformbufferhost_parameter(ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, l_buffer, p_buffer_memory.allocator.host_buffers.get(l_buffer));

        return p_render_allocator.allocate_renderable_object(l_renderable_object);
    };

    inline static void free_renderable_object_with_buffers(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator,
                                                           const TokenT(RenderableObject) p_renderable_object)
    {
        RenderableObject& l_renderable_object = p_render_allocator.heap.renderable_objects.get(p_renderable_object);
        GraphicsAllocatorComposition::free_shaderparameter_uniformbufferhost_with_buffer(p_buffer_memory, p_graphics_allocator, l_renderable_object.model);
        p_render_allocator.free_renderable_object(p_renderable_object);
    };

    inline static TokenT(RenderableObject)
        allocate_renderable_object_with_mesh_and_buffers(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator,
                                                         const Slice<Vertex>& p_initial_vertices, const Slice<uint32>& p_initial_indices)
    {
        TokenT(Mesh) l_mesh = allocate_mesh_with_buffers(p_buffer_memory, p_render_allocator, p_initial_vertices, p_initial_indices);
        return allocate_renderable_object_with_buffers(p_buffer_memory, p_graphics_allocator, p_render_allocator, l_mesh);
    };

    inline static void free_renderable_object_with_mesh_and_buffers(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator,
                                                                    const TokenT(RenderableObject) p_renderable_object)
    {
        RenderableObject& l_renderable_object = p_render_allocator.heap.renderable_objects.get(p_renderable_object);
        free_mesh_with_buffers(p_buffer_memory, p_render_allocator, l_renderable_object.mesh);
        free_renderable_object_with_buffers(p_buffer_memory, p_graphics_allocator, p_render_allocator, p_renderable_object);
    };

    inline static void free_renderable_object_external_ressources(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator,
                                                                  RenderableObject& p_renderable_object)
    {
        GraphicsAllocatorComposition::free_shaderparameter_uniformbufferhost_with_buffer(p_buffer_memory, p_graphics_allocator, p_renderable_object.model);
        free_mesh_with_buffers(p_buffer_memory, p_render_allocator, p_renderable_object.mesh);
    };

    inline static TokenT(ShaderIndex)
        allocate_colorstep_shader_with_shaderlayout(GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator, const Slice<ShaderLayoutParameterType>& p_specific_parameters,
                                                    const uimax p_execution_order, const GraphicsPass& p_graphics_pass, const ShaderConfiguration& p_shader_configuration,
                                                    const ShaderModule& p_vertex_shader, const ShaderModule& p_fragment_shader)
    {
        Span<ShaderLayoutParameterType> l_span =
            Span<ShaderLayoutParameterType>::allocate_slice_3(ColorStep_const::shaderlayout_before.to_slice(), p_specific_parameters, ColorStep_const::shaderlayout_after.to_slice());
        Span<ShaderLayout::VertexInputParameter> l_vertex_input = Span<ShaderLayout::VertexInputParameter>::allocate_slicen(ColorStep_const::shaderlayout_vertex_input);

        ShaderIndex l_shader_index;
        l_shader_index.execution_order = p_execution_order;
        l_shader_index.shader_layout = p_graphics_allocator.allocate_shader_layout(l_span, l_vertex_input, sizeof(Vertex));

        ShaderAllocateInfo l_shader_allocate_info{p_graphics_pass, p_shader_configuration, p_graphics_allocator.heap.shader_layouts.get(l_shader_index.shader_layout), p_vertex_shader,
                                                  p_fragment_shader};
        l_shader_index.shader_index = p_graphics_allocator.allocate_shader(l_shader_allocate_info);
        return p_render_allocator.allocate_shader(l_shader_index);
    };

    inline static void free_shader_with_shaderlayout(GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator, const TokenT(ShaderIndex) p_shader)
    {
        p_graphics_allocator.free_shader_layout(p_render_allocator.heap.shaders.get(p_shader).shader_layout);
        p_graphics_allocator.free_shader(p_render_allocator.heap.shaders.get(p_shader).shader_index);
        p_render_allocator.free_shader(p_shader);
    };

    inline static void free_shader_recursively_with_gpu_ressources(BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, D3RendererAllocator& p_render_allocator,
                                                                   const TokenT(ShaderIndex) p_shader)
    {
        auto l_materials_to_remove = p_render_allocator.heap.get_materials_from_shader(p_shader);

        for (loop_reverse(i, 0, l_materials_to_remove.get_size()))
        {
            TokenT(Material) l_material_to_remove = l_materials_to_remove.get(i);

            auto l_renderable_objects_to_remove = p_render_allocator.heap.get_renderableobjects_from_material(l_material_to_remove);

            for (loop_reverse(j, 0, l_renderable_objects_to_remove.get_size()))
            {
                TokenT(RenderableObject) l_renderable_object_to_remove = l_renderable_objects_to_remove.get(j);
                p_render_allocator.heap.unlink_material_with_renderable_object(l_material_to_remove, l_renderable_object_to_remove);
                free_renderable_object_with_mesh_and_buffers(p_buffer_memory, p_graphics_allocator, p_render_allocator, l_renderable_object_to_remove);
            }

            p_render_allocator.heap.unlink_shader_with_material(p_shader, l_material_to_remove);
            free_material_with_parameters_and_textures(p_buffer_memory, p_graphics_allocator, p_render_allocator, l_material_to_remove);
        }

        free_shader_with_shaderlayout(p_graphics_allocator, p_render_allocator, p_shader);
    };
};

inline ColorStep ColorStep::allocate(GPUContext& p_gpu_context, const AllocateInfo& p_allocate_info)
{
    ColorStep l_step;

    Span<ShaderLayoutParameterType> l_global_buffer_parameters = Span<ShaderLayoutParameterType>::allocate(1);
    l_global_buffer_parameters.get(0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX;
    Span<ShaderLayout::VertexInputParameter> l_global_buffer_vertices_parameters = Span<ShaderLayout::VertexInputParameter>::build(NULL, 0);

    ImageUsageFlag l_additional_attachment_usage_flags = ImageUsageFlag::UNDEFINED;
    if (p_allocate_info.attachment_host_read)
    {
        l_additional_attachment_usage_flags = (ImageUsageFlag)((ImageUsageFlags)l_additional_attachment_usage_flags | (ImageUsageFlags)ImageUsageFlag::TRANSFER_READ);
    }
    if(p_allocate_info.color_attachment_sample)
    {
        l_additional_attachment_usage_flags = (ImageUsageFlag)((ImageUsageFlags)l_additional_attachment_usage_flags | (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER);
    }



    SliceN<RenderPassAttachment, 2> l_attachments = {
        RenderPassAttachment{AttachmentType::COLOR, ImageFormat::build_color_2d(p_allocate_info.render_target_dimensions, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT |
                                                                                                                                           (ImageUsageFlags)l_additional_attachment_usage_flags))},
        RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(p_allocate_info.render_target_dimensions, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::SHADER_DEPTH_ATTACHMENT |
                                                                                                                                           (ImageUsageFlags)l_additional_attachment_usage_flags))}};

    l_step.render_target_dimensions = p_allocate_info.render_target_dimensions;
    l_step.clear_values = Span<v4f>::allocate_slicen(SliceN<v4f, 2>{v4f{0.0f, 0.0f, 0.0f, 1.0f}, v4f{1.0f, 0.0f, 0.0f, 0.0f}});
    l_step.pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_attachments);
    l_step.global_buffer_layout = p_gpu_context.graphics_allocator.allocate_shader_layout(l_global_buffer_parameters, l_global_buffer_vertices_parameters, 0);

    Camera l_empty_camera{};
    l_step.global_material = Material::allocate_empty(p_gpu_context.graphics_allocator, 0);
    l_step.global_material.add_and_allocate_buffer_host_parameter_typed(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory.allocator,
                                                                        p_gpu_context.graphics_allocator.heap.shader_layouts.get(l_step.global_buffer_layout), l_empty_camera);

    return l_step;
};

inline void ColorStep::free(GPUContext& p_gpu_context)
{
    this->clear_values.free();
    p_gpu_context.graphics_allocator.free_shader_layout(this->global_buffer_layout);
    GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, this->pass);
    this->global_material.free_with_textures(p_gpu_context.graphics_allocator, p_gpu_context.buffer_memory);
};

inline void ColorStep::set_camera(GPUContext& p_gpu_context, const Camera& p_camera)
{
    slice_memcpy(p_gpu_context.buffer_memory.allocator.host_buffers
                     .get(p_gpu_context.graphics_allocator.heap.shader_uniform_buffer_host_parameters
                              .get(p_gpu_context.graphics_allocator.heap.material_parameters.get_vector(this->global_material.parameters).get(0).uniform_host)
                              .memory)
                     .get_mapped_memory(),
                 Slice<Camera>::build_asint8_memory_singleelement(&p_camera));
};

inline void ColorStep::set_camera_projection(GPUContext& p_gpu_context, const float32 p_near, const float32 p_far, const float32 p_fov)
{
    this->get_camera(p_gpu_context).get(0).projection = m44f::perspective(p_fov, (float32)this->render_target_dimensions.x / this->render_target_dimensions.y, p_near, p_far);
};

inline void ColorStep::set_camera_view(GPUContext& p_gpu_context, const v3f& p_world_position, const v3f& p_forward, const v3f& p_up)
{
    this->get_camera(p_gpu_context).get(0).view = m44f::view(p_world_position, p_forward, p_up);
};

inline Slice<Camera> ColorStep::get_camera(GPUContext& p_gpu_context)
{
    return slice_cast<Camera>(p_gpu_context.buffer_memory.allocator.host_buffers
                                  .get(p_gpu_context.graphics_allocator.heap.shader_uniform_buffer_host_parameters
                                           .get(p_gpu_context.graphics_allocator.heap.material_parameters.get_vector(this->global_material.parameters).get(0).uniform_host)
                                           .memory)
                                  .get_mapped_memory());
};

inline D3Renderer D3Renderer::allocate(GPUContext& p_gpu_context, const ColorStep::AllocateInfo& p_allocation_info)
{
    return D3Renderer{D3RendererAllocator::allocate(), ColorStep::allocate(p_gpu_context, p_allocation_info)};
};

inline void D3Renderer::free(GPUContext& p_gpu_context)
{
    this->buffer_step(p_gpu_context);

    this->allocator.free();
    this->color_step.free(p_gpu_context);
};

inline D3RendererHeap& D3Renderer::heap()
{
    return this->allocator.heap;
};

inline void D3Renderer::buffer_step(GPUContext& p_gpu_context)
{
    for (loop(i, 0, this->heap().model_update_events.Size))
    {
        auto& l_event = this->heap().model_update_events.get(i);

        Slice<int8>& l_mapped_memory =
            p_gpu_context.buffer_memory.allocator.host_buffers
                .get(p_gpu_context.graphics_allocator.heap.shader_uniform_buffer_host_parameters.get(this->allocator.heap.renderable_objects.get(l_event.renderable_object).model).memory)
                .get_mapped_memory();

        slice_memcpy(l_mapped_memory, Slice<m44f>::build_asint8_memory_singleelement(&l_event.model_matrix));
    };

    this->heap().model_update_events.clear();
};

inline void D3Renderer::graphics_step(GraphicsBinder& p_graphics_binder)
{
    p_graphics_binder.bind_shader_layout(p_graphics_binder.graphics_allocator.heap.shader_layouts.get(this->color_step.global_buffer_layout));
    p_graphics_binder.bind_material(this->color_step.global_material);

    p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.heap.graphics_pass.get(this->color_step.pass), this->color_step.clear_values.slice);

    for (loop(i, 0, this->heap().shaders_indexed.Size))
    {
        TokenT(ShaderIndex) l_shader_token = this->heap().shaders_indexed.get(i);
        ShaderIndex& l_shader_index = this->heap().shaders.get(l_shader_token);
        p_graphics_binder.bind_shader(p_graphics_binder.graphics_allocator.heap.shaders.get(l_shader_index.shader_index));

        auto l_materials = this->heap().get_materials_from_shader(l_shader_token);
        for (loop(j, 0, l_materials.get_size()))
        {
            TokenT(Material) l_material = l_materials.get(j);
            p_graphics_binder.bind_material(this->heap().materials.get(l_material));

            auto l_renderable_objects = this->heap().get_renderableobjects_from_material(l_material);

            for (loop(k, 0, l_renderable_objects.get_size()))
            {
                TokenT(RenderableObject) l_renderable_object_token = l_renderable_objects.get(k);
                RenderableObject& l_renderable_object = this->heap().renderable_objects.get(l_renderable_object_token);

                p_graphics_binder.bind_shaderbufferhost_parameter(p_graphics_binder.graphics_allocator.heap.shader_uniform_buffer_host_parameters.get(l_renderable_object.model));

                Mesh& l_mesh = this->allocator.heap.meshes.get(l_renderable_object.mesh);
                p_graphics_binder.bind_vertex_buffer_gpu(p_graphics_binder.buffer_allocator.gpu_buffers.get(l_mesh.vertices_buffer));
                p_graphics_binder.bind_index_buffer_gpu(p_graphics_binder.buffer_allocator.gpu_buffers.get(l_mesh.indices_buffer), BufferIndexType::UINT32);
                p_graphics_binder.draw_indexed(l_mesh.indices_count);

                p_graphics_binder.pop_shaderbufferhost_parameter();
            }

            p_graphics_binder.pop_material_bind(this->heap().materials.get(l_material));
        }
    }

    p_graphics_binder.end_render_pass();

    p_graphics_binder.pop_material_bind(this->color_step.global_material);
};
}; // namespace v2