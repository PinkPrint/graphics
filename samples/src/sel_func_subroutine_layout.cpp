///////////////////////////////////////////////////////////////////////
//
// sel_func_subroutine_layout.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 22 September 2014
//
// requires OpenGL 4.3 or above
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

#define	IDX_FUNC_BASE	5
GLuint	idx_func = IDX_FUNC_BASE;


GLuint	vao;
GLuint	vbo;
GLuint	h_prog;

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


GLint	idx_shader_func[2];
GLint	loc_func;
GLint	n_func;
void init(void)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLfloat vertices[6][2] = {
		{ -0.90, -0.90 }, // Triangle 1
		{  0.85, -0.90 },
		{ -0.90,  0.85 },
		{  0.90, -0.85 }, // Triangle 2
		{  0.90,  0.90 },
		{ -0.85,  0.90 }
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	h_prog = build_program_from_files("triangles.vert", "sel_func_subroutine_layout.frag");

	glUseProgram(h_prog);

	loc_func = glGetSubroutineUniformLocation(h_prog, GL_FRAGMENT_SHADER, "func");

	idx_shader_func[0] = 7;
	idx_shader_func[1] = 10;

	printf("location of the uniform subroutine \"func\" = %d\n", loc_func);

	glGetProgramStageiv(h_prog, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &n_func);

	printf("# of active subroutine uniforms = %d\n", n_func);

	GLint	n;
	glGetProgramStageiv(h_prog, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINES, &n);
	printf("# of active subroutines = %d\n", n);

	char	name[1024];
	GLsizei	length;

	glGetActiveSubroutineName(h_prog, GL_FRAGMENT_SHADER, idx_shader_func[0], 1024, &length, name);
	printf("subroutine name for index %d = %s\n", idx_shader_func[0], name);
	glGetActiveSubroutineName(h_prog, GL_FRAGMENT_SHADER, idx_shader_func[1], 1024, &length, name);
	printf("subroutine name for index %d = %s\n", idx_shader_func[1], name);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(h_prog);



	GLuint	*indices = new GLuint[n_func];
	indices[loc_func] = idx_shader_func[idx_func-IDX_FUNC_BASE];
	printf("idx_func=%d\n", idx_func);
	printf("idx_shader_func[idx_func-IDX_FUNC_BASE]=%d\n", idx_shader_func[idx_func - IDX_FUNC_BASE]);

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, n_func, indices);
	delete [] indices;

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glFlush();
}
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case ' ':
			idx_func = (idx_func+1);
			if(idx_func == IDX_FUNC_BASE+2)	idx_func = IDX_FUNC_BASE;
			glutPostRedisplay();	// Forces to redraw the screen!
			break;
	}
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
	glutInitContextVersion(4,3);
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
	glutMainLoop();
}
