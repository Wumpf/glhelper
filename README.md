glhelper
========

Helper &amp; thin abstraction classes for OpenGL **4.5**.  
_No compromises!_ Assumes full avaibility of direct state access, texture/buffer storage and so on.

This is **not** a complete API abstraction, rather a bunch of helper classes!

Platform/Programming Language
--------
* Makes heavy use of some C++11 stuff like enum class, initializer lists, variadic templates...
* Should be platform independent but I only tested win x64 NVIDIA.  
(Simply because there are not may drives out there yet ;)

Development
--------
Used in some personal (experimental!) projects. Functionallity is extendend on personal necessity.  
However, feedback from fellow OpenGL users is warmly welcome :)

Contents
--------
* Shader abstraction
  * File loading
  * `#include` parsing & resolve
  * Reflection via OpenGL functions (e.g. for uniform variable positions etc.)
  * "hooks" for reloading (very useful for recompile on file change)
* Buffer
  * Texture and Uniform Buffer view
  * Easy creation from shader reflection
  * Memorizes creation information and bindings (avoids redundant ones)
* Texture (2D)
  * Memorizes creation information and bindings (avoids redundant ones)
* Sampler
  * Simplified creation
* Framebuffer
  * Easy creation from multiple textures
* Error handling & Check mechanism
* Wraps many OpenGL defines in enums to avoid invalid GL calls

Notes on how to use
--------
* Not intended to be compiled into a library. Just add the code to the project and add your own glhelperconfig.hpp.  
(just copy defaultconfig/glhelperconfig.hpp and change it according to your needs)  
* Assumes that there is a OpenGL header <GL/GL.h> which defines everything that is needed (I use GLEW).  
* Context creation etc. is (intentionally) up to the user!
  * I recommend GLFW :)
