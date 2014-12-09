///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
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
GLuint	vao_quad, vao_axes;
GLuint	buf_quad, buf_axes;
GLuint	loc_color, loc_MVP;
GLfloat	angle_R, angle_G, angle_B;
glm::mat4	P, T, T_base;
GLfloat	x_R, y_R;

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
	GLfloat vertices[4][3] = {
		{-.5,-.5, 0.1},
		{ .5,-.5, 0.1},
		{-.5, .5, 0.1},
		{ .5, .5, 0.1}
	};
	glGenVertexArrays(1, &vao_quad);
	glBindVertexArray(vao_quad);
	glGenBuffers(1, &buf_quad);
	glBindBuffer(GL_ARRAY_BUFFER, buf_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), BUFFER_OFFSET(0));

	GLfloat	axes[4][3] = {
		{ 0, 0, 0},
		{ 2, 0, 0},
		{ 0, 0, 0},
		{ 0, 2, 0},
	};
	glGenVertexArrays(1, &vao_axes);
	glBindVertexArray(vao_axes);
	glGenBuffers(1, &buf_axes);
	glBindBuffer(GL_ARRAY_BUFFER, buf_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), BUFFER_OFFSET(0));

	GLuint h_prog = build_program_from_files("xfm_hierarchy.vert", "xfm_hierarchy.frag");

	glUseProgram(h_prog);

	loc_color = glGetUniformLocation(h_prog, "color");
	loc_MVP = glGetUniformLocation(h_prog, "MVP");

	P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 4.0f);
	T_base = glm::translate(glm::mat4(1.0f), glm::vec3(x_R, y_R, 0.0f));
	T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));

	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vao_quad);

#define	WIDTH_RED	0.3f
#define	LENGTH_RED	1.0f
	glm::mat4	Rr = glm::rotate(glm::mat4(1.0f), angle_R, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4	Sr = glm::scale(glm::mat4(1.0f), glm::vec3(LENGTH_RED, WIDTH_RED, 1.0f));
	glm::mat4	Tr_low = glm::translate(glm::mat4(1.0f), glm::vec3((LENGTH_RED - WIDTH_RED)/2.0f, 0.0f, 0.1f));
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*T*T_base*Rr*Tr_low*Sr));
	glUniform4f(loc_color, 1, 0, 0, 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#define	WIDTH_GREEN	0.2f
#define	LENGTH_GREEN	0.8f
	glm::mat4	Tr_high = glm::translate(glm::mat4(1.0f), glm::vec3(LENGTH_RED - WIDTH_RED, 0.0f, 0.0f));
	glm::mat4	Tg_low = glm::translate(glm::mat4(1.0f), glm::vec3((LENGTH_GREEN - WIDTH_GREEN)/2.0f, 0.0f, 0.2f));
	glm::mat4	Sg = glm::scale(glm::mat4(1.0f), glm::vec3(LENGTH_GREEN, WIDTH_GREEN, 1.0f));
	glm::mat4	Rg = glm::rotate(glm::mat4(1.0f), angle_G, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*T*T_base*Rr*Tr_high*Rg*Tg_low*Sg));
	glUniform4f(loc_color, 0, 1, 0, 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#define	WIDTH_BLUE	(WIDTH_GREEN/2.0f)
#define	LENGTH_BLUE	0.3f
	glm::mat4	Tg_high1 = glm::translate(glm::mat4(1.0f), glm::vec3(LENGTH_GREEN - WIDTH_GREEN + WIDTH_BLUE/2.0f, WIDTH_BLUE/2.0f, 0.0f));
	glm::mat4	Tg_high2 = glm::translate(glm::mat4(1.0f), glm::vec3(LENGTH_GREEN - WIDTH_GREEN + WIDTH_BLUE/2.0f, -WIDTH_BLUE/2.0f, 0.0f));
	glm::mat4	Tb = glm::translate(glm::mat4(1.0f), glm::vec3((LENGTH_BLUE - WIDTH_BLUE)/2.0f, 0.0f, 0.3f));
	glm::mat4	Sb = glm::scale(glm::mat4(1.0f), glm::vec3(LENGTH_BLUE, WIDTH_BLUE, 1.0f));
	glm::mat4	Rb1 = glm::rotate(glm::mat4(1.0f), angle_B, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4	Rb2 = glm::rotate(glm::mat4(1.0f), -angle_B, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniform4f(loc_color, 0, 0, 1, 1);
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*T*T_base*Rr*Tr_high*Rg*Tg_high1*Rb1*Tb*Sb));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*T*T_base*Rr*Tr_high*Rg*Tg_high2*Rb2*Tb*Sb));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);



	glBindVertexArray(vao_axes);
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*T));
	glUniform4f(loc_color, 1, 0, 0, 1);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform4f(loc_color, 0, 1, 0, 1);
	glDrawArrays(GL_LINES, 2, 2);
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'R':
		angle_R += 5.0f;
		glutPostRedisplay();
		break;
	case 'r':
		angle_R -= 5.0f;
		glutPostRedisplay();
		break;
	case 'G':
		angle_G += 5.0f;
		glutPostRedisplay();
		break;
	case 'g':
		angle_G -= 5.0f;
		glutPostRedisplay();
		break;
	case 'B':
		angle_B += 5.0f;
		glutPostRedisplay();
		break;
	case 'b':
		angle_B -= 5.0f;
		glutPostRedisplay();
		break;
	}
}

void special(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:
		y_R += 0.05f;
		T_base = glm::translate(glm::mat4(1.0f), glm::vec3(x_R, y_R, 0.0f));
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		y_R -= 0.05f;
		T_base = glm::translate(glm::mat4(1.0f), glm::vec3(x_R, y_R, 0.0f));
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		x_R -= 0.05f;
		T_base = glm::translate(glm::mat4(1.0f), glm::vec3(x_R, y_R, 0.0f));
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		x_R += 0.05f;
		T_base = glm::translate(glm::mat4(1.0f), glm::vec3(x_R, y_R, 0.0f));
		glutPostRedisplay();
		break;
	}
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
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMainLoop();
}
