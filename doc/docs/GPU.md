The GPU module is responsible of :

* Providing convenvient systems to allocate, free and write to GPU memory chunks.
* Deferring commands that needs synchronization with the gpu.
* Abstracting graphics objects such as Shader, Material ...

# Architecture

<svg-inline src="./gpu_arch.svg"></svg-inline>

The GPU module is composed of :

* GPUHeap : allocates huge chunk of GPU memory and cut slice into it based on vulkan constraint.
* BufferAllocator : allocates and holds token of vulkan buffer objects.
* BufferEvents : stores all GPU operations that are deferred to be executed on a command buffer.
* BufferMemory : acts as an entry point for memory allocation and read/write event push.
* GraphicsHeap : stores token of graphics objects.
* GraphicsAllocator : allocates abstracted graphics objects.
* GraphicsBinder : uses graphics objects to execute draw commands.

On initialisation, the GPU systems creates two command buffer objects that are responsible of oredring the GPU to
execute draw operations or buffer copy operations.

Before the start of the frame, these command buffers are flushed from previous commands. At the end of the frame, these
command buffers are submitted in the following order :

1. Transfer command buffer : responsible for buffer copy operations and texture layout transition.
2. Graphics command buffer : responsible for binding graphics objects and draw vertices.

# GPU memory allocation

The GPU memory allocation is done by allocating one or more huge chunks of memory on the GPU. Any GPU allocation or
deallocation works with a slice of memory of the huge chunk.

Memory requirements such as size and alignment is provided by the vulkan API. From these constraint, we manually
identify and cut a slice of memory from the GPUHeap. Every heap has it's own memory type defined by the the types of
memory requested.

## Buffer

A Buffer is a general purpose memory (like if you allocate memory from CPU). A buffer can be of two type :

* _Host means that the memory pointer can be directly mapped and visible from CPU. The memory is still allocated on the
  GPU but reading from it may be less efficient.
* _GPU means that the memory pointer cannot be visible from CPU. In order to read/write to it, we must create a
  temporary Host buffer and execute command to transfer data from/to the GPU.

All buffer allocations are instant.

1. Ask vulkan for memory requirements.
2. Cut a memory from an already allocated buffer.

BufferHost read/write are instantaneous because their memory type corresponds to a host heap chunk that has already been
mapped.

BufferGPU read/write have an additional level of indirection as a temporary Host buffer must be created and a gpu
operation must be registered to a command buffer.

## Image

An Image is allocated the same way as a Buffer. Instead of passing a size in bytes, we use an image format object.

```
struct ImageFormat
{
    VkImageAspectFlags imageAspect;
    VkImageType imageType;
    ImageUsageFlag imageUsage;
    VkFormat format;
    v3ui extent;
    uint32 mipLevels;
    uint32 arrayLayers;
    VkSampleCountFlagBits samples;
};
```

> **WARNING**: mipLevels and arrayLayers are not supported yet.

The vulkan api let the user manage an internal state of an image called ImageLayout. It acts as a flag to indicates what
operations are allowed on the image.

The imageUsage flag indicates where this image will be used (as a shader parameter, or as the target of a render pass)
. <br/>
This flags is the only parameter used to set the underlying image layout.

Because the vulkan API needs a specific ImageLayout value when the image is being read or written to, every operations
on image must be deferred to execute layout transition before and after.

## BufferEvents step

The BufferEvents stores all deferred GPU buffer read/write and texture copy events. <br/>
These events are consumed and the beginning of a frame by the transfer command buffer. <br/>
Events are consumed in the following order :

1. Image allocation event to execute image layout transition to it's target layout.
2. Read/Write to BufferGPU.
3. Read/Write to ImageGPU.

> **WARNING**: It is very important that image host and gpu allocation events are processed before image copy operations because further events assume that the image layout of the image is the targetted one.

### Image allocation

When an image is allocated, it's image layout is undefined. This events execute image layout transition based on the
image imageUsage.

1. ImageLayout transition from unknown state to desired imageUsage.

### Read/Write to BufferGPU

1. Push copy commands to the temporary buffer (as decribed in the [buffer section](#buffer))

### Read/Write to ImageGPU

1. ImageLayout transition from desired imageUsage to transfert_src or transfert_dst depending on the operation.
2. Copy command.
3. ImageLayout transition from transfert_src or transfert_dst to desired imageUsage.

# Graphics abstraction

All graphics abstraction objects describe how a draw command is performed. Allocation of any graphics object is done by
having a reference to the BufferMemory object because they can cause buffer or image allocation. <br/>
Graphics abstraction is a layer on top of the GPU memory layer.

There is no depedencies between all these objects. This choice has been made to not enforce any hierarchy. The
dependency between objects is ensured by the GraphicsBinding.

**TextureGPU:**

A TextureGPU is an Image with a description of which mip map or arraylayer is selected.

**GraphicsPass:**

The GraphicsPass is the render texture attachments that will be used for drawing. <br/>
Texture attachment supported are color attachment and depth attachment.

**Shader:**

A Shader is a program that is executed against a GraphicsPass and a set of ShaderParameters. <br/>
On allocation, a shader is configured to decide wether ztest and zwrite is perfored.

**ShaderModule:**

A ShaderModule is the compiled small unit of (vertex|fragment) shader pass executed by the Shader.

**ShaderLayout:**

The ShaderLayout indicate the format of all parameters of the Shader (including vertex format). It can be seen as the
Reflection data of a Shader.

**ShaderParameter:**

A ShaderParameter is a buffer or image that is binded at a certain location of the shader to be used by the
program. <br/>
ShaderParameters can be UniformHost, UniformGPU or TextureGPU.

**Material:**

A Material is an array of ShaderParameters. 

# Graphics binding

The graphics binding object allows to notify to the GPU all data needed to perform a draw call. <br/>
Binding draw commands are sended to the graphics command buffer. A specific bindind order must be respected as
validation is performed for every binding. <br/>

<svg-inline src="./gpu_bind_order.svg"></svg-inline>

Once an object has been binded, it will still be binded until another one takes it's place. This means that we don't
have to re-bind the GraphicsPass or the Shader when we bind another set of ShaderParameters. <br/>
ShaderParameters can be binded at any position, they also stay binded at the desired index until another one takes
place.

# Presentation

The presentation is a direct usage of the GraphicsBinding. The presentation layer consists of a SwapChain render the
input render texture to a present texture. <br/>
Present textures are allocated internally by the vulkan API by providing a native handle of the window. The SwapChain
allocates all GraphicsObjects necessary for drawing the render texture to a present texture. <br/>
The presentation module is optional, it is up to the consumer to decide whether or not to allocate one and include it
the the GraphicsBinding loop.

**Input :**

* Compiled image quad blit shader module.
* Already allocated render texture.

## Window resize event

When the native window is resized internal present textures are reallocated to fit the window dimensions.