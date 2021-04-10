From Scratch is a personal project created in C++ as a way to improve my understanding and skill of :

* Manual manipulation of cpu and gpu memory.
* Handmade abstraction of memory management and container.
* Manipulating a 3D graphics api.
* Handmade 3D mathematics.

For an overview of the engine architecture see [architecture overview](architecture_overview.md).

# Coding rules

These are a set of rules that I have imposed myself for this journey :

* The standard library is never used (no std::something), every containers and algorithms are builded from scratch.
* RAII C++ feature is never used. Memory is manually allocated and deallocated.
* Inheritence and virtual functions are forbidden. All data structures are plain structs.
* Singletons are forbidden. If we want to execute an algorithm that involves multiple systems, then these systems must
  be carried over as function parameters.
* Forward declarations are forbidded (struct, functions and member functions). The whole codebase is an aggregation of
  layers, where the Engine structure is at the top. All functions are implemented inline with their definition.
* As much as possible, avoid nested if statements and consider implementing a specialized version of the function.
* Aside from memory manipulation, working with raw pointers is forbidded. The codebase works with token instead of pointers. 
  Tokens are essentially indices, it provides safety when the initial memory is reallocated or moved.
  To dereferrence a token, the token must be presented back to the structure that generated it. 

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
