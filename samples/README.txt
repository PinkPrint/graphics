----------------------------------------------------------------------------------------------------------------------
How to check the OpenGL version on your computer? (2014/9/11)

* Windows
- DoctorGL by onOne software: http://www.ononesoftware.com/support/607/
	(direct link: http://ononesoft.cachefly.net/support/DoctorGL.exe.zip)
- glVersion by Topicscape: http://www.topicscape.com/download3.php

* OSX
- Install "OpenGL Extension Viewer" from the AppStore.

----------------------------------------------------------------------------------------------------------------------
Compiling on OpenGL 3.1 (2014/9/11)

Try "triangles_310.cpp" instead of "triangles.cpp". (Check out the differences between them!)
----------------------------------------------------------------------------------------------------------------------
How to build the samples (2014/9/4)
* tested with "Visual Studio 2010 Professional (SP1)" on "Windows 7 Professional"

0. First, create a folder where all your OpenGL-related work will be stored. Let's call it <my_OpenGL>
1. Build freeglut
	(1) Download the source codes from http://freeglut.sourceforge.net
	(2) Extract it somewhere. A folder named like "freeglut-x.x.x" will be created. Let the folder name <freeglut>.
	(3) Go to <freeglut>/VisualStudio/2010 and open "freeglut.sln".
	(4) Select "Release" and "Win32" options.
	(5) We only need to build freeglut. Select the "freeglut" project -> right click -> Project only -> Build only freeglut
	(6) "freeglut.dll" and "freeglut.lib" are created in <freeglut>/lib/x86.

2. Build glew
	(1) Download the source codes from http://glew.sourceforge.net
	(2) Extract it somewhere. A folder named like "glew-x.x.x" will be create. Let the folder name <glew>.
	(3) Go to <glew>/build/vc10 and open "glew.sln".
	(4) Select "Release" and "Win32" options.
	(5) We only need to build "glew_shared". Select the "glew_shared" project -> right click -> Project only -> Build only glew_shared.
	(6) "glew32.lib" is created in "<glew>/lib/Release/Win32" and "glew32.dll" is created in "<glew>/bin/Release/Win32".

3. Build a sample
	(1) Create a folder <my_OpenGL>/samples/include/GL and copy the header files (*.h) in "<freeglut>/include/GL" and "<glew>/include/GL" there.
	(2) Create a folder <my_OpenGL>/samples/lib and copy "glew32.lib" and "freeglut.lib" there.
	(3) Create a project with
		- Template: Empty Project
		- Name: samples
		- Location: <my_OpenGL>/samples
		- "Create directory for solution" option unchecked
	(4) Download "triangles.cpp" in <my_OpenGL>/samples/samples and add it to the "Source Files" of the project just created.
	(5) Set the project properties as follows:
		- C/C++ --> General --> Additional Include Directories: ../include
		- Linker --> General --> Additional Library Directories: ../lib
		- Linker --> Input --> Additional Dependencies: Add "glew32.lib" and "freeglut.lib"
	(6) Start build.

4. Run a sample
	(1) Copy "glew32.dll" and "freeglut.dll" in <my_OpenGL/samples/samples.
	(2) Run the sample from the Visual Studio.


----------------------------------------------------------------------------------------------------------------------

Chapter 2: Shader Fundamentals
triangles.cpp                  : minimal OpenGL 3.30 sample (No error checking, embedded shaders)
triangles_310.cpp              : minimal OpenGL 3.10 sample (No error checking, embedded shaders)
shader_check.cpp               : minimal sample with GLSL error checking
shader_files.cpp               : loads external shader source files (triangles.vert & triangles.frag)
uniform.cpp                    : how to use uniform variables
uniform_anim.cpp               : a simple animation using a uniform variable
attribs_int.cpp                : integer vertex attributes
sel_func_two_shaders.cpp       : how to select GLSL function dynamically (using two different shader programs)
sel_func_uniform_var.cpp       : how to select GLSL function dynamically (using a uniform variable and if~else statement)
sel_func_subroutine.cpp        : how to select GLSL function dynamically (using GLSL subroutines >= OpenGL 4.0)
sel_func_subroutine_layout.cpp : how to select GLSL function dynamically (using GLSL subroutines & layout specifier >= OpenGL 4.3)


Chapter 3: Drawing with OpenGL
triangles_two_attribs.cpp     : using two attributes
triangles_uniform.cpp         : using a uniform variable
quad.cpp                      : rendering a quad using three ways (GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN)
attribs.cpp                   : how to use multiple attributes
quad_indexed.cpp              : how to use indexed rendering
mesh_assimp.cpp               : how to load & render mesh objects using Assimp library
BufferSubData.cpp             : how to use glBufferSubData()
culling.cpp                   : how to use culling
default_VAO_310.cpp           : tests the default VAO on OpenGL 3.1
default_VAO_330.cpp           : tests the default VAO on OpenGL 3.3
DrawArraysInstanced.cpp       : how to use glDrawArraysInstanced()
DrawElements.cpp              : how to use glDrawElements()
MultiDrawArrays.cpp           : how to use glMultiDrawArrays()
one_vbo_for_multiple_vaos.cpp : tests one VBO for multiple VAOs
PolygonMode.cpp               : how to use glPolygonMode
primitive_restart.cpp         : how to use primitive restart feature
quad_map.cpp                  : how to use glMapBuffer
instanced_vert_attribs.cpp    : how to use "instanced vertex attributes"
InstanceID.cpp                : instanced rendering by texture look-up using gl_InstanceID

Chapter 4: Colors, Pixels, and Framebuffers
ColorMask.cpp     : how to use colour mask
frag_analytic.cpp : computing fragment color analytically
msaa.cpp          : how to enable MSAA (MultiSample Anti-Aliasing) feature
stencil.cpp       : how to use the stencil buffer
scissor.cpp       : how to use the scissor test
depth.cpp         : how to enable depth buffering (z-buffering)
blend.cpp         : how to use blending
fbo_rbo.cpp       : how to use FBO (framebuffer object) + rbo (renderbuffer object)
fbo_tex.cpp       : how to use FBO (framebuffer object) + texture

Chapter 5: Viewing Transformations, Clipping, and Feedback
xfm_no_glm       : basic 3D transformations without using the GLM library
xfm_with_glm     : basic 3D transformations using the GLM library
xfm_with_glm_2   : How to use the 1st parameter of GLM transformation functions (rotate, scale, translate, etc.)
xfm_anim         : simple animation
xfm_lookat       : how to use glm::lookAt function for view transformation
xfm_hierarchy    : hierarchical transformations without using any stack
xfm_stack        : hierarchical transformations using a stack
xfm_viewport     : viewport mapping to avoid screen distortion
xfm_viewport2    : another viewport mapping
viewer           : intuitive GUI

Chapter 6: Textures
quad_tex_DevIL   : how to load external image files as textures using DevIL
quad_tex_GLI     : how to load external image files as textures using GLI
quad_tex_multi   : multi texturing
quad_tex_sampler : how to use samplers
quad_tex_mipmap  : how to use mipmap (glGenerateMipmap)
tex_cubemap      : how to use cubemap texture

Chapter 7: Light and Shadow
shading          : shading ("Phong OR Blinn-Phong reflection model" + "Phong OR Gouraud interpolation")
lights           : how to use various light types (directional, positional, spot)
two_sided        : two-sided lighting
assimp_mat       : how to handle the material data in assimp


miscellaneous
text             : text rendering
mouse_rotate     : mouse UI to rotate an object
picking_color    : picking
mouse_examine    : mouse UI to rotate & move an object



For OSX (Mac)
-The samples assume that “glm”, “assimp”, “DevIL”, and “freetype2” libraries are installed in /opt/local using MacPorts.


