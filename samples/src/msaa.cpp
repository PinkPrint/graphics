///////////////////////////////////////////////////////////////////////
//
// msaa.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 1 October 2014
//
///////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
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

void init(void)
{
	glEnable(GL_MULTISAMPLE);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLfloat vertices[][2] = {
		{ -0.90, -0.90 }, // Triangle 1
		{ 0.85, -0.90 },
		{ -0.90, 0.85 },
		{ 0.90, -0.85 }, // Triangle 2
		{ 0.90, 0.90 },
		{ -0.85, 0.90 }
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
	vertices, GL_STATIC_DRAW);

	const char	src_vert[] = {
		"#version 330 core\n"
		"layout(location = 0) in vec4 vPosition;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tgl_Position = vPosition;\n"
		"}\n"
	};
	const char src_frag[] = {
		"#version 330 core\n"
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
//	glFlush();	// GL_DOUBLE doesn't work with glFlush()
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE | GLUT_DOUBLE | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow(argv[0]);
#else
	glutInitDisplayMode(GLUT_RGB | GLUT_MULTISAMPLE | GLUT_DOUBLE);
//	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE | GLUT_DOUBLE);
//	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);	// antialiasing doesn't work with single buffer!
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
	glutMainLoop();
}
