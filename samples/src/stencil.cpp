///////////////////////////////////////////////////////////////////////
//
// stencil.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 14 October 2014
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

GLuint	vao_points;
GLuint	vbo_points;
#define	NX	100
#define	NY	100
GLfloat	verts_points[NX][NY][2];

GLuint	vao_tri;
GLuint	vbo_tri;
GLfloat	verts_tri[3][2] = 
{
	{-0.9, 0.9},
	{ 0.0,-0.9},
	{ 0.9, 0.9},
};

GLuint	vao_mask;
GLuint	vbo_mask;
GLfloat	verts_mask[3][2] = 
{
	{-0.5,-0.5},
	{ 0.5,-0.5},
	{ 0.0, 0.5},
};

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
	GLfloat	stepx = 2.0/GLfloat(NX+1);
	GLfloat	stepy = 2.0/GLfloat(NY+1);

	for(int x=0 ; x<NX ; x++)
	{
		for(int y=0 ; y<NY ; y++)
		{
			verts_points[x][y][0] = stepx*GLfloat(x+1)-1.0;
			verts_points[x][y][1] = stepy*GLfloat(y+1)-1.0;
		}
	}
	glGenVertexArrays(1, &vao_points);
	glBindVertexArray(vao_points);
	glGenBuffers(1, &vbo_points);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_points), verts_points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &vao_tri);
	glBindVertexArray(vao_tri);
	glGenBuffers(1, &vbo_tri);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tri);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_tri), verts_tri, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);


	glGenVertexArrays(1, &vao_mask);
	glBindVertexArray(vao_mask);
	glGenBuffers(1, &vbo_mask);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mask);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_mask), verts_mask, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	GLuint h_prog = build_program_from_files("stencil.vert", "stencil.frag");

	glUseProgram(h_prog);

	glClearStencil(0x0);

	glEnable(GL_STENCIL_TEST);
}

void display(void)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glBindVertexArray(vao_mask);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glClear(GL_COLOR_BUFFER_BIT);

	glStencilFunc(GL_EQUAL, 0x0, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glBindVertexArray(vao_tri);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glBindVertexArray(vao_points);
	glDrawArrays(GL_POINTS, 0, NX*NY);

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow(argv[0]);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL);
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
