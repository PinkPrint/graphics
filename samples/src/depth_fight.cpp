///////////////////////////////////////////////////////////////////////
//
// depth_fight.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 15 October 2014
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
GLuint	h_prog;
GLuint	loc;

void init(void)
{
	/*
	GLfloat vertices[][4] = {
		{ -0.7, -0.7,  0.8, 1.0 }, 
		{  0.7, -0.7,  0.8, 1.0 },
		{ -0.7,  0.7, -0.8, 1.0 },
		{  0.7,  0.7, -0.8, 1.0 },
	};
	*/
	GLfloat vertices[][4+4] = {

		
		{  0.0,  0.7, -0.8, 1.0, 0.0, 0.0, 1.0, 1.0},
		{ -0.7, -0.7,  0.8, 1.0, 0.0, 0.0, 1.0, 1.0},
		{  0.7, -0.7,  0.8, 1.0, 0.0, 0.0, 1.0, 1.0},

		{  0.0, -0.7,  0.8, 1.0, 0.0, 1.0, 0.0, 1.0},
		{  0.7,  0.7, -0.8, 1.0, 0.0, 1.0, 0.0, 1.0},
		{ -0.7,  0.7, -0.8, 1.0, 0.0, 1.0, 0.0, 1.0},
		/*

		{  0.0,  0.7,  0.0, 1.0, 0.0, 0.0, 1.0, 1.0},
		{ -0.7, -0.7,  0.0, 1.0, 0.0, 0.0, 1.0, 1.0},
		{  0.7, -0.7,  0.0, 1.0, 0.0, 0.0, 1.0, 1.0},

		{  0.0, -0.7,  0.0, 1.0, 0.0, 1.0, 0.0, 1.0},
		{  0.7,  0.7,  0.0, 1.0, 0.0, 1.0, 0.0, 1.0},
		{ -0.7,  0.7,  0.0, 1.0, 0.0, 1.0, 0.0, 1.0},
		*/
	};

	const char	src_vert[] = {
		"#version 330 core	\n"
		"layout(location = 1) in vec4 vPosition;	\n"
		"layout(location = 2) in vec4 vColor;	\n"
		"uniform mat4	P;	\n"
		"uniform mat4	MV;	\n"
		"out vec4	color;	\n"
		"void main()	\n"
		"{	\n"
		"\t	gl_Position = P*MV*vPosition;	\n"
//		"\t	gl_Position = vPosition;	\n"
		"\t	color = vColor;	\n"
		"}	\n"
	};
	const char src_frag[] = {
		"#version 330 core\n"
		"out vec4 fColor;\n"
		"in vec4 color;\n"
		"void main()\n"
		"{\n"
		"\tfColor = color;\n"
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

	h_prog = glCreateProgram();
	glAttachShader( h_prog, h_vert );
	glAttachShader( h_prog, h_frag );
	glLinkProgram( h_prog );

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, BUFFER_OFFSET(sizeof(GLfloat)*4));
	glEnableVertexAttribArray(2);

	glUseProgram(h_prog);

//	loc = glGetUniformLocation(h_prog, "color");

	GLfloat	n = 1.0, f = 3.0;
	GLfloat	l = -1.0, r = 1.0, b = -1.0, t = 1.0;
	GLfloat	P[4][4] = 
	{
		{2.0f*n/(r-l),   0.0,          0.0,          0.0},
		{0.0,          2.0f*n/(t-b),   0.0,          0.0},
		{(r+l)/(r-l),  (t+b)/(t-b), -(f+n)/(f-n), -1.0},
		{0.0,          0.0,          -2.0f*f*n/(f-n),  0.0},
	};
	GLfloat	MV[4][4] = 
	{
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, -2.0, 1.0},
	};

	glUniformMatrix4fv(glGetUniformLocation(h_prog, "P"), 1, GL_FALSE, (GLfloat*)&P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, (GLfloat*)&MV[0][0]);

	glClearColor(0.5, 0.5, 0.5, 1.0);

	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(h_prog);
	glBindVertexArray(vao);


	/*
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUniform4f(loc, 1.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform4f(loc, 1.0, 1.0, 1.0, 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	*/
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef  __APPLE__
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow(argv[0]);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
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

