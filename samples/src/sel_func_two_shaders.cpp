///////////////////////////////////////////////////////////////////////
//
// sel_func_two_shaders.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 22 September 2014
//
// for OpenGL 3.3 and above
//
///////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <GLUT/GLUT.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif
#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint	vao;
GLuint	vbo;
GLuint	h_prog[2];
GLuint	idx_prog = 0;

void init(void)
{
	GLfloat vertices[6][2] = {
		{ -0.90, -0.90 }, // Triangle 1
		{  0.85, -0.90 },
		{ -0.90,  0.85 },
		{  0.90, -0.85 }, // Triangle 2
		{  0.90,  0.90 },
		{ -0.85,  0.90 }
	};

	const char	src_vert[] = {
		"#version 330 core	\n"
		"layout(location = 1) in vec4 vPosition;	\n"
		"void	\n"
		"main()	\n"
		"{	\n"
		"\t	gl_Position = vPosition;	\n"
		"}	\n"
	};
	const char src_frag_0[] = {
		"#version 330 core\n"
		"out vec4 fColor;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tfColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
		"}\n"
	};

	const char src_frag_1[] = {
		"#version 330 core\n"
		"out vec4 fColor;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tfColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n"
	};
	const char	*src;
	GLuint	h_vert = glCreateShader(GL_VERTEX_SHADER);
	src = src_vert;
	glShaderSource( h_vert, 1, &src, NULL );
	glCompileShader( h_vert );

	GLuint	h_frag;

	h_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = src_frag_0;
	glShaderSource( h_frag, 1, &src, NULL );
	glCompileShader( h_frag );

	h_prog[0] = glCreateProgram();
	glAttachShader( h_prog[0], h_vert );
	glAttachShader( h_prog[0], h_frag );
	glLinkProgram( h_prog[0] );

	glDeleteShader(h_frag);

	h_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = src_frag_1;
	glShaderSource( h_frag, 1, &src, NULL );
	glCompileShader( h_frag );

	h_prog[1] = glCreateProgram();
	glAttachShader( h_prog[1], h_vert );
	glAttachShader( h_prog[1], h_frag );
	glLinkProgram( h_prog[1] );

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Thr order of bindings of VBO & VAO does not matter,
	// but they both should be bound before calling glVertexAttribPointer!
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case ' ':
			idx_prog = (idx_prog+1)&0x01;	// toggles between 0 & 1
			glutPostRedisplay();	// Forces to redraw the screen!
			break;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(h_prog[idx_prog]);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glFlush();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef  __APPLE__
	glutInitDisplayMode(GLUT_RGBA | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow(argv[0]);
#else
	glutInitDisplayMode(GLUT_RGBA);
	glutInitContextVersion(3,3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
	glutCreateWindow(argv[0]);
	if (glewInit()) {
		std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
		exit(EXIT_FAILURE);
	}
#endif
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
}
