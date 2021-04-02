# Render

The Render module is the 3D renderer of the engine. It registers all graphics objects and organize them in a hierarchy
to call GPU graphics bindings against them. <br/>
The render module creates an internal 2D render target texture and draw the hierachy to it every frame.

# Architecture

<svg-inline src="./D3Renderer_arch.svg"></svg-inline>

The render module is composed of :

* ColorStep : responsible of the graphics passes supported by the module.
* D3RendererHeap : holds values of the render hierarchy.
* D3RendererAllocator : ensure the coherence of the render hierarchy (links between graphics objects).
* D3RendererEvents : acts as a buffer that store references of all renderable objects that will update it's model
  matrix.

# Data model

```

* RenderableObject
    * Mesh
    * ShaderUniformBufferHostParameter (model)
  
* Mesh
    * BufferGPU (vertices_buffer)
    * BufferGPU (indices_buffer)
  
* ShaderIndex
    * Shader
    * ShaderLayout

(ShaderIndex)(1)--(0..*)(Material)(1)--(0..*)(RenderableObject)
(RenderableObject)(1)--(1)(Mesh)

```

**Mesh:**

Holds reference to a vertex and index GPU buffer. <br/>
It is up to the render module consumer to define the vertex buffer format. It must match with the shader vertex
input. <br/>
By default, mesh buffers are GPU allocated because we suppose that their vamue won't be modified often.

**RenderableObject:**

The renderable object is the representation of an object in 3D space with it's shape. <br/>
It holds a reference to a Mesh and a model matrix. The model matrix is alaways host shader parameter because it's value
is subject to change often.

**Camera:**

The camera is from where the whole hierarchy is rendered. <br/>

> **WARNING**: only one instance of camera can be instantiated for now.

**ShaderIndex:**

A shader index is a Shader with a layout and an execution order.

# Render hierarchy

Every frame, the render hierarchy is binded to a GraphicsBinder by following an order :

<svg-inline src="./render_hierarchy.svg"></svg-inline>

This hierarchy impose that global buffer are binded before everything else and that the model buffer is binded at the
very end. <br/>
This contraint is translated in the shader source code :

```

struct Camera
{
    mat4 view;
    mat4 projection;
};

layout(set = 0, binding = 0) uniform camera { Camera cam; };
// insert all material buffer or samplers
layout(set = END, binding = 0) uniform model { mat4 mod; };

```

## Shader ordering

ShaderIndex inside the hierarchy are ordered by their execution order. This will come useful when we want to add
transparency pass for exemple.

# Model matrix update

Once a RenderableObject has been allocated, consumers of the render module can notify the renderer that an object has
moved (pos, rot, scale) by sending a ModelUpdateEvent.

When consumed, the ModelUpdateEvent will update the model buffer by the inputted matrix.

