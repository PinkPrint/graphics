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
#include <gli/gli.hpp>
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
GLuint vao_quad;
GLuint buf_quad;
GLint	loc_color, loc_tex;
GLuint	tex;

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

// http://www.g-truc.net/project-0024.html
void load_texture_dds(void)
{
	gli::texture2D Texture(gli::loadStorageDDS("data/images/uos.dds"));
	assert(!Texture.empty());
	printf("texture loaded. (width=%d, height=%d)\n", Texture.dimensions().x, Texture.dimensions().y);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	glTexStorage2D(GL_TEXTURE_2D,
		GLint(Texture.levels()),
		GLenum(gli::internal_format(Texture.format())),
		GLsizei(Texture.dimensions().x),
		GLsizei(Texture.dimensions().y));
	if(gli::is_compressed(Texture.format()))
	{
		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::internal_format(Texture.format())),
				GLsizei(Texture[Level].size()),
				Texture[Level].data());
		}
	}
	else
	{
		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::external_format(Texture.format())),
				GLenum(gli::type_format(Texture.format())),
				Texture[Level].data());
		}
	}
}

void init(void)
{
	glClearColor(.5, .5, .5, 1);

	glGenVertexArrays(1, &vao_quad);
	glBindVertexArray(vao_quad);
	GLfloat	vertices[4][4] = {
		{ -0.90, -0.90, 0, 1 }, 
		{  0.90, -0.90, 1, 1 },
		{ -0.90,  0.90, 0, 0 },
		{  0.90,  0.90, 1, 0 },
	};
	glGenBuffers(1, &buf_quad);
	glBindBuffer(GL_ARRAY_BUFFER, buf_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	GLuint h_prog = build_program_from_files("quad_tex.vert", "quad_tex.frag");

	glUseProgram(h_prog);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, BUFFER_OFFSET(sizeof(GLfloat)*2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	loc_color = glGetUniformLocation(h_prog, "color");
	loc_tex = glGetUniformLocation(h_prog, "tex");

	glEnable(GL_CULL_FACE);
//	glFrontFace(GL_CW);

	load_texture_dds();
}

void display(void)
{
	GLfloat	color[4] = {1, .5, .5, 1};
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform4fv(loc_color, 1, color);
	glUniform1i(loc_tex, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBindVertexArray(vao_quad);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
