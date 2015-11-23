glhelper
========

Helper &amp; thin abstraction classes for OpenGL **4.5**.  
_No compromises!_ Assumes full availability of direct state access, texture/buffer storage and so on.

This is **not** a complete API abstraction, rather a bunch of helper classes!

Platform/Programming Language
--------
* Makes heavy use of some C++11 stuff like enum class, initializer lists, variadic templates...
* Should be platform independent but I only tested win x64 NVIDIA. 

Development
--------
Used in some personal (experimental!) projects. Functionallity is mostly extendend on personal necessity.  
However, feedback from fellow OpenGL users is warmly welcome :) 
I am aware of the lack of unit tests. But since I lack time and motivation to write them, I just use the projects using this library as (bad-coverage) tests.

Contents
--------
* Shader abstraction
  * File loading
  * `#include` parsing & resolve
  * Reflection via OpenGL functions (e.g. for uniform variable positions etc.)
    * Info can be used to fill arbitrary memory
  * "hooks" for reloading (very useful for recompile on file change)
* Buffer
  * Can be used as Vertex/Index/Uniform/ShaderStorage/IndirectDraw/IndirectDispatch -Buffer
  * Memorizes creation information and bindings (avoids redundant ones)
  * Various checks for wrapped functionallity
* Persistent Ring-Buffer 
  * Helper class on top of buffer to provide an easy interface for a low driver overhead write-only ring buffer ("AZDO style")
  * Warns automatically if GPU-CPU syncs happen
* Vertex Array Object
  * Comfortable abstraction for usage as vertex format declaration  
    (possible using ARB_vertex_attrib_binding, which is Core in OpenGL 4.3)
* Textures
  * Memorizes creation information and bindings (avoids redundant ones)
  * 2D, 3D, Views
* TextureBufferView
  * Wrapper for Buffer to use it as texture buffer
* Sampler
  * Simplified creation
  * Reuse of already created sampler objects (all existing ones are hashed)
* Framebuffer
  * Easy creation from multiple textures
* State Wrapping
  * [Only glEnable/Disable so far]
  * Redundant state change checking and enums
* Error handling & Check mechanism
* Wraps many OpenGL defines in enums to avoid invalid GL calls and provide an overview over all possibilities

Notes on how to use
--------
* Not intended to be compiled into a library. Just add the code to the project and add your own glhelperconfig.hpp.  
(just copy defaultconfig/glhelperconfig.hpp and change it according to your needs)  
* Assumes that there is a OpenGL header <GL/GL.h> which defines everything that is needed (I use GLEW).  
* Context creation etc. is (intentionally) up to the user!
  * I recommend GLFW :)
 
Todos
--------
(just notes, in no specific order)
* multi bind-functions for vertex/index/uniform/shaderstorage buffers
* better buffer destructor: Iterates currently over all possible bindings to update intern binding tables
* add support for cube textures
* add support for 2d texture arrays
* add support for 3d texture arrays
* add wrapper for drawing and viewport settings
* add wrapper for glGet
  * at least for constant device properties like texture size, UBO padding, ...
