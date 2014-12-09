///////////////////////////////////////////////////////////////////////
//
// quad_map.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 22 September 2014
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
#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint	vao;
GLuint	vbo;
GLint	loc;

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
	glClearColor(.5, .5, .5, 1);


	glGenBuffers(1, &vbo);

	// NOTE: you can map the buffer with as target as follows.

#if	0	// map the buffer as GL_COPY_WRITE_BUFFER target
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(GLfloat)*12, NULL, GL_STATIC_DRAW);
	void	*data = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	FILE	*pfile = fopen("quad_triangles.raw", "rb");
	fread(data, sizeof(GLfloat), 12, pfile);
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	fclose(pfile);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
#else	// map the buffer as GL_ARRAY_BUFFER target
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*12, NULL, GL_STATIC_DRAW);
	void	*data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	FILE	*pfile = fopen("quad_triangles.raw", "rb");
	fread(data, sizeof(GLfloat), 12, pfile);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	fclose(pfile);
#endif

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*2, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	GLuint h_prog = build_program_from_files("triangles_uniform.vert", "triangles_uniform.frag");
	glUseProgram(h_prog);
	loc = glGetUniformLocation(h_prog, "color");

	glEnable(GL_CULL_FACE);
}

void display(void)
{
	GLfloat	color[4] = {1, .5, .5, 1};
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform4fv(loc, 1, color);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glFlush();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef __APPLE__
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
