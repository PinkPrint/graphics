///////////////////////////////////////////////////////////////////////
//
// viewer.cpp
//
// by Minho Kim (minhokim@uos.ac.kr) on 22 October 2014
//
///////////////////////////////////////////////////////////////////////
#pragma comment(lib, "../lib/assimp.lib")

#pragma comment(lib, "../lib/x32/DevIL.lib")
#pragma comment(lib, "../lib/x32/ILU.lib")
#pragma comment(lib, "../lib/x32/ILUT.lib")
#pragma comment(lib, "../lib/glew32.lib")
#pragma comment(lib, "../lib/freeglut.lib")

#include "stdinc.h"
#include "ExtractIconManager.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <IL/il.h>
#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
enum GUI_STATE {STATE_NONE=0, STATE_ROTATING_OBJ, STATE_MOVING_OBJ, STATE_ROTATING_WORLD, STATE_MOVING_WORLD};
GLuint	vao_axes;
glm::mat4	P;
GLuint	h_prog, h_prog_pick, h_prog_axes;
GLuint	selected = 0;
#define	FOVY_MIN	5.0f
#define	FOVY_MAX	179.0f

#define	MIN(x,y)	((x)<(y)?(x):(y))
#define	MAX(x,y)	((x)>(y)?(x):(y))
struct {
	bool perspective;
	GLfloat	fovy;
} camera = {
	true,
	45.0f,
};

struct {
	GLuint state;
	glm::ivec2	offset;
	glm::ivec2	position_mouse;
	glm::mat4	T;
	glm::mat4	R;
} gui = {
	STATE_NONE,
	glm::ivec2(0,0),
	glm::ivec2(0,0),
	glm::mat4(1.0f),
	glm::mat4(1.0f),
};

#define	WIN_WIDTH	512
#define	WIN_HEIGHT	512

class MeshGL
{
	GLuint	vao;
	GLuint	vbo_positions;
	GLuint	vbo_texcoords;
	GLuint	vbo_normals;
	GLuint	vbo_indices;
	GLuint	nTriangles;
	GLuint	nVertices;
	
	struct {
		aiColor4D	ambient;
		aiColor4D	diffuse;
		aiColor4D	specular;
		aiColor4D	emission;
		GLfloat		shininess;
		GLfloat		strength;
	} material;
	
public:
	MeshGL(aiScene* p_scene, aiMesh* p_mesh);

	void render(void)
	{
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, nTriangles*3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}
};

MeshGL::MeshGL(aiScene* p_scene, aiMesh* p_mesh)
{
	vao = vbo_positions = vbo_texcoords = vbo_normals = 0;

	aiMaterial	*p_mtl = p_scene->mMaterials[p_mesh->mMaterialIndex];

	aiGetMaterialColor(p_mtl, AI_MATKEY_COLOR_DIFFUSE, &material.diffuse);
	aiGetMaterialColor(p_mtl, AI_MATKEY_COLOR_SPECULAR, &material.specular);
	aiGetMaterialColor(p_mtl, AI_MATKEY_COLOR_AMBIENT, &material.ambient);
	aiGetMaterialColor(p_mtl, AI_MATKEY_COLOR_EMISSIVE, &material.emission);

	unsigned int max;
	aiGetMaterialFloatArray(p_mtl, AI_MATKEY_SHININESS, &material.shininess, &max);
    aiGetMaterialFloatArray(p_mtl, AI_MATKEY_SHININESS_STRENGTH, &material.strength, &max);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if(p_mesh->HasPositions())
	{
		nVertices = p_mesh->mNumVertices;
		glGenBuffers(1, &vbo_positions);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*p_mesh->mNumVertices, &p_mesh->mVertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), BUFFER_OFFSET(0));
	}
	if(p_mesh->HasNormals())
	{
		glGenBuffers(1, &vbo_normals);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*p_mesh->mNumVertices, &p_mesh->mNormals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), BUFFER_OFFSET(0));
	}
	if(p_mesh->HasFaces())
	{
		nTriangles = p_mesh->mNumFaces;
		std::vector<GLuint>	indices;
		for(GLuint i=0 ; i<p_mesh->mNumFaces ; i++)
		{
			for(GLuint j=0 ; j<p_mesh->mFaces[i].mNumIndices ; j++)
			{
				indices.push_back(p_mesh->mFaces[i].mIndices[j]);
			}
		}
		GLuint	n = indices.size();
		glGenBuffers(1, &vbo_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*3*nTriangles, indices.data(), GL_STATIC_DRAW);
	}
}

class NodeGL
{
	std::vector<NodeGL*>	children;
	NodeGL*					parent;
	aiMatrix4x4				transformation;
	std::vector<MeshGL*>		meshes;
public:
	NodeGL(aiScene* p_scene, NodeGL* _parent, aiNode* p_node)
	{
		parent = _parent;
		transformation = p_node->mTransformation;
		MeshGL*	node = NULL;
		for(GLuint i=0 ; i<p_node->mNumMeshes ; i++)
		{
			meshes.push_back(new MeshGL(p_scene, p_scene->mMeshes[p_node->mMeshes[i]]));
		}
		for(GLuint i=0 ; i<p_node->mNumChildren ; i++)
		{
			children.push_back(new NodeGL(p_scene, this, p_node->mChildren[i]));
		}
		
	};
	~NodeGL()
	{
		if(parent)	parent = NULL;
		for(GLuint i=0 ; i<children.size() ; i++)
		{
			delete children[i];
		}
		children.empty();
		for(GLuint i=0 ; i<meshes.size() ; i++)
		{
			delete meshes[i];
		}
		meshes.empty();
	}
	void render(void)
	{
		for(GLuint i=0 ; i<meshes.size() ; i++)
		{
			meshes[i]->render();
		}
		for(GLuint i=0 ; i<children.size() ; i++)
		{
			children[i]->render();
		}
	}
};
void render_axes(glm::mat4 MVP);

class SceneGL
{
	NodeGL	*root;
public:
	GLuint	id;
	glm::mat4	T;
	glm::mat4	R;
	SceneGL() {root = NULL;}
	~SceneGL() {if(root)	delete root;}
	void	build_from_aiScene(struct aiScene* p_scene);
	void	render(bool picking);
};

void SceneGL::build_from_aiScene(struct aiScene*p_scene)
{
	root = new NodeGL(p_scene, NULL, p_scene->mRootNode);
}

void SceneGL::render(bool picking)
{
	root->render();
	if(!picking)
	{
		render_axes(P*gui.T*gui.R*T*R);
		glUseProgram(h_prog);
	}
}


#define	NUM_OBJS	4
SceneGL	scene[NUM_OBJS];


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
glm::vec3	position[NUM_OBJS] =
{
#define	OFFSET	1.5f
	glm::vec3(-OFFSET, -OFFSET, 0.0f),
	glm::vec3( OFFSET, -OFFSET, 0.0f),
	glm::vec3( OFFSET,  OFFSET, 0.0f),
	glm::vec3(-OFFSET,  OFFSET, 0.0f),
};

void load_scene(void)
{
	const char	*filename[NUM_OBJS] =
	{
		"data/models/ModernDeskOBJ.obj",
		"data1/models/test/book.obj",
		"data/models/torus.obj",
		"data/models/room2.obj",
	};
	for(GLuint i=0 ; i<NUM_OBJS ; i++)
	{
		scene[i].id = i+1;
		scene[i].T = glm::translate(glm::mat4(1.0f), glm::vec3(position[i][0], position[i][1], position[i][2]));
		scene[i].R = glm::mat4(1.0f);
		scene[i].build_from_aiScene((aiScene*)aiImportFile(filename[i],aiProcessPreset_TargetRealtime_MaxQuality));
	}
}

void init_axes(void)
{
	GLfloat	vertices[6][6] =
	{
		{0,0,0,1,0,0},
		{2,0,0,1,0,0},
		{0,0,0,0,1,0},
		{0,2,0,0,1,0},
		{0,0,0,0,0,1},
		{0,0,2,0,0,1},
	};
	glGenVertexArrays(1, &vao_axes);
	glBindVertexArray(vao_axes);

	GLuint	vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), BUFFER_OFFSET(3*sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	h_prog_axes = build_program_from_files("axes.vert", "axes.frag");
	std::cout << "Buliding axes.vert & aves.frag succeeded." << std::endl;
	
}

void render_axes(glm::mat4 MVP)
{
	glUseProgram(h_prog_axes);
	glUniformMatrix4fv(glGetUniformLocation(h_prog_axes, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glBindVertexArray(vao_axes);
	glDrawArrays(GL_LINES, 0, 6);
}

void load_texture(void)
{
	static const char *filenames[6] = {
		"data/images/RomeChurch/posx.jpg",
		"data/images/RomeChurch/negx.jpg",
		"data/images/RomeChurch/posy.jpg",
		"data/images/RomeChurch/negy.jpg",
		"data/images/RomeChurch/posz.jpg",
		"data/images/RomeChurch/negz.jpg"
	};
	GLenum	faces[6] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};


	ilInit();
	ILuint	img[6];
	
	ilGenImages(6, img);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &cube_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);

	for (int i = 0; i<6; i++)
	{
		ilBindImage(img[i]);
		ilLoadImage(filenames[i]);
		glTexImage2D(faces[i], 0, GL_RGB8, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
			ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
	}
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindSampler(cube_tex_unit, sampler);
}


void init_desktop(void)
{

	//glClearColor(.5, .5, .5, 1);

#define	VERT	5.0f
	static const GLfloat positions[8][4] =
	{
		{ -VERT, -VERT, -VERT, VERT },
		{ -VERT, -VERT, VERT, VERT },
		{ -VERT, VERT, -VERT, VERT },
		{ -VERT, VERT, VERT, VERT },
		{ VERT, -VERT, -VERT, VERT },
		{ VERT, -VERT, VERT, VERT },
		{ VERT, VERT, -VERT, VERT },
		{ VERT, VERT, VERT, VERT },
	};

	static const GLushort indices[] =
	{
		0, 1, 2, 3, 6, 7, 4, 5,         // First strip
		0xFFFF,                         // <<-- This is the restart index
		2, 6, 0, 4, 1, 5, 3, 7          // Second strip
	};
	GLuint	buf_attribs, buf_indices;

	glGenVertexArrays(1, &vao_room);
	glBindVertexArray(vao_room);

	glGenBuffers(1, &buf_attribs);
	glBindBuffer(GL_ARRAY_BUFFER, buf_attribs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glGenBuffers(1, &buf_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	h_prog_room = build_program_from_files("tex_cubemap_room.vert", "tex_cubemap_room.frag");

	glUseProgram(h_prog_room);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 4, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glUniform1i(glGetUniformLocation(h_prog_room, "tex"), cube_tex);

	P = glm::perspective(45.0f, 1.0f, 0.1f, 10.0f);
	V = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -6.0f));
	glm::mat4	M = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(h_prog_room, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));


	h_prog_cubemap = build_program_from_files("tex_cubemap.vert", "tex_cubemap.frag");
	glUseProgram(h_prog_cubemap);
	glUniform1i(glGetUniformLocation(h_prog_cubemap, "tex"), cube_tex);

	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	load_texture();
}


void init(void)
{
	h_prog = build_program_from_files("shading_Blinn_Phong.vert", "shading_Blinn_Phong.frag");
	std::cout << "Buliding shading_Blinn_Phong.vert & shading_Blinn_Phong.frag succeeded." << std::endl;

	h_prog_pick = build_program_from_files("picking_color.vert", "picking_color.frag");
	std::cout << "Buliding picking_color.vert & picking_color.frag succeeded." << std::endl;


//	P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -50.0f, 50.0f);
	P = glm::perspective(camera.fovy, 1.0f, 0.1f, 20.0f);
	gui.T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));

	glEnable(GL_DEPTH_TEST);

	load_scene();

	init_axes();
}





void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(h_prog);

	glUniform4f(glGetUniformLocation(h_prog, "light.position"), 2.0f, 2.0f, 2.0f, 0.0f);
	glUniform3f(glGetUniformLocation(h_prog, "light.ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(h_prog, "light.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(h_prog, "light.specular"), 1.0f, 1.0f, 1.0f);


	glm::mat4	M;
	glm::mat4	V = gui.T*gui.R;
	for(int i=0 ; i<NUM_OBJS ; i++)
	{
		M = scene[i].T*scene[i].R;
		glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));
		glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, glm::value_ptr(V*M));
		glUniformMatrix4fv(glGetUniformLocation(h_prog, "V"), 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix3fv(glGetUniformLocation(h_prog, "matNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*M)));
		
		
			glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), 0.4*128.0);
		
		/*

		if(scene[i].id == selected)
		{
			// GOLD	http://devernay.free.fr/cours/opengl/materials.html
			glUniform3f(glGetUniformLocation(h_prog, "material.ambient"), 0.24725, 0.1995, 0.0745);
			glUniform3f(glGetUniformLocation(h_prog, "material.diffuse"), 0.75164, 0.60648, 0.22648);
			glUniform3f(glGetUniformLocation(h_prog, "material.specular"), 0.628281, 0.555802, 0.366065);
			glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), 0.4*128.0);
		}
		else
		{
			// SILVER	http://devernay.free.fr/cours/opengl/materials.html
			glUniform3f(glGetUniformLocation(h_prog, "material.ambient"), 0.19225, 0.19225, 0.19225);
			glUniform3f(glGetUniformLocation(h_prog, "material.diffuse"), 0.50754, 0.50754, 0.50754);
			glUniform3f(glGetUniformLocation(h_prog, "material.specular"), 0.508273, 0.508273, 0.508273);
			glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), 0.4*128.0);
		}*/
		scene[i].render(false);
		
		
	}
	render_axes(P*V);

	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	glm::mat4	V = gui.T*gui.R;
	gui.position_mouse = glm::ivec2(x,y);
	if(((button==GLUT_LEFT_BUTTON) || (button==GLUT_RIGHT_BUTTON)) && (state==GLUT_DOWN))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(h_prog_pick);
		glm::mat4	M;
		for(int i=0 ; i<NUM_OBJS ; i++)
		{
			M = scene[i].T*scene[i].R;
			glUniformMatrix4fv(glGetUniformLocation(h_prog_pick, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));
			glUniform1i(glGetUniformLocation(h_prog_pick, "id"), scene[i].id);
			scene[i].render(true);
		}
		glFlush();
		GLubyte	pixel[4] = {0,0,0,0};
		glReadPixels(x, WIN_HEIGHT-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		selected = pixel[0];
		if(button==GLUT_LEFT_BUTTON)
		{
			if(selected > 0)	gui.state = STATE_ROTATING_OBJ;
			else				gui.state = STATE_ROTATING_WORLD;
		}
		else
		{
			if(selected > 0)	gui.state = STATE_MOVING_OBJ;
			else				gui.state = STATE_MOVING_WORLD;
		}
		glutPostRedisplay();
	}
}

void motion(int x, int y)
{
	gui.offset = glm::ivec2(x,y)-gui.position_mouse;
	gui.position_mouse = glm::ivec2(x,y);
	glm::vec4	viewport;
	glm::vec3	org_win, dir_win, dir_local;
	glGetFloatv(GL_VIEWPORT, glm::value_ptr(viewport));
	glm::mat4	MV;
	SceneGL*	p_scene;
	switch(gui.state)
	{
	case STATE_ROTATING_OBJ:
		p_scene = &scene[selected-1];
		MV = gui.T*gui.R*p_scene->T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.y, gui.offset.x, 0.0f);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		p_scene->R = glm::rotate(glm::mat4(1.0f), 1.0f*GLfloat(dir_win.length()), dir_local)*p_scene->R;
		glutPostRedisplay();
		break;
	case STATE_ROTATING_WORLD:
		MV = gui.T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.y, gui.offset.x, 0.0f);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		gui.R = glm::rotate(glm::mat4(1.0f), 1.0f*GLfloat(dir_win.length()), dir_local)*gui.R;
		glutPostRedisplay();
		break;
	case STATE_MOVING_OBJ:
		p_scene = &scene[selected-1];
		MV = gui.T*gui.R*p_scene->T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.x, -gui.offset.y, 0.0f);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		p_scene->T = glm::translate(glm::mat4(1.0f), dir_local)*p_scene->T;
		glutPostRedisplay();
		break;
	case STATE_MOVING_WORLD:
		MV = gui.T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.x, -gui.offset.y, 0.0f);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		gui.T = glm::translate(glm::mat4(1.0f), dir_local)*gui.T;
		glutPostRedisplay();
		break;
	}
}
void mouse_wheel(int wheel, int dir, int x, int y)
{
	camera.fovy = MIN(MAX(camera.fovy + dir, FOVY_MIN), FOVY_MAX);
	P = glm::perspective(camera.fovy, 1.0f, 0.1f, 20.0f);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
#ifdef  __APPLE__
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
#ifdef __FREEGLUT_H__
	glutMouseWheelFunc(mouse_wheel);
#endif
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();
}
