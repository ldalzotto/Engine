The render middleware is the interface between the scene and the render system. <br/>
It allocates scene components and update their render state when associated nodes are moving.

# Architecture

<svg-inline src="render_middleware_architecture.svg"></svg-inline>

* CameraComponent: Attach a camera object to a node.
* MeshRendererComponentUnit: Allocate render components

## MeshRendererComponent

The meshrenderer component uses a material resource and a mesh resource to allocate and link a renderable object.

# 3D position update

Every frame, the system checks if any of the meshrenderer components and camera component associated node has
moved. If that's the case, then a model matrix update event is sent to the render system.