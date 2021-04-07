
# The philosophy of the engine

The execution flow of the engine can be seen as a succession of shader pass execution. <br/>
For example, let's say that the engine needs to allocate some render objects. What the engine does is reading a buffer where is stored all events of allocation and execute them all before continuing exectuion. <br/>
The whole engine comes together by chaining multiple execution passes in the right order in order to draw a frame.

# Execution graph

Here is the engine chain of execution represented as a graph:

<svg-inline src="architecture_overview_execution.svg"></svg-inline>

## Main loop

The main loop is composed of execution units. Execution units are step that trigger all dependant execution block in the order defined by the dependencies. <br/>
For exemple, we can see that the Render system is updated when the following execution units are completed : Render allocation, Render middleware and the GPU buffer step.

Some execution unit within the main loop are called "user logic". These execution units are hooks that the engine user can hook on to execute custom logic at this specific point.

> Custom logic execution units are likely to create node, add component or move node. Thus, we must be sure that all scene middleware steps are called before the scene tree events are cleaned.

## Scene Tree

An Engine instance is associated to a unique Scene Tree. The Scene Tree is a hierachical collection of Nodes located in 3D space. Every nodes can have multiple components attached to them. These components act as a link between the user and the internal systems. ([scene tree](scene.md))

<svg-inline src="architecture_overview_scene_tree.svg"></svg-inline>

## Allocations

The allocation steps act as an interface for allocating system objects. Allocations input can either be provided directly by the user or by requesting the asset database. ([ressource](ressource.md))

## Scene middleware

Scene middlewares are where node component ressources are stored. They act as the communication layer between the scene tree and the internal systems.

Because nodes can move in 3D space. The state of the Node is updated when it's position, rotation or scale has changed. <br/>
The Scene Middlewares are the consumers of this state. When a Node has moved, it's they send events to internal systems to take this change into account. ([scene middleware](SceneMiddleware.md)).

# Interaction with user

If we want to make a node display a 3D mesh, the user must :

 1. Allocate a Node and set it's 3D coordinates.
 2. Allocate a mesh renderer component ressource from the render middleware.
 3. Add a component to the Node and link it with the mesh renderer token.

# Asset management

All assets (3DModels, textures, user defined assets...) are stored in a unique local database. Asset files are written either in a human readable format (.json, .obj, ...) or compressed (.png). However, when stored in the database, they are compiled into a blob that allows the engine to map it to internal objects with little efforts.

This compilation is done during the build process, so we eliminate the runtime overhead of loading and decoding file formats. ([asset database](AssetDatabase.md))