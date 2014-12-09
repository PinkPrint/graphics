///////////////////////////////////////////////////////////////////////
//
// xfm_viewport.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 22 October 2014
//
///////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <stdlib.h>
#include <cmath>
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
#define PI 3.14159265
#endif
GLuint	vao_car,vao_axes,vao_rooms;
GLuint	buf_car,buf_axes,buf_rooms;
GLuint  h_progCar,h_progAxes,h_progRooms;
GLuint	loc_MVP;
GLuint	width,height;
GLfloat	angle = 0;
GLfloat x_T,z_T;
glm::mat4	P;


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
	GLfloat car[3][8] = {//자동차
		{ 0.25, 0.00, -0.25, 1.0, 0, 1, 1, 0.3 },
		{ -0.25, 0.00, -0.25, 1.0, 0, 1, 1, 0.3 },
		{ 0.0,  0.00, 0.25, 1.0, 0, 1, 1, 0.3 },
	};

	glGenVertexArrays(1, &vao_car);
	glBindVertexArray(vao_car);
	glGenBuffers(1, &buf_car);
	glBindBuffer(GL_ARRAY_BUFFER, buf_car);
	glBufferData(GL_ARRAY_BUFFER, sizeof(car), car, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(4*sizeof(GLfloat)));
	h_progCar = build_program_from_files("car.vert", "car.frag");
	glUseProgram(h_progCar);
	
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

	h_progAxes = build_program_from_files("axes.vert", "axes.frag");
	
	loc_MVP = glGetUniformLocation(h_progAxes, "MVP");
	glUseProgram(h_progAxes);

	GLfloat room_width = 3.0f;
	GLfloat room_height = 0.8f;

	GLfloat rooms[16][8] = {//room
		//room1 = red
		{ -room_width,  0.0,    -room_width, 1.0, 1, 0, 0, 0 },
		{  room_width,  0.0,    -room_width, 1.0, 1, 0, 0, 0 },
		{  room_width,  room_height, -room_width, 1.0, 1, 0, 0, 0 },
		{ -room_width,  room_height, -room_width, 1.0, 1, 0, 0, 0 },
		//room2 = purple
		{ -room_width,  0.0,     room_width, 1.0, 1, 0, 1, 0 },
		{ -room_width,  0.0,    -room_width, 1.0, 1, 0, 1, 0 },
		{ -room_width,  room_height, -room_width, 1.0, 1, 0, 1, 0 },
		{ -room_width,  room_height,  room_width, 1.0, 1, 0, 1, 0 },
		//room3 = blue
		{ -room_width,  0.0,     room_width, 1.0, 0, 0, 1, 0 },
		{  room_width,  0.0,     room_width, 1.0, 0, 0, 1, 0 },
		{  room_width,  room_height,  room_width, 1.0, 0, 0, 1, 0 },
		{ -room_width,  room_height,  room_width, 1.0, 0, 0, 1, 0 },
		//room4 = green
		{  room_width,  0.0,     room_width, 1.0, 0, 1, 0, 0 },
		{  room_width,  0.0,    -room_width, 1.0, 0, 1, 0, 0 },
		{  room_width,  room_height, -room_width, 1.0, 0, 1, 0, 0 },
		{  room_width,  room_height,  room_width, 1.0, 1, 1, 0, 0.3 },
	};

	glGenVertexArrays(1, &vao_rooms);
	glBindVertexArray(vao_rooms);
	glGenBuffers(1, &buf_rooms);
	glBindBuffer(GL_ARRAY_BUFFER, buf_rooms);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rooms), rooms, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(4*sizeof(GLfloat)));

	h_progRooms = build_program_from_files("room.vert", "room.frag");
	glUseProgram(h_progRooms);

	P = glm::perspective(60.0f, GLfloat(room_width)/3*GLfloat(room_height), 0.1f, 100.0f);

	glEnable(GL_DEPTH_TEST);
	
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//왼쪽 화면
	glViewport(0,0,width/2,height);
	//왼쪽화면 자동차를 그리는 부분
	glBindVertexArray(vao_car);
	glLineWidth(2.0f);	
	
	glm::mat4 V_left = glm::lookAt(glm::vec3(6,6,6),glm::vec3(0,0,0),glm::vec3(0,1,0));//왼쪽화면 카메라 시점
	glm::mat4 R_car = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 T_base = glm::translate(glm::mat4(1.0f), glm::vec3(x_T,0.0f,z_T));
	
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*V_left*T_base*R_car));
	glDrawArrays(GL_TRIANGLES, 0, 3);
	//왼쪽화면 x,y,z축 그리는 부분
	glBindVertexArray(vao_axes);
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, glm::value_ptr(P*V_left));
	glDrawArrays(GL_LINES, 0, 6);
	//왼쪽하면 벽 그리는 부분
	glBindVertexArray(vao_rooms);
	glDrawArrays(GL_QUADS, 0, 16);

	//오른쪽 화면
	glViewport(width/2,0,width/2,height);
	//오른쪽화면 벽 그리는 부분
	glBindVertexArray(vao_rooms);
	
	glm::mat4 V_right = glm::lookAt(glm::vec3(x_T,0.2f,z_T),glm::vec3(x_T+5.0,0,z_T+5.0),glm::vec3(0,1,0));//오른쪽화면 카메라 시점
	glm::mat4 R_init = glm::rotate(glm::mat4(1.0f),40.0f,glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 R_rooms = glm::rotate(glm::mat4(1.0f),-angle,glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(h_progRooms, "MVP"), 1, GL_FALSE, glm::value_ptr(P*R_rooms*R_init*V_right));
	
	glDrawArrays(GL_QUADS, 0, 16);
	
	glutSwapBuffers();
}
void reshape(int w,int h){
	width = w;
	height = h;
}
void special(int key, int x, int y)
{//키보드 조작부분
	switch(key)
	{
	case GLUT_KEY_UP:
		z_T += 0.2f*cos(angle*PI/180.0f);
		x_T += 0.2f*sin(angle*PI/180.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		z_T -= 0.2f*cos(angle*PI/180.0f);
		x_T -= 0.2f*sin(angle*PI/180.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		angle+=2.0f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		angle-=2.0f;
		glutPostRedisplay();
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 512);
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
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutMainLoop();
}
