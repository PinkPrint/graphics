///////////////////////////////////////////////////////////////////////
//
// one_vbo_for_multiple_vaos.cpp
//
// created by Minho Kim (minhokim@uos.ac.kr) on 15 September 2014
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
GLuint	vao[2];
GLuint	vbo;
GLuint	h_prog[2];

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

	const char	src_vert_1[] = {
		"#version 330 core\n"
		"layout(location = 0) in vec4 vPosition;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tgl_Position = vPosition - vec4(1,0,0,0);\n"
		"}\n"
	};
	const char src_frag_1[] = {
		"#version 330 core\n"
		"out vec4 fColor;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tfColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
		"}\n"
	};

	const char	src_vert_2[] = {
		"#version 330 core\n"
		"layout(location = 0) in vec4 vPosition;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tgl_Position = vPosition + vec4(1,0,0,0);\n"
		"}\n"
	};
	const char src_frag_2[] = {
		"#version 330 core\n"
		"out vec4 fColor;\n"
		"void\n"
		"main()\n"
		"{\n"
		"\tfColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n"
	};

	const char	*src;
	GLuint	h_vert, h_frag;
	
	// program #0
	h_vert = glCreateShader(GL_VERTEX_SHADER);
	src = src_vert_1;
	glShaderSource( h_vert, 1, &src, NULL );
	glCompileShader( h_vert );

	h_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = src_frag_1;
	glShaderSource( h_frag, 1, &src, NULL );
	glCompileShader( h_frag );

	h_prog[0] = glCreateProgram();
	glAttachShader( h_prog[0], h_vert );
	glAttachShader( h_prog[0], h_frag );
	glLinkProgram( h_prog[0] );

	glDeleteShader(h_vert);
	glDeleteShader(h_frag);

	// program #1
	h_vert = glCreateShader(GL_VERTEX_SHADER);
	src = src_vert_2;
	glShaderSource( h_vert, 1, &src, NULL );
	glCompileShader( h_vert );

	h_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = src_frag_2;
	glShaderSource( h_frag, 1, &src, NULL );
	glCompileShader( h_frag );

	h_prog[1] = glCreateProgram();
	glAttachShader( h_prog[1], h_vert );
	glAttachShader( h_prog[1], h_frag );
	glLinkProgram( h_prog[1] );

	glDeleteShader(h_vert);
	glDeleteShader(h_frag);


	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(2, vao);

	// VAO #0
	glBindVertexArray(vao[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);


	// VAO #1
	glBindVertexArray(vao[1]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	// VAO #0
	glUseProgram(h_prog[0]);
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// VAO #1
	glUseProgram(h_prog[1]);
	glBindVertexArray(vao[1]);
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
	glutMainLoop();
}
