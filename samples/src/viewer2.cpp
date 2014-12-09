

///////////////////////////////////////////////////////////////////////
//
// viewer.cpp
//
// by Minho Kim (minhokim@uos.ac.kr) on 15 July 2013
//
///////////////////////////////////////////////////////////////////////
#pragma comment(lib, "../lib/assimp.lib")
#pragma comment(lib, "../lib/x32/DevIL.lib")
#pragma comment(lib, "../lib/x32/ILU.lib")
#pragma comment(lib, "../lib/x32/ILUT.lib")


#define _CRT_SECURE_NO_WARNINGS


//#include "MessageProcessor.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <stdlib.h>
#if __APPLE__
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
#include <IL/il.h>
#include <Windows.h>


#ifndef BUFFER_OFFSET	
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif

enum GUI_STATE {STATE_NONE=0, STATE_ROTATING_OBJ, STATE_MOVING_OBJ, STATE_ROTATING_WORLD, STATE_MOVING_WORLD,
				STATE_OUT_BOOK, STATE_IN_BOOK, STATE_SHAKING};
enum BOOK_STATE { BOOK_STATE_NONE = 0, BOOK_STATE_MOVING, BOOK_STATE_CLICKED, BOOK_STATE_OPENING, BOOK_STATE_OPEN};
enum BOOKSHELF_STATE { BOOKSHELF_STATE_NONE = 0, BOOKSHELF_STATE_FRONT, BOOKSHELF_STATE_BACK, BOOKSHELF_STATE_ROTATING};
enum OBJECT_TYPE { BOOKSHELF = 0 , BOOK, NULL_BOOK, ROOM};

GLuint	vao_axes, vao_room, vao_wall;

GLint	loc_color, loc_tex;
GLuint	tex, tex_unit = 3;
GLuint	sampler;

int saveIndex;

GLuint*		textureIds;							// pointer to texture Array


GLuint	h_prog, h_prog_pick, h_prog_axes, h_prog_tex, h_prog_room, h_prog_cubemap;
GLuint	selected = 0;
GLuint	world_state = STATE_NONE;

GLfloat angle_R = 0.0f;

#define	FOVY_MIN	5.0f
#define	FOVY_MAX	179.0f

#define BOOKSHEIF_HEIGHT 6.65f
#define BOOKSHELF_WIDTH  5.5f

#define	MIN(x,y)	((x)<(y)?(x):(y))
#define	MAX(x,y)	((x)>(y)?(x):(y))
#define ABS(x,y)	((x)>(y)?((x)-(y)):((y)-(x)))

#define M_PI        3.141592654f
#define DEG2RAD(d)	( (d) * M_PI/180.0f )

struct
{
	bool	perspective;
	GLfloat	fovy;
	glm::mat4 P;
	glm::mat4 V;
	glm::vec3 eye_point;
	glm::vec3 target_point;
} camera =
{
	true,
	100.0f,
	glm::mat4(1.0f),
	glm::mat4(1.0f),
	glm::vec3(0.0f,0.0f,12.0f),
	glm::vec3(0.0f,0.0f,0.0f),
};

struct
{
	GLuint		state;
	glm::ivec2	offset;
	glm::ivec2	position_mouse;
	glm::mat4	T;
	glm::mat4	R;
} gui =
{
	STATE_NONE,
	glm::ivec2(0,0),
	glm::ivec2(0,0),
	glm::mat4(1.0f),
	glm::mat4(1.0f),
};

#define	WIN_WIDTH	800
#define	WIN_HEIGHT	800


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

		//추가
		glUniform3f(glGetUniformLocation(h_prog, "material.ambient"), material.ambient.r, material.ambient.g, material.ambient.b);
		glUniform3f(glGetUniformLocation(h_prog, "material.diffuse"), material.diffuse.r, material.diffuse.g, material.ambient.b);
		glUniform3f(glGetUniformLocation(h_prog, "material.specular"), material.specular.r, material.specular.g, material.specular.b);
		glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), material.shininess);
		//material.ambient.a

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
	glm::mat4	S;
	int ObjectType;
	void* obj_ptr;
	SceneGL() {root = NULL;}
	~SceneGL() {if(root)	delete root;}
	void	build_from_aiScene(struct aiScene* p_scene);
	//void	render(bool picking);
	void	render();
};

void SceneGL::build_from_aiScene(struct aiScene*p_scene)
{
	root = new NodeGL(p_scene, NULL, p_scene->mRootNode);
}

/*
void SceneGL::render(bool picking)
{
	root->render();
	if(!picking)
	{
		//좌표축 노필요
		//render_axes(P*gui.T*gui.R*T*R);
		glUseProgram(h_prog);
	}
}
*/

void SceneGL::render()
{
	//glDepthMask(false);
	root->render();
	//glDepthMask(true);
}


typedef struct BookIndex
{
	GLuint x;
	GLuint y;
};


class Bookshelf;

class Book
{
public:
	glm::vec4 rpos;
	BookIndex ipos;
	BookIndex goto_ipos;
	GLfloat tic;
	SceneGL* scene;
	std::string name;
	std::string path;
	Bookshelf* parent;
	int state;

	Book(SceneGL* p_scene, Bookshelf* p_bs)
	{
		tic = 0.0f;
		state = BOOK_STATE_NONE;
		scene = p_scene;
		parent = p_bs;
	}
	void move(BookIndex s_pos, BookIndex e_pos);
	void SetIndexPosition(BookIndex bi)
	{
		ipos = bi;
		//초기 위치 설정
		rpos = indexToPos(ipos);
		scene->T = glm::translate(glm::mat4(1.0f), glm::vec3(rpos));
	}
	glm::vec4 indexToPos(BookIndex pos);
	void action();
	void SetMovement(BookIndex s_pos, BookIndex e_pos)
	{
		if (state != BOOK_STATE_NONE) return;
		ipos = s_pos;
		goto_ipos = e_pos;
		state = BOOK_STATE_MOVING;
	}

};


class Bookshelf 
{
public:

	SceneGL* scene;

	GLfloat height;
	GLfloat width;
	GLuint row_max;
	GLuint col_max;
	GLfloat tic;
	glm::vec4 rpos; // real world position
	int state;
	std::string name;
	std::vector<std::vector<Book*>> books;

	Bookshelf(SceneGL* p_scene, GLfloat _height, GLfloat _width, glm::vec4 pos, GLuint row, GLuint col)
	{
		tic = 0.0f;
		scene = p_scene;
		rpos = pos;
		row_max = row;
		col_max = col;
		height = _height;
		width = _width;
		books.resize(col_max);
		for (int i = 0; i < col_max; i++)
		{
			books[i].resize(row_max);
		}
		for (int j = 0; j < row_max; j++)
		{
			for (int i = 0; i < col_max; i++)
			{
				books[i][j] = NULL;
			}
		}
	}
	BookIndex insertBook(Book* p_book)
	{
		BookIndex bi = GetEmptyPosition();
		if (bi.x == -1 && bi.y == -1)
		{
			printf("책장의 빈공간이 없습니다.\n");
			return bi;
		}
		books[bi.x][bi.y] = p_book;
		//p_book->SetIndexPosition(bi);
		return bi;
	}
	void deleteBook(Book* p_book)
	{
		for (int j = 0; j < row_max; j++)
		{
			for (int i = 0; i < col_max; i++)
			{
				if (books[i][j] == p_book)
				{
					books[i][j] = NULL;
					//해당 정보 삭제와 scene에서 삭제 추가
					return;
				}
			}
		}
	}
	void deleteBook(BookIndex bi)
	{
		if (books[bi.x][bi.y] != NULL)
		{
			books[bi.x][bi.y] = NULL;
			//해당 정보 삭제와 scene에서 삭제 추가
			return;
		}
	}
	void action(void)
	{
		
		//자신의 행동 정의
		switch (state)
		{
		case BOOKSHELF_STATE_NONE:
			break;
		case BOOKSHELF_STATE_ROTATING:
			tic = tic + 0.1f;
			rpos = glm::rotate(glm::mat4(1.0f), 32.75f * tic, glm::vec3(0.0f, 1.0f, 0.0f)) * (rpos);
			//rpos = glm::rotate(glm::mat4(1.0f), DEG2RAD(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * (rpos);
		
			if (tic >= 1.0f)
			{
				tic = 0.0f;
				state = BOOKSHELF_STATE_NONE;
			}

			break;

		}

		scene->T = glm::translate(glm::mat4(1.0f), glm::vec3(rpos));

		//책들한테 액션 다 넣음
		for (int j = 0; j < row_max; j++)
		{
			for (int i = 0; i < col_max; i++)
			{
				if (books[i][j] != NULL)
				{
					books[i][j]->action();

				}
			}
		}


	}
	void SetPosition(glm::vec4 pos);
	glm::vec4 GetPosition();
	void SetRotate()
	{
		if (state == BOOKSHELF_STATE_NONE)
		{
			state = BOOKSHELF_STATE_ROTATING;
		}
	}
	//비어있는 자리 출력 함수
	BookIndex GetEmptyPosition(void);
};



BookIndex Bookshelf::GetEmptyPosition(void)
{
	BookIndex b;
	for (int j = 0; j < row_max; j++)
	{
		for (int i = 0; i < col_max; i++)
		{
			if (books[i][j] == NULL)
			{
				b.x = i, b.y = j;
				return b;
			}
		}
	}
	b.x = -1, b.y = -1;
	return b;
}


glm::vec4 Book::indexToPos(BookIndex pos)
{
	//추후 보정값
	glm::vec4 vec4_pos;
	vec4_pos.x = parent->rpos.x - parent->width / 2.0f + parent->width / parent->col_max * pos.x + (parent->width / parent->col_max) / 2.5f;
	vec4_pos.y = parent->rpos.y + parent->height - parent->height / parent->row_max * pos.y - (parent->height / parent->row_max) / 1.45f;
	vec4_pos.z = parent->rpos.z + 0.8f;
	return vec4_pos;
}


void Book::action()
{
	
	
	glm::vec4 init_rpos = indexToPos(ipos);
	glm::vec4 end_rpos = indexToPos(goto_ipos);
	switch (state)
	{
	case BOOK_STATE_NONE:
		rpos = indexToPos(ipos);
		break;
	case BOOK_STATE_MOVING:

		//바라보는 방향쪽으로?
		if (ABS(rpos.x, init_rpos.x) < 0.005f && ABS(rpos.y, init_rpos.y) < 0.005f && rpos.z - parent->rpos.z <= 1.8f)
		{
			//speed
			rpos.z = rpos.z + 0.05f;
		}
		else if (ABS(rpos.x, end_rpos.x) < 0.005f && ABS(rpos.y, end_rpos.y) < 0.005f && rpos.z - parent->rpos.z >= 0.8f)
		{
			//speed
			rpos.z = rpos.z - 0.05f;
			
		}
		else if (ABS(rpos.x, end_rpos.x) < 0.005f && ABS(rpos.y, end_rpos.y) < 0.005f && rpos.z - parent->rpos.z < 0.8f)
		{
			//도착 확인 -> 도착했으면 state 바꾸기
			state = BOOK_STATE_NONE;
			rpos = end_rpos;
			ipos = goto_ipos;
		}
		else
		{
			// speed = 0.1f
			tic = tic + 0.02f;
			rpos.x = init_rpos.x + (end_rpos.x - init_rpos.x) * tic;
			rpos.y = init_rpos.y + (end_rpos.y - init_rpos.y) * tic;
			// 바라보는 방향으로... 튀어나왔다 들어가게 
			
			rpos.z = parent->rpos.z + 1.8f + 0.5f * sin(3.14f * tic);
			if (tic >= 1.0f)
			{
				tic = 0.0f;
			}
		}

		break;
	case BOOK_STATE_CLICKED:

		break;

	}
	//위치보정
	scene->T = glm::translate(glm::mat4(1.0f), glm::vec3(rpos));

}

std::vector<SceneGL*> Scene_list;
std::vector<Bookshelf*> Bookshelf_list;
std::vector<Book*> Book_list;


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


void load_bookshelf(glm::vec4 pos)
{

	SceneGL* sc = new SceneGL();
	Scene_list.push_back(sc);

	const char *filename = { "data/models/test/modi4_desk.obj" };
	//scene[0].id = 0; index를 id로
	sc->ObjectType = BOOKSHELF;
	sc->T = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x,pos.y,pos.z));
	sc->R = glm::mat4(1.0f);
	sc->build_from_aiScene((aiScene*)aiImportFile(filename, aiProcessPreset_TargetRealtime_MaxQuality));
	//추후 텍스처 입히는 부분은 따로

	Bookshelf* b = new Bookshelf(sc, BOOKSHEIF_HEIGHT, BOOKSHELF_WIDTH,pos, 4, 10);
	Bookshelf_list.push_back(b);

}

Book* load_book(Bookshelf* bs, std::string name, std::string path)
{

	SceneGL* sc = new SceneGL();
	Scene_list.push_back(sc);
	const char *filename = { "data/models/test/modi_book.obj" };
	//scene[0].id = 0; index를 id로
	sc->ObjectType = BOOK;
	sc->T = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f));
	sc->R = glm::mat4(1.0f);
	sc->build_from_aiScene((aiScene*)aiImportFile(filename, aiProcessPreset_TargetRealtime_MaxQuality));

	Book* b = new Book(sc, bs);
	BookIndex bi = bs->insertBook(b);
	b->SetIndexPosition(bi);
	Book_list.push_back(b);
	return b;
}

void load_scene(void)
{

	//const char	*filename[NUM_OBJS] =
	//{
	//	//"data/models/sphere.dae",
	//	//"data/models/WusonOBJ.obj",
	//	//"data/models/torus.obj",
	//	//"data/models/monkey_low.obj",
	//	//"data/models/box.obj"

	//	//"data/models/test/room.obj",
	//	//"data/models/test/modi4_desk.obj",
	//	//"data/models/test/modi_book.obj",
	//	//"data/models/test/modi_book.obj"

	//	
	//	"data/models/test/modi4_desk.obj",
	//	"data/models/test/modi_book.obj",
	//	"data/models/test/modi_book.obj",
	//	"data/models/test/modi_book.obj"

	//};
	//

	//for(GLuint i=0 ; i<NUM_OBJS ; i++)
	//{
	//	scene[i].id = i+1;
	//	scene[i].T = glm::translate(glm::mat4(1.0f), glm::vec3(position[i][0], position[i][1], position[i][2]));
	//	scene[i].R = glm::mat4(1.0f);
	//	scene[i].build_from_aiScene((aiScene*)aiImportFile(filename[i],aiProcessPreset_TargetRealtime_MaxQuality));
	//	//scene[i].build_from_aiScene((aiScene*)aiImportFile(filename[i], aiProcess_OptimizeMeshes));
	//}

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
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

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
	glBindSampler(tex_unit, sampler);
}


void init_room(void)
{
	glm::mat4 P = camera.P;
	glm::mat4 V = camera.V;

	//glClearColor(.5, .5, .5, 1);
	GLfloat vertices[9][8] = {
		//빨간색
		{ -0.90, -0.90, -0.1, 1.0, 1, 0, 0, 0.3 },
		{ 0.50, -0.90, -0.1, 1.0, 1, 0, 0, 0.3 },
		{ -0.20, 0.50, -0.0, 1.0, 1, 0, 0, 0.3 },

		//초록색
		{ -0.70, -0.70, 0.1, 1.0, 0, 1, 0, 0.3 },
		{ 0.70, -0.70, 0.0, 1.0, 0, 1, 0, 0.3 },
		{ 0.00, 0.70, -0.1, 1.0, 0, 1, 0, 0.3 },

		//파란색
		{ -0.50, -0.50, 0.1, 1.0, 0, 0, 1, 0.3 },
		{ 0.90, -0.50, 0.1, 1.0, 0, 0, 1, 0.3 },
		{ 0.20, 0.90, 0.1, 1.0, 0, 0, 1, 0.3 },
	};


#define	VERT	8.0f
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

	glUniform1i(glGetUniformLocation(h_prog_room, "tex"), tex_unit);

	//P = glm::perspective(45.0f, 1.0f, 0.1f, 10.0f);
	//V = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -6.0f));
	//glm::mat4	M = glm::mat4(1.0f);

	//glUniformMatrix4fv(glGetUniformLocation(h_prog_room, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));


	//h_prog_cubemap = build_program_from_files("tex_cubemap.vert", "tex_cubemap.frag");
	//glUseProgram(h_prog_cubemap);
	//glUniform1i(glGetUniformLocation(h_prog_cubemap, "tex"), tex_unit);

	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	load_texture();

}


void render_room(glm::mat4 MVP)
{
	glm::mat4 P = camera.P;
	glm::mat4 V = camera.V;

	static float	angle = 0.0f;
	angle += 100.0f;
	if (angle > 360.0f)	angle -= 360.0f;

	glFrontFace(GL_CW);
	glUseProgram(h_prog_room);

	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	glBindVertexArray(vao_room);
	glEnable(GL_PRIMITIVE_RESTART);

	glUniformMatrix4fv(glGetUniformLocation(h_prog_room, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V));

	glPrimitiveRestartIndex(0xFFFF);
	glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glDisable(GL_PRIMITIVE_RESTART);

	glFrontFace(GL_CCW);


	//glUseProgram(h_prog_cubemap);
	//glActiveTexture(GL_TEXTURE0 + tex_unit);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	//glm::mat4	M = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	////glUniformMatrix4fv(glGetUniformLocation(h_prog_cubemap, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));
	////glUniformMatrix4fv(glGetUniformLocation(h_prog_cubemap, "MV"), 1, GL_FALSE, glm::value_ptr(V*M));
	////glUniformMatrix3fv(glGetUniformLocation(h_prog_cubemap, "M_normal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*M)));

	//glUniformMatrix4fv(glGetUniformLocation(h_prog_cubemap, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));


}

void init(void)
{
	//h_prog = build_program_from_files("shading_Blinn_Phong.vert", "shading_Blinn_Phong.frag");
	h_prog = build_program_from_files("shading_Phong_Phong.vert", "shading_Phong_Phong.frag");
	//h_prog = build_program_from_files("car.vert", "car.frag");

	h_prog_pick = build_program_from_files("picking_color.vert", "picking_color.frag");

	//P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -50.0f, 50.0f);
	camera.P = glm::perspective(camera.fovy, 1.0f, 0.1f, 40.0f);
	gui.T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	//load_scene();
	
	load_bookshelf(glm::vec4(-3.0f,-7.5f,3.0f,0.0f));
	


	Book* b_temp;
	BookIndex be;

	b_temp = load_book(Bookshelf_list[0], "aaa", "bbbb");
	be.x = 1; be.y = 2;
	b_temp->SetMovement(b_temp->ipos, be);

	
	load_bookshelf(glm::vec4(3.0f, -7.5f, -3.0f, 0.0f));


	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");
	load_book(Bookshelf_list[1], "aaa", "bbbb");


	//load_bookshelf(glm::vec4(0.0f, 1.0f, 4.0f, 0.0f));

	init_room();


}


void temp_add_book()
{

	Book* b_temp;
	BookIndex be;

	b_temp = load_book(Bookshelf_list[0], "aaa", "bbbb");
	do
	{
		be.x = rand() % (Bookshelf_list[0]->col_max - 1);
		be.y = rand() % (Bookshelf_list[0]->row_max - 1);
	} while (be.x == b_temp->ipos.x && be.y == b_temp->ipos.y);
	b_temp->SetMovement(b_temp->ipos, be);
}


void rotate_booksheif()
{
	//180도 원점 대칭 회전
	Bookshelf_list[0]->SetRotate();
	Bookshelf_list[1]->SetRotate();
}


void display(void)
{
	glm::mat4 P;
	glm::mat4	M;
	//glm::mat4	V = gui.T*gui.R;
	glm::mat4 V;

	camera.V = glm::lookAt(camera.eye_point, camera.target_point, glm::vec3(0, 1, 0));
	camera.V = glm::rotate(camera.V, angle_R, glm::vec3(0.0f, 1.0f, 0.0f));

	V = camera.V;
	P = camera.P;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//motion blur
	//glAccum(GL_RETURN, 0.99f);

	render_room(camera.P);

	glUseProgram(h_prog);

	//lighting

	glUniform4f(glGetUniformLocation(h_prog, "light.position"), 2.0f, 2.0f, 2.0f, 0.0f);
	glUniform3f(glGetUniformLocation(h_prog, "light.ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(h_prog, "light.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(h_prog, "light.specular"), 1.0f, 1.0f, 1.0f);


	
	for (int i = 0; i < Scene_list.size(); i++)
	{   
		
		M = Scene_list[i]->T*Scene_list[i]->R;
		glUniformMatrix4fv(glGetUniformLocation(h_prog, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));
		glUniformMatrix4fv(glGetUniformLocation(h_prog, "MV"), 1, GL_FALSE, glm::value_ptr(V*M));
		glUniformMatrix4fv(glGetUniformLocation(h_prog, "V"), 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix3fv(glGetUniformLocation(h_prog, "matNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(V*M)));

		//여기서 오브젝트가 투명이면...



		//if(Scene_list[i]-> == selected)
		//{
		//	// GOLD	http://devernay.free.fr/cours/opengl/materials.html
		//	//glUniform3f(glGetUniformLocation(h_prog, "material.ambient"), 0.24725, 0.1995, 0.0745);
		//	//glUniform3f(glGetUniformLocation(h_prog, "material.diffuse"), 0.75164, 0.60648, 0.22648);
		//	//glUniform3f(glGetUniformLocation(h_prog, "material.specular"), 0.628281, 0.555802, 0.366065);
		//	//glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), 0.4*128.0);
		//	

		//}
		//else
		//{
		//	// SILVER	http://devernay.free.fr/cours/opengl/materials.html
		//	//glUniform3f(glGetUniformLocation(h_prog, "material.ambient"), 0.19225, 0.19225, 0.19225);
		//	//glUniform3f(glGetUniformLocation(h_prog, "material.diffuse"), 0.50754, 0.50754, 0.50754);
		//	//glUniform3f(glGetUniformLocation(h_prog, "material.specular"), 0.508273, 0.508273, 0.508273);
		//	//glUniform1f(glGetUniformLocation(h_prog, "material.shininess"), 0.4*128.0);
		//}




		//scene[i].render(false);
		if (Scene_list[i]->ObjectType == NULL_BOOK)
		Scene_list[i]->render();
	}
	//좌표축 노필요
	//render_axes(P*V);
	//render_wall(P);
	
	glutSwapBuffers();
	//motion blur
	//glAccum(GL_ACCUM, 0.98f);
}

void mouse(int button, int state, int x, int y)
{
	glm::mat4 P = camera.P;


	glm::mat4	V = gui.T*gui.R;
	gui.position_mouse = glm::ivec2(x,y);
	SceneGL*	p_scene;

	glm::vec4	viewport;
	glm::vec3	org_win, dir_win, dir_local;
	glm::mat4	MV;

	//print!!
	char buf[128] = { 0, };
	sprintf(buf, "x = %d \t y = %d\t\t \n", x, y);

//	msgpr::sendMsg(buf);

	if(((button==GLUT_LEFT_BUTTON) || (button==GLUT_RIGHT_BUTTON)) && ((state==GLUT_DOWN) || (state==GLUT_UP)))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(h_prog_pick);
		glm::mat4	M;
		for(int i=0 ; i<Scene_list.size() ; i++)
		{
			M = Scene_list[i]->T*Scene_list[i]->R;
			glUniformMatrix4fv(glGetUniformLocation(h_prog_pick, "MVP"), 1, GL_FALSE, glm::value_ptr(P*V*M));
			//glUniform1i(glGetUniformLocation(h_prog_pick, "id"), scene[i].id);
			glUniform1i(glGetUniformLocation(h_prog_pick, "id"), i);
			//scene[i].render(true);
			Scene_list[i]->render();
		}
		glFlush();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		//window_width = glutGet(GLUT_WINDOW_WIDTH);
		int tempy = glutGet(GLUT_WINDOW_HEIGHT);

		GLubyte	pixel[4] = {0,0,0,0};
		glReadPixels(x, tempy -y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		selected = pixel[0];


		if (button == GLUT_LEFT_BUTTON && state== GLUT_DOWN)
		{
			//if (selected > 0)	gui.state = STATE_ROTATING_OBJ;
			//else				gui.state = STATE_ROTATING_WORLD;

		
			if (selected > 1)
			{
				gui.state = STATE_OUT_BOOK;
				saveIndex = selected - 1;
				p_scene = Scene_list[selected-1];
				p_scene->T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, +1.5f))*p_scene->T;
			}
			else
			{
				gui.state = STATE_ROTATING_WORLD;
			}


		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			if (selected > 0)	gui.state = STATE_MOVING_OBJ;
			else				gui.state = STATE_MOVING_WORLD;
		}
		else if (state == GLUT_UP)
		{
			if (selected > 0 && gui.state == STATE_OUT_BOOK)
			{
				gui.state = STATE_IN_BOOK;
				selected = saveIndex + 1;
				p_scene = Scene_list[saveIndex];
				p_scene->T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.5f))*p_scene->T;
			}
		}
		glutPostRedisplay();
	}

}

void world_shake()
{
	if (world_state != STATE_SHAKING) return;
	static GLfloat tic = 0.0f;
	tic = tic + 0.1f;
	camera.eye_point = glm::vec3(camera.eye_point.x + 8.0f * sin(3.14f * 10.0f * tic), camera.eye_point.y, camera.eye_point.z);
	camera.target_point = glm::vec3(camera.target_point.x + 8.0f * sin(3.14f * 10.0f * tic), camera.target_point.y, camera.target_point.z);

	if (tic >= 1.0f)
	{
		tic = 0.0f;
		world_state = STATE_NONE;
	}
}

void setWorldShaking()
{
	if (world_state != STATE_SHAKING)
	{
		world_state = STATE_SHAKING;
	}
}
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		//시점 회전
		angle_R += 5.0f;
		glutPostRedisplay();
		break;
	case 'd':
		//시점 회전
		angle_R -= 5.0f;
		glutPostRedisplay();
		break;
	case 'q':
		temp_add_book();
		break;
	case 'e':
		rotate_booksheif();
		break;
	case 't':
		setWorldShaking();
		break;
		
	}
}

void special(int key, int x, int y)
{
	//SceneGL*	p_scene;
	//p_scene = &scene[0];
	//glm::mat4 T_base, R_base;
	//glm::mat4	MV;


	//glm::vec4	viewport;
	//glm::vec3	org_win, dir_win, dir_local;
	////glGetFloatv(GL_VIEWPORT, glm::value_ptr(viewport));


	//switch (key)
	//{
	//case GLUT_KEY_UP:
	//	local_position = local_position + glm::normalize(p_scene->R * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)) * 0.15f;
	//	T_base = glm::translate(glm::mat4(1.0f), glm::vec3(local_position));

	//	//p_scene->T = p_scene->T * T_base;
	//	p_scene->T = T_base;


	//	glutPostRedisplay();
	//	break;
	//case GLUT_KEY_DOWN:
	//	//T_base = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5));
	//	local_position = local_position - glm::normalize(p_scene->R * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)) * 0.15f;
	//	T_base = glm::translate(glm::mat4(1.0f), glm::vec3(local_position));

	//	//p_scene->T = p_scene->T * T_base;
	//	p_scene->T = T_base;


	//	glutPostRedisplay();
	//	break;
	//case GLUT_KEY_LEFT:
	//	angle_R += 10.0f;
	//	R_base = glm::rotate(glm::mat4(1.0f), angle_R, glm::vec3(0.0f, 1.0f, 0.0f));

	//	//p_scene->R = p_scene->R * R_base;
	//	p_scene->R = R_base;

	//	glutPostRedisplay();
	//	break;
	//case GLUT_KEY_RIGHT:
	//	angle_R -= 10.0f;
	//	R_base = glm::rotate(glm::mat4(1.0f), angle_R, glm::vec3(0.0f, 1.0f, 0.0f));

	//	//p_scene->R = p_scene->R * R_base;
	//	p_scene->R = R_base;

	//	glutPostRedisplay();
	//	break;
	//}
}

void motion(int x, int y)
{
	glm::mat4 P = camera.P;

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
		if (selected == 1) break; //임시 책장 이동 unable
		p_scene = Scene_list[selected - 1];
		MV = gui.T*gui.R*p_scene->T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.y, gui.offset.x, 0.0f);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		p_scene->R = glm::rotate(glm::mat4(1.0f), 1.0f*GLfloat(dir_win.length()), dir_local)*p_scene->R;
		glutPostRedisplay();
		break;
	case STATE_ROTATING_WORLD:
		//MV = gui.T;
		//org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		//dir_win = glm::vec3(0.0f, gui.offset.x, 0.0f);
		//dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		//gui.R = glm::rotate(glm::mat4(1.0f), 1.0f*GLfloat(dir_win.length()), dir_local)*gui.R;
		//glutPostRedisplay();

		/*MV = gui.T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.y, gui.offset.x, 0.0f);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		P = glm::rotate(glm::mat4(1.0f), 1.0f*GLfloat(dir_win.length()), dir_local)*P;
		glutPostRedisplay();*/

		break;
	case STATE_MOVING_OBJ:
		if (selected == 1) break; //임시 책장 이동 unable
		p_scene = Scene_list[selected - 1];
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
	case STATE_OUT_BOOK:
		if (selected == 1) break; //임시 책장 이동 unable
		p_scene = Scene_list[selected - 1];
		MV = gui.T*gui.R*p_scene->T;
		org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
		dir_win = glm::vec3(gui.offset.x, -gui.offset.y, 0);
		dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
		p_scene->T = glm::translate(glm::mat4(1.0f), dir_local)*p_scene->T;
		glutPostRedisplay();
		break;
	//case STATE_IN_BOOK:
	//	if (selected == 1) break; //임시 책장 이동 unable
	//	p_scene = &scene[selected - 1];
	//	MV = gui.T*gui.R*p_scene->T;
	//	org_win = glm::project(glm::vec3(0.0f, 0.0f, 0.0f), MV, P, viewport);
	//	dir_win = glm::vec3(gui.offset.x, -gui.offset.y, 0.0f);
	//	dir_local = glm::unProject(dir_win + org_win, MV, P, viewport);
	//	p_scene->T = glm::translate(glm::mat4(1.0f), dir_local)*p_scene->T;
	//	glutPostRedisplay();
	//	break;
	}
}

//휠 운용 - 카메라 시점 변환
void mouse_wheel(int wheel, int dir, int x, int y)
{
	//camera.fovy = MIN(MAX(camera.fovy + dir, FOVY_MIN), FOVY_MAX);
	//camera.P = glm::perspective(camera.fovy, 1.0f, 0.1f, 20.0f);
	//P = glm::perspective(camera.fovy, x, y, 20.0f);
	
	camera.eye_point = glm::vec3(camera.eye_point.x, camera.eye_point.y, camera.eye_point.z + (GLfloat)dir * 0.1f);
	
	glutPostRedisplay();
}

void timer(int value)
{
	
	Bookshelf_list[0]->action();
	Bookshelf_list[1]->action();
	world_shake();
	glutPostRedisplay();
	glutTimerFunc(30, timer, 1);
}

void print(char* buf)
{
	puts(buf);
}

int main(int argc, char** argv)
{
//	msgpr::initialize(print);
	glutInit(&argc, argv);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
#ifdef  __APPLE__
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow(argv[0]);
#else
	//glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
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
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutTimerFunc(30, timer, 1);
	glutMainLoop();
}
