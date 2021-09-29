/*
Class used to create the ground object for the program
completely procedurally generated using perlin noise.
Also will generate the other models onto this ground.
*/
#pragma once

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <list>
#include<string>

#include "Tree.h"
#include "Bush.h"
#include "Rock.h"

class Tree;

using namespace std;
using namespace glm;

class Ground {
public:
	Ground(int size);
	Ground(vec3 mPosition, float mRotate, vec3 mScale, int size);
	void moveLocation(float x, float y);
	void scaleGround(float scale);
	void rotateGround(float angle);
	void Draw(int shaderProgram, GLuint grassTextureID, GLuint cementTextureID);
	void DrawSky(int shaderProgram, GLuint moonTextureID);
	void GenerateForest(int shaderProgram, vector<Tree>* treeList, vector<Bush>* bushList, vector<Rock>* rockList, GLuint rockTextureID, GLuint bushTextureID);
	void GenerateForest(int shaderProgram, list<Tree>* tree);
	vec3 generateColor(float height);
	void SetUniformMat4(GLuint shader_id, const char * uniform_name, mat4 uniform_value);
	void SetUniformVec3(GLuint shader_id, const char * uniform_name, vec3 uniform_value);

	GLuint setupModelEBO(string path, int & vertexCount);
	bool loadOBJ2(const char * path, std::vector<int>& vertexIndices, std::vector<glm::vec3>& temp_vertices, std::vector<glm::vec3>& out_normals, std::vector<glm::vec2>& out_uvs);
	int size;
	vector<vector<float>> terrainHeight;
	vec3 scale_factor;
	vec3 position;

private:
	
	float rotation_angle;
	vec3 color_f;

	int cubeVertices;
	GLuint cubeVAO;
	int sphereVertices;
	GLuint sphereVAO;
	int cylinderVertices;
	GLuint cylinderVAO;
	
	float perlNoise;

	string cylinderPath = "../Assets/Models/cylinder.obj";
	string cubePath = "../Assets/Models/cube.obj";
	string spherePath = "../Assets/Models/sphere.obj";
	
};