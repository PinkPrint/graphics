///////////////////////////////////////////////////////////////////////
//
// xfm_lookat.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 22 October 2014
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
GLuint	vao_cube, vao_axes;
GLuint	buf_cube, buf_idx, buf_axes;
GLuint	h_prog;
GLuint	loc_MVP;

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
	GLfloat vertices[8][6] = {
		{-1,-1,-1, 1, 1, 1},		//0
		{-1,-1, 1, 1, 1, 1},		//1
		{-1, 1,-1, 1, 1, 1},		//2
		{-1, 1, 1, 1, 1, 1},		//3
		{ 1,-1,-1, 1, 1, 1},		//4
		{ 1,-1, 1, 1, 1, 1},		//5	
		{ 1, 1,-1, 1, 1, 1},		//6
		{ 1, 1, 1, 1, 1, 1},	//7
	};
	GLubyte	indices[24] = {
		0, 1,
		0, 2,
		0, 4,
		7, 6,
		7, 5,
		7, 3,
		1, 3,
		3, 2,
		2, 6,
		6, 4,
		4, 5,
		5, 1
	};
	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	glGenBuffers(1, &buf_cube);
	glBindBuffer(GL_ARRAY_BUFFER, buf_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &buf_idx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	h_prog = build_program_from_files("triangles_xfm.vert", "triangles_xfm.frag");

	loc_MVP = glGetUniformLocation(h_prog, "MVP");

	glUseProgram(h_prog);


	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	static GLfloat	angle = 0;
	static GLfloat	angle_world = 0;
	angle += 1.0f;
	angle_world += 0.2f;
	if(angle > 360.0f)
	{
		angle -= 360.0f;
	}
	if(angle_world > 360.0f)
	{
		angle_world -= 360.0f;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4	V = glm::lookAt(glm::vec3(3.0f,3.0f,3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//	glm::mat4	P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 4.0f);
	glm::mat4	P = glm::perspective(80.0f, 1.0f, 0.5f, 10.0f);

	glBindVertexArray(vao_cube);
	glLineWidth(2.0f);
	glm::mat4	Rz_cube = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4	S_cube = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 1.0f));
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*V*Rz_cube*S_cube));
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	
	glBindVertexArray(vao_axes);
	glLineWidth(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V));
	glDrawArrays(GL_LINES, 0, 6);
	glutSwapBuffers();
}

void idle(void)
{
	glutPostRedisplay();
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
	glutIdleFunc(idle);
	glutMainLoop();
}
