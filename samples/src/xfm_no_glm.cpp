///////////////////////////////////////////////////////////////////////
//
// xfm_no_glm.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 15 October 2014
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

#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
#define	PI	3.14159265358979
#define	DEG2RAD(x)		((x)*PI/180.0)
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
		{ -0.70, -0.70, -0.2, 1.0, 0, 1, 1, 0.3 },	// cyan, farthest
		{  0.70, -0.70, -0.2, 1.0, 0, 1, 1, 0.3 },
		{  0.00,  0.70, -0.2, 1.0, 0, 1, 1, 0.3 },

		{ -0.70, -0.70,  0.0, 1.0, 1, 0, 1, 0.3 },	// violet, middle
		{  0.70, -0.70,  0.0, 1.0, 1, 0, 1, 0.3 },
		{  0.00,  0.70,  0.0, 1.0, 1, 0, 1, 0.3 },

		{ -0.70, -0.70,  0.2, 1.0, 1, 1, 0, 0.3 },	// yellow, nearest
		{  0.70, -0.70,  0.2, 1.0, 1, 1, 0, 0.3 },
		{  0.00,  0.70,  0.2, 1.0, 1, 1, 0, 0.3 },
	};

	glGenVertexArrays(1, &vao_tris);
	glBindVertexArray(vao_tris);

	glGenBuffers(1, &buf_tris);
	glBindBuffer(GL_ARRAY_BUFFER, buf_tris);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), BUFFER_OFFSET(4*sizeof(GLfloat)));
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

	GLuint h_prog = build_program_from_files("xfm_no_glm.vert", "xfm_no_glm.frag");

	glUseProgram(h_prog);

	// rotation around the y-axis
	GLfloat	thetay = DEG2RAD(-30.0f);
	GLfloat	Ry[4][4] =
	{
		{cosf(thetay), 0, -sinf(thetay), 0},
		{0,            1, 0,             0},
		{sinf(thetay), 0, cosf(thetay),  0},
		{0,            0, 0,             1},
	};

	// rotation around the x-axis
	GLfloat	thetax = DEG2RAD(30.0f);
	GLfloat	Rx[4][4] =
	{
		{1.0f, 0,             0,            0},
		{0,    cosf(thetax),  sinf(thetax), 0},
		{0,    -sinf(thetax), cosf(thetax), 0},
		{0,    0,             0,            1.0f},
	};

	// translation
	GLfloat	T[4][4] = 
	{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f,-2.0f, 1.0f},
	};

#define	PERSPECTIVE	1
#define	FRUSTUM		2
#define	ORTHO		3

//#define	PROJECTION	PERSPECTIVE
//#define	PROJECTION	FRUSTUM
#define	PROJECTION	ORTHO


	// projection matrix
#if	(PROJECTION==PERSPECTIVE)	// perspective
	GLfloat	n = 1.0f, f = 3.0f;
	GLfloat	fovy = DEG2RAD(45.0f);
	GLfloat	aspect = 1.0f;
	GLfloat	P[4][4] =
	{
		{1.0f/(aspect*tanf(fovy/2.0f)), 0.0,                  0.0,            0.0},
		{0.0,                           1.0f/tanf(fovy/2.0f), 0.0,            0.0},
		{0.0,                           0.0,                  -(f+n)/(f-n),    -1.0f},
		{0.0,                           0.0,                  -2.0f*f*n/(f-n), 0.0},
	};

#elif (PROJECTION==FRUSTUM)	// frustum
	GLfloat	l = -1.0f, r = 1.0f, b = -1.0f, t = 1.0f, n = 1.0f, f = 3.0f;
	GLfloat	P[4][4] =
	{
		{2.0f*n/(r-l), 0,            0,               0},
		{0,            2.0f*n/(t-b), 0,               0},
		{(r+l)/(r-l),  (t+b)/(t-b),  -(f+n)/(f-n),    -1.0f},
		{0,            0,            -2.0f*f*n/(f-n), 0.0f},
	};
#elif (PROJECTION==ORTHO) // orthogonal
	GLfloat	l = -2.0f, r = 2.0f, b = -2.0f, t = 2.0f, n = 0.0f, f = 4.0f;
	GLfloat	P[4][4] =
	{
		{2.0f/(r-l),   0,            0,            0},
		{0,            2.0f/(t-b),   0,            0},
		{0,            0,            -2.0f/(f-n),  0},
		{-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1.0f},
	};
#endif

	glUniformMatrix4fv(glGetUniformLocation(h_prog, "P"), 1, GL_FALSE, (GLfloat*)P);
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "Rx"), 1, GL_FALSE, (GLfloat*)Rx);
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "Ry"), 1, GL_FALSE, (GLfloat*)Ry);
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "T"), 1, GL_FALSE, (GLfloat*)T);

	glEnable(GL_DEPTH_TEST);

}

void display(void)
{
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
