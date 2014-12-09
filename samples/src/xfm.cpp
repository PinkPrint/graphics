///////////////////////////////////////////////////////////////////////
//
// xfm_glm.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 15 July 2013
//
///////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/GLUT.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint	vao_tris, vao_axes;
GLuint	buf_tris, buf_axes;

bool check_compile_status(GLuint handle)
{
	GLint	status;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint	len;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);
		GLchar  *buf_log = new GLchar [len+1];
		glGetShaderInfoLog(handle, len, NULL, buf_log);
		std::cerr << "Compilation failed: " << buf_log << std::endl;
		delete[] buf_log;
	}
	return (status==GL_TRUE);
}

GLuint compile_shader(GLenum type, const char* source)
{
	GLuint	handle = glCreateShader(type);
	if(!handle)
	{
		std::cerr << "Shader object creation failed." << std::endl;
		return handle;
	}
	glShaderSource( handle, 1, &source, NULL );
	glCompileShader( handle );
	if(!check_compile_status(handle))
	{
		std::cerr << "Shader compilation failed." << std::endl;
		return 0;
	}
	return handle;
}

bool check_link_status(GLuint handle)
{
	GLint	status;
	glGetProgramiv(handle, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint	len;
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);
		GLchar	*buf_log = new GLchar[len+1];
		glGetProgramInfoLog(handle, len, NULL, buf_log);
		std::cerr << "Linking failed: " << buf_log << std::endl;
		delete[] buf_log;
	}
	return (status==GL_TRUE);
}

GLuint build_program(const char* src_vert, const char* src_frag)
{
	GLuint	h_vert = compile_shader(GL_VERTEX_SHADER, src_vert);
	if(!h_vert)
	{
		std::cerr << "Error while compiling the vertex shader" << std::endl;
		return 0;
	}
	GLuint	h_frag = compile_shader(GL_FRAGMENT_SHADER, src_frag);
	if(!h_frag)
	{
		std::cerr << "Error wihle compiling the fragment shader" << std::endl;
		return 0;
	}

	GLuint h_prog = glCreateProgram();
	if(!h_prog)
	{
		std::cerr << "Program object creation failed." << std::endl;
		return h_prog;
	}
	glAttachShader( h_prog, h_vert );
	glAttachShader( h_prog, h_frag );
	glLinkProgram( h_prog );
	if(!check_link_status(h_prog))	return 0;

	return h_prog;
}

// original source:
// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
//
std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

GLuint build_program_from_files(const char* file_vert, const char* file_frag)
{
	std::string src_vert = get_file_contents(file_vert);
	std::string src_frag = get_file_contents(file_frag);
	return build_program(src_vert.c_str(), src_frag.c_str());
}

void init(void)
{
	GLfloat vertices[9][8] = {
		/*
		{ -0.90, -0.90, -0.1, 1.0, 0, 1, 1, 0.3 },
		{  0.50, -0.90, -0.1, 1.0, 0, 1, 1, 0.3 },
		{ -0.20,  0.50, -0.1, 1.0, 0, 1, 1, 0.3 },

		{ -0.70, -0.70,  0.0, 1.0, 1, 0, 1, 0.3 },
		{  0.70, -0.70,  0.0, 1.0, 1, 0, 1, 0.3 },
		{  0.00,  0.70,  0.0, 1.0, 1, 0, 1, 0.3 },

		{ -0.50, -0.50,  0.1, 1.0, 1, 1, 0, 0.3 },
		{  0.90, -0.50,  0.1, 1.0, 1, 1, 0, 0.3 },
		{  0.20,  0.90,  0.1, 1.0, 1, 1, 0, 0.3 },
		*/
		{ -0.70, -0.70, -0.2, 1.0, 0, 1, 1, 0.3 },
		{  0.70, -0.70, -0.2, 1.0, 0, 1, 1, 0.3 },
		{  0.00,  0.70, -0.2, 1.0, 0, 1, 1, 0.3 },

		{ -0.70, -0.70,  0.0, 1.0, 1, 0, 1, 0.3 },
		{  0.70, -0.70,  0.0, 1.0, 1, 0, 1, 0.3 },
		{  0.00,  0.70,  0.0, 1.0, 1, 0, 1, 0.3 },

		{ -0.70, -0.70,  0.2, 1.0, 1, 1, 0, 0.3 },
		{  0.70, -0.70,  0.2, 1.0, 1, 1, 0, 0.3 },
		{  0.00,  0.70,  0.2, 1.0, 1, 1, 0, 0.3 },
	};
	glGenVertexArrays(1, &vao_tris);
	glBindVertexArray(vao_tris);
	glGenBuffers(1, &buf_tris);
	glBindBuffer(GL_ARRAY_BUFFER, buf_tris);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(4*sizeof(GLfloat)));

	GLfloat	axes[6][6] = {
		{ 0, 0, 0, 1, 0, 0},
		{ 2, 0, 0, 1, 0, 0},
		{ 0, 0, 0, 0, 1, 0},
		{ 0, 2, 0, 0, 1, 0},
		{ 0, 0, 0, 0, 0, 1},
		{ 0, 0, 2, 0, 0, 1},
	};
	glGenVertexArrays(1, &vao_axes);
	glBindVertexArray(vao_axes);
	glGenBuffers(1, &buf_axes);
	glBindBuffer(GL_ARRAY_BUFFER, buf_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(3*sizeof(GLfloat)));

	GLuint h_prog = build_program_from_files("triangles_xfm.vert", "triangles_xfm.frag");


//	glBindVertexArray(vao_tris);
//	glBindBuffer(GL_ARRAY_BUFFER, buf_tris);

	glUseProgram(h_prog);

	glm::mat4	Ry = glm::rotate(glm::mat4(1.0f), -30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4	Rx = glm::rotate(glm::mat4(1.0f), 30.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4	T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
	glm::mat4	P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 4.0f);
//	glm::mat4	P = glm::perspective(45.0f, 1.0f, 1.0f, 3.0f);

	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*T*Rx*Ry));
//	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(V));

	glEnable(GL_DEPTH_TEST);

}

void display(void)
{
//	glViewport(128, 128, 128, 128);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vao_tris);
	glDrawArrays(GL_TRIANGLES, 0, 9);
	glBindVertexArray(vao_axes);
	glDrawArrays(GL_LINES, 0, 6);
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow(argv[0]);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(4,1);
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
