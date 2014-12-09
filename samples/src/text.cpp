//
// text rendering example
// original source: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
// modified by Minho Kim (12 Nov. 2013)
//

#pragma comment(lib, "../lib/freetype.lib")
#include <iostream>
#include <fstream>
#include <errno.h>
#ifdef	__APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/GLUT.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint h_prog;
GLuint vbo, vao;

FT_Library ft;
FT_Face face;

const char *fontfilename;


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


int init(void)
{
	if(FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "Could not init freetype library\n");
		return 0;
	}
	
	if (FT_New_Face(ft, "arial.ttf", 0, &face))
//	if (FT_New_Face(ft, "NanumMyeongjo.ttf", 0, &face))
//	if (FT_New_Face(ft, "ARIALUNI.TTF", 0, &face))
	{
		fprintf(stderr, "Could not open font %s\n", fontfilename);
		return 0;
	}

	h_prog = build_program_from_files("text.vert", "text.frag");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
	glGenBuffers(1, &vbo);

	return 1;
}
void render_text(const char *text, float x, float y, float sx, float sy)
{
	const char *p;
	FT_GlyphSlot g = face->glyph;

	GLuint tex;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(h_prog, "tex"), 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), BUFFER_OFFSET(2*sizeof(GLfloat)));

	for (p = text; *p; p++)
    {
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;
        // No "GL_ALPHA" format anymore... Use "GL_RED" instead.
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
        
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] = {
			{x2, -y2, 0, 0},
			{x2 + w, -y2, 1, 0},
			{x2, -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDeleteTextures(1, &tex);
}
void display()
{
	float sx = 2.0 / glutGet(GLUT_WINDOW_WIDTH);
	float sy = 2.0 / glutGet(GLUT_WINDOW_HEIGHT);


	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(h_prog);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat black[4] = { 0, 0, 0, 1 };
	GLfloat red[4] = { 1, 0, 0, 1 };
	GLfloat transparent_green[4] = { 0, 1, 0, 0.5 };

	FT_Set_Pixel_Sizes(face, 0, 48);
	glUniform4fv(glGetUniformLocation(h_prog, "color"), 1, black);

	render_text("OpenGL Rocks!", -1 + 8 * sx, 1 - 50 * sy, sx, sy);
	render_text("The Misaligned Fox Jumps Over The Lazy Dog", -1 + 8.5 * sx, 1 - 100.5 * sy, sx, sy);

	render_text("The Small Texture Scaled Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 175 * sy, sx * 0.5, sy * 0.5);
	FT_Set_Pixel_Sizes(face, 0, 24);
	render_text("The Small Font Sized Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 200 * sy, sx, sy);
	FT_Set_Pixel_Sizes(face, 0, 48);
	render_text("The Tiny Texture Scaled Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 235 * sy, sx * 0.25, sy * 0.25);
	FT_Set_Pixel_Sizes(face, 0, 12);
	render_text("The Tiny Font Sized Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 250 * sy, sx, sy);
	FT_Set_Pixel_Sizes(face, 0, 48);

	render_text("The Solid Black Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 430 * sy, sx, sy);

	glUniform4fv(glGetUniformLocation(h_prog, "color"), 1, red);
	render_text("The Solid Red Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 330 * sy, sx, sy);
	render_text("The Solid Red Fox Jumps Over The Lazy Dog", -1 + 28 * sx, 1 - 450 * sy, sx, sy);

	glUniform4fv(glGetUniformLocation(h_prog, "color"), 1, transparent_green);
	render_text("The Transparent Green Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 380 * sy, sx, sy);
	render_text("The Transparent Green Fox Jumps Over The Lazy Dog", -1 + 18 * sx, 1 - 440 * sy, sx, sy);
	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
#ifdef	__APPLE__
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
	return 0;
}
