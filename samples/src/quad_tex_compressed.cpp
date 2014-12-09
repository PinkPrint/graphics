///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 15 July 2013
//
///////////////////////////////////////////////////////////////////////
#pragma comment(lib, "../lib/x32/DevIL.lib")
#pragma comment(lib, "../lib/x32/ILU.lib")
#pragma comment(lib, "../lib/x32/ILUT.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <stdlib.h>
#include <IL/il.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint vao_quad;
GLuint buf_quad;
GLint	loc_color, loc_tex;
GLuint	tex, tex_unit = 3;

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

void load_texture(void)
{
	ilInit();
	ILuint	img;
	ilGenImages(1,&img);
	ilBindImage(img);
	ilLoadImage("data/images/uos_compressed.dds");
	printf("image loaded. (width=%d, height=%d, format=%d, type=%d, compress mode=%d)\n",
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		ilGetInteger(IL_IMAGE_TYPE),
		ilGetInteger(IL_COMPRESS_MODE));

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#define	METHOD	2
#if	(METHOD==1)
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
	/*
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
		*/
	/*
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
		*/
#elif (METHOD==2)
	/*
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 
		0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
		*/
	// GL_COMPRESSED_RGB8_ETC2
	// GL_COMPRESSED_SRGB8_ETC2
	// GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
	// GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
	// GL_COMPRESSED_RGBA8_ETC2_EAC
	// GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
	// GL_COMPRESSED_R11_EAC
	// GL_COMPRESSED_SIGNED_R11_EAC
	// GL_COMPRESSED_RG11_EAC
	// GL_COMPRESSED_SIGNED_RG11_EAC
	/*
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 
		0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
		*/
#elif (METHOD==3)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 
		0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), NULL);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), ilGetInteger(IL_IMAGE_FORMAT),
		ilGetInteger(IL_IMAGE_TYPE), ilGetData());
#endif
}

void init(void)
{
	glClearColor(.5, .5, .5, 1);

	glGenVertexArrays(1, &vao_quad);
	glBindVertexArray(vao_quad);
	GLfloat	vertices[4][4] = {
		{ -0.90, -0.90, 0, 1 }, 
		{ 0.90, -0.90, 1, 1 },
		{ -0.90, 0.90, 0, 0 },
		{ 0.90, 0.90, 1, 0 },
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

	load_texture();
}

void display(void)
{
	GLfloat	color[4] = {1, 0, 0, 1};
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform4fv(loc_color, 1, color);
	glUniform1i(loc_tex, tex_unit);

	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBindVertexArray(vao_quad);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glFlush();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(4,3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
	glutCreateWindow(argv[0]);
	if (glewInit()) {
		std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
		exit(EXIT_FAILURE);
	}
	init();
	glutDisplayFunc(display);
	glutMainLoop();
}
