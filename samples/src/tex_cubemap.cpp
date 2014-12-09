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
#pragma comment(lib, "../lib/assimp.lib")

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cerrno>
#include <stdlib.h>
#include <IL/il.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/GLUT.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint	vao_room;
GLuint	tex, tex_unit = 3;
GLuint	sampler;
GLuint	h_prog_room, h_prog_cubemap;
glm::mat4	P, V;


class MeshGL
{
	GLuint	vao;
	GLuint	vbo_positions;
	GLuint	vbo_texcoords;
	GLuint	vbo_normals;
	GLuint	vbo_indices;
	GLuint	nTriangles;
	GLuint	nVertices;
	struct
	{
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

class SceneGL
{
	NodeGL		*root;
public:
	SceneGL() {root = NULL;}
	~SceneGL() {if(root)	delete root;}
	void	build_from_aiScene(struct aiScene* p_scene);
	void	render(void);
};

void SceneGL::build_from_aiScene(struct aiScene*p_scene)
{
	root = new NodeGL(p_scene, NULL, p_scene->mRootNode);
}

void SceneGL::render(void)
{
	root->render();
}


SceneGL	scene;

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
	ilGenImages(6,img);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	for(int i=0 ; i<6 ; i++)
	{
		ilBindImage(img[i]);
		ilLoadImage(filenames[i]);
		glTexImage2D(faces[i], 0, GL_RGB8, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
			ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
	}
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindSampler(tex_unit, sampler);
}

void load_scene(void)
{
//#define	FILENAME	"data/models/sphere.dae"
//#define	FILENAME	"data/models/WusonOBJ.obj"
//#define	FILENAME	"data/models/torus.obj"
//#define	FILENAME	"data/models/monkey_low.obj"
#define	FILENAME	"data/models/monkey_high.obj"
	aiScene*	p_scene = (aiScene*)aiImportFile(FILENAME,aiProcessPreset_TargetRealtime_MaxQuality);
	scene.build_from_aiScene(p_scene);
}

void init(void)
{
	glClearColor(.5, .5, .5, 1);

#define	VERT	2.0f
	static const GLfloat positions[8][4] =
	{
		{-VERT, -VERT, -VERT, VERT},
		{-VERT, -VERT,  VERT, VERT},
		{-VERT,  VERT, -VERT, VERT},
		{-VERT,  VERT,  VERT, VERT},
		{ VERT, -VERT, -VERT, VERT},
		{ VERT, -VERT,  VERT, VERT},
		{ VERT,  VERT, -VERT, VERT},
		{ VERT,  VERT,  VERT, VERT},
	};
#undef	VERT
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
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glUniform1i(glGetUniformLocation(h_prog_room, "tex"), tex_unit);

	P = glm::perspective(45.0f, 1.0f, 0.1f, 10.0f);
	V = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -6.0f));
	glm::mat4	M = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(h_prog_room, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));


	h_prog_cubemap = build_program_from_files("tex_cubemap.vert", "tex_cubemap.frag");
	glUseProgram(h_prog_cubemap);
	glUniform1i(glGetUniformLocation(h_prog_cubemap, "tex"), tex_unit);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	load_texture();
	load_scene();
}

void display(void)
{
	static float	angle = 0.0f;
	angle += 1.0f;
	if(angle > 360.0f)	angle -= 360.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFrontFace(GL_CW);
	glUseProgram(h_prog_room);

	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	glBindVertexArray(vao_room);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    glDisable(GL_PRIMITIVE_RESTART);

	glFrontFace(GL_CCW);
	glUseProgram(h_prog_cubemap);
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glm::mat4	M = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(h_prog_cubemap, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog_cubemap, "MV"), 1, GL_FALSE, glm::value_ptr(V*M));
	glUniformMatrix3fv(glGetUniformLocation(h_prog_cubemap, "M_normal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*M)));
	scene.render();

	glutSwapBuffers();
}

void idle(void)
{
	glutPostRedisplay();
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
	glutIdleFunc(idle);
	glutMainLoop();
}
