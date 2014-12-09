///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
// modified by Minho Kim (minhokim@uos.ac.kr) on 15 July 2013
//
///////////////////////////////////////////////////////////////////////
#pragma comment(lib, "../lib/assimp.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <stdlib.h>
#ifdef  __APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/GLUT.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include <vector>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
GLuint	vao_quad, vao_axes,vao_room;
GLuint	buf_quad, buf_axes,buf_room;
GLfloat	angle_R, angle_G, angle_B;
glm::mat4	P, V, T,T1,T2;
GLuint	h_prog,prog_room;

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
		glUniform3fv(glGetUniformLocation(h_prog, "material.ambient"), 1, &material.ambient[0]);
		glUniform3fv(glGetUniformLocation(h_prog, "material.diffuse"), 1, &material.diffuse[0]);
		glUniform3fv(glGetUniformLocation(h_prog, "material.specular"), 1, &material.specular[0]);
		glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), material.shininess);
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
	max = 1;
	int	ret1, ret2;
	ret1 = aiGetMaterialFloatArray(p_mtl, AI_MATKEY_SHININESS, &material.shininess, &max);
	if(ret1 == AI_SUCCESS)
	{
		max = 1;
		ret2 = aiGetMaterialFloatArray(p_mtl, AI_MATKEY_SHININESS_STRENGTH, &material.strength, &max);
		if(ret2 == AI_SUCCESS)	material.shininess *= material.strength;
	}
	if(material.shininess == 0.0f)	material.specular = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);

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


SceneGL*	scene;


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

void load_scene(void)
{
//#define	FILENAME	"data/models/sphere.dae"
//#define	FILENAME	"data/models/WusonOBJ.obj"
//#define	FILENAME	"data/models/torus.obj"
//#define	FILENAME	"data/models/monkey_low.obj"
#define	FILENAME	"data/models/desk.obj"
#define FILENAME2	"data/models/house.obj"
#define FILENAME3	"data/models/FREE_TRASHCAN_2_OBJ.obj"
#define FILENAME4	"data/models/BedRound.obj"

//#define	FILENAME	"data/models/spider.obj"
//#define	FILENAME	"data/models/duck_triangulate.dae"
	aiScene*	p_scene = (aiScene*)aiImportFile(FILENAME,aiProcessPreset_TargetRealtime_MaxQuality);
	aiScene*	p_scene2 = (aiScene*)aiImportFile(FILENAME2,aiProcessPreset_TargetRealtime_MaxQuality);
	aiScene*	p_scene3 = (aiScene*)aiImportFile(FILENAME3,aiProcessPreset_TargetRealtime_MaxQuality);
	aiScene*	p_scene4 = (aiScene*)aiImportFile(FILENAME4,aiProcessPreset_TargetRealtime_MaxQuality);
	scene = (SceneGL*)malloc(2*sizeof(SceneGL));
	scene[0].build_from_aiScene(p_scene);
	scene[1].build_from_aiScene(p_scene2);
	scene[2].build_from_aiScene(p_scene3);
	scene[3].build_from_aiScene(p_scene4);
}
void init(void)
{
//	h_prog = build_program_from_files("shading_Phong_Gouraud.vert", "shading_Phong_Gouraud.frag");
//	h_prog = build_program_from_files("shading_Phong_Phong.vert", "shading_Phong_Phong.frag");
//	h_prog = build_program_from_files("shading_Blinn_Gouraud.vert", "shading_Blinn_Gouraud.frag");
	h_prog = build_program_from_files("shading_Blinn_Phong.vert", "shading_Blinn_Phong.frag");

	glUseProgram(h_prog);

//	P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -100.0f, 100.0f);
	P = glm::perspective(100.0f, 1.0f, 0.1f, 200.0f);
	V = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f));
	T = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f,0.3f,-0.05f));
	T1 =  glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f,0.3f,-0.5f));
	T2 =  glm::translate(glm::mat4(1.0f), glm::vec3(1.0f,0.3f,-1.0f));
	glEnable(GL_DEPTH_TEST);

	glClearColor(.5f, .5f, .5f, .5f);

	load_scene();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform4f(glGetUniformLocation(h_prog, "light.position"), 2.0f, 2.0f, 2.0f, 0.0f);
	glUniform3f(glGetUniformLocation(h_prog, "light.ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(h_prog, "light.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(h_prog, "light.specular"), 1.0f, 1.0f, 1.0f);

	glm::mat4	M;
	glm::mat4 V = glm::lookAt(glm::vec3(10.0f,10.0f,0.7f),glm::vec3(0,0,0),glm::vec3(0,1,0));
	M = glm::scale(M, glm::vec3(0.3f, 0.3f, 0.3f));	// for "spider.obj"
//	M = glm::scale(M, glm::vec3(0.01f, 0.01f, 0.01f));	// for "spider.obj"
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*T*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, glm::value_ptr(V*T*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "V"), 1, GL_FALSE, glm::value_ptr(V*T));
	glUniformMatrix3fv(glGetUniformLocation(h_prog, "matNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*T*M)));
	scene[0].render();

	M = glm::scale(M, glm::vec3(0.1f, 0.1f, 0.1f));	// for "spider.obj"
//	M = glm::scale(M, glm::vec3(0.01f, 0.01f, 0.01f));	// for "spider.obj"
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*T1*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, glm::value_ptr(V*T1*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "V"), 1, GL_FALSE, glm::value_ptr(V*T1));
	glUniformMatrix3fv(glGetUniformLocation(h_prog, "matNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*T1*M)));
	scene[2].render();

//	M = glm::scale(M, glm::vec3(3.0f, 3.0f, 3.0f));	// for "spider.obj"
	M = glm::scale(M, glm::vec3(0.4f, 0.4f, 0.4f));	// for "spider.obj"
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*T2*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, glm::value_ptr(V*T2*M));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "V"), 1, GL_FALSE, glm::value_ptr(V*T2));
	glUniformMatrix3fv(glGetUniformLocation(h_prog, "matNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*T2*M)));
	scene[3].render();
	
	
	M = glm::scale(M, glm::vec3(0.8f,0.8f,0.8f));
	glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V));
	//glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, glm::value_ptr(V));
	//glUniformMatrix4fv(glGetUniformLocation(h_prog, "V"), 1, GL_FALSE, glm::value_ptr(V));
	glUniformMatrix3fv(glGetUniformLocation(h_prog, "matNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V)));
	
	scene[1].render();

	glutSwapBuffers();

}

void idle(void)
{
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 1024);
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
	glutIdleFunc(idle);
	glutMainLoop();
}
