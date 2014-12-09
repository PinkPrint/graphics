///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 15 July 2013
//
///////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#if defined(WIN32) || defined(WIN64)
#include <GL/freeglut.h>
#elif   defined __APPLE__
#include <OpenGL/OpenGL.h>
#include <GL/glfw.h>
#endif
#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;

void init(void)
{
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Triangles]);
	GLfloat vertices[NumVertices][2] = {
		{ -0.90, -0.90 }, // Triangle 1
		{ 0.85, -0.90 },
		{ -0.90, 0.85 },
		{ 0.90, -0.85 }, // Triangle 2
		{ 0.90, 0.90 },
		{ -0.85, 0.90 }
	};
	glGenBuffers(NumBuffers, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
	vertices, GL_STATIC_DRAW);

	const char	src_vert[] = {
		"#version 430 core\n"
		"layout(location = 0) in vec4 vPosition;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tgl_Position = vPosition;\n"
		"}\n"
	};
	const char src_frag[] = {
		"#version 430 core\n"
		"out vec4 fColor;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tfColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
		"}\n"
	};
	const char	*src;
	GLuint	h_vert = glCreateShader(GL_VERTEX_SHADER);
	src = src_vert;
	glShaderSource( h_vert, 1, &src, NULL );
	glCompileShader( h_vert );

	GLuint	h_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = src_frag;
	glShaderSource( h_frag, 1, &src, NULL );
	glCompileShader( h_frag );

	GLuint h_prog = glCreateProgram();
	glAttachShader( h_prog, h_vert );
	glAttachShader( h_prog, h_frag );
	glLinkProgram( h_prog );

	glUseProgram(h_prog);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT,
	GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(VAOs[Triangles]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glFlush();
}

int main(int argc, char** argv)
{
#if defined(__APPLE__)
    glfwInit();
    
    glfwEnable(GLFW_AUTO_POLL_EVENTS); /* No explicit call to glfwPollEvents() */
    
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_FALSE);
    
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#if defined(NDEBUG)
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
#else
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    
    GLboolean Result = glfwOpenWindow(512, 512, 0, 0, 0, 0, 16, 0, GLFW_WINDOW);
    assert(Result == GL_TRUE);
    
	glewExperimental = GL_TRUE;
	glewInit();
    /*
	if(::LoadFunctions() == LS_LOAD_FAILED)
	{
		exit(1);
	}
	glTexImage3D = (PFNGLTEXIMAGE3DPROC)glfwGetProcAddress("glTexImage3D");
     */
	
    
    glfwSetWindowTitle("triangles");
    //    glfwSetMousePosCallback(cursor_position_callback);
    //    glfwSetWindowCloseCallback(close_callback);
    //    glfwSetKeyCallback(glfw_key);
//    glfwSetKeyCallback(special);
//	glfwSetCharCallback(keyboard);
  //  glfwSetWindowSizeCallback(reshape);
    
    GLint MajorVersionContext = 0;
    GLint MinorVersionContext = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &MajorVersionContext);
    glGetIntegerv(GL_MINOR_VERSION, &MinorVersionContext);
    
    printf("OpenGL version = %d.%d\n", MajorVersionContext, MinorVersionContext);
    
    init();
    
	print_summary();
    
    while(true)
    {
        if(terminating)
            break;
        display();
    }
    
    glfwTerminate();
    
    exit(EXIT_SUCCESS);
    
    return 0;
    
#else
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(4,3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
	glutCreateWindow(argv[0]);
	if (glewInit()) {
		std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
		exit(EXIT_FAILURE);
	}
	init();
	glutDisplayFunc(display);
	glutMainLoop();
#endif
}
