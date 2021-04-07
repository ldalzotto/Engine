A node hierarchy where every node can have components.

The node hierarchy allows to position points in 3D space where parent movement influence the child position. <br/>
Components are functionality attached to the node that communicate with internal systems vie the scene middleware.

# Architecture

<svg-inline src="scene_architecture.svg"></svg-inline>

* Node: a point int 3D space.
* SceneTree: the hierachical view of nodes. All node movement must be done through the SceneTree.
* Component: a component is a typed external object associated to a node.
* Scene: an object that link nodes to components. It acts as an interface of the scene tree.

<svg-inline src="scene_model.svg"></svg-inline>

# Scene hierarchical update

The node is composed of a local transform (position, rotation, scale) and a local to world matrix. <br/>
When updating a node transform, what we change is the local transform. The local to world matrix is recalculated on the fly, only when needed. THe local to world matrix value is cached until the transform change again.

Because nodes are organized in a tree, the parent movement affect the child position. When a node parent position is set, this change is propagated recursively into the tree. This propagation is done instantaneously.

When a node transform has changed, the internal state of the node indicates that this frame, the node has changed. This state can be used by middleware to check if a node has moved or not.

> **WARNING:** The state of the node is cleared every frame.

# Component allocation

The component is a structure that contains a type and an external object token.

When we want to add a component to a node, it's external ressource has already been allocated by the scene middleware (or at least, a token has been generated). So, we provide this token to the node component allocation.

<svg-inline src="scene_component_allocation.svg"></svg-inline>

# Component deallocation events

Because component deallocation can be triggered manually by the user logic on indirectly by removing a node, component deallocation events are stored to an event buffer.

From the point of view of the scene, we have no idea of what is the external component object allocated or what is it's purpose. So we need to send notifications when the component is removed.
To do so, the component deallocation event buffer can be consumed to execute functions accordingly to the component type.

> **WARNING**: The component deallocation event buffer is cleared at the end of every frame. If events are not consumed before that, then events will be lost.