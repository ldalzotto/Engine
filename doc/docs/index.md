# Welcome

This project is a 3D engine created as a personal project and coded in C++. <br/>
The engine features :

* 3D scene hierarchy manipulation.
* 3D Graphics abstraction powered by vulkan.
* Asset compilation to database.
* Handmade 3D mathematics.
* Manual manipulation of CPU and GPU memory.

For an overview of the engine architecture see [architecture overview](https://ldalzotto.github.io/EngineCPPRewrite/#architecture_overview/).

# What this project can do ?

It allows the user to spawn and move 3D nodes in a hierarchical scene. <br/>
Every node can display a mesh with a material fetched from the asset database or by manually providing asset data. <br/>
The engine works with an asset database that stores precompiled asset files.

![](https://i.imgur.com/L1YIDMH.gif)

An asset database can be fetched by a tool to view it's content.

![](https://s4.gifyu.com/images/ezgif-3-997f1678fb0e.gif)

# How it's coded ?

This project is a learning journey, so as much as possible, it will use handmade solutions (memory containers, maths
library, 3D scene management, 3D graphics abstraction, OS interactions).

I would like to carry this project as far as possible and take pleasure to build it. So to avoid any frustrations during the development process, every
layer of the engine has automated test. This greatly improve the quality of development and debugging sessions.

Compared to my other projects, I am focusing more on implementation details and having clean interfaces for every modules before implementing something new.

# Coding rules

These are a set of rules that I have imposed myself for this journey :

* The standard library is never used (no std::something), every container and algorithms are built from scratch.
* RAII C++ feature is never used. Memory is manually allocated and deallocated.
* Inheritance and virtual functions are forbidden. All data structures are plain structs.
* Singletons are forbidden. If we want to execute an algorithm that involves multiple systems, then these systems must
  be carried over as function parameters.
* Forward declarations are forbidden (struct, functions and member functions). The whole codebase is an aggregation of
  layers, where the Engine structure is at the top. All functions are implemented inline with their definition.
* As much as possible, avoid nested if statements and consider implementing a specialized version of the function.
* Aside from memory manipulation, working with raw pointers is forbidden. The codebase works with token instead of
  pointers. Tokens are essentially indices, it provides safety when the underlying memory is reallocated or moved. To
  dereference a token, the token must be presented back to the structure that generated it.

All these rules enforce a code that is procedural, leveraging C++ template. <br/>
With this code style, isolation of every layer is trivial and testing every layer is simple (although quite verbose).

# Third party

Usage of third party libraries is limited to :

1. [sqlite3](https://github.com/sqlite/sqlite) for database
2. The C vulkan headers.
3. [glslang](https://github.com/KhronosGroup/glslang) for compiling shaders to be understandable by vulkan.
4. [stbimage](https://github.com/nothings/stb) for png loading. ***(a)***
5. [Qt](https://github.com/qt/qt5) for gui tools implementation. ***(b)***

> ***(a)*** There is a work in progress to roll a custom png loader with zlib. Thus removing stbimage for zlib.

> ***(b)*** If one day the engine supports GUI programming, then the Qt dependency can be replaced by it.

# Try it

Both previous engine build used to record gifs are available at the [release page](https://github.com/ldalzotto/EngineCPPRewrite/releases/tag/0.0.1). Extract archives and run executables. 