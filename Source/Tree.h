/*
Class used to create the Tree object for the program
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
#include<string>


using namespace std;
using namespace glm;

class Tree {
public:
	Tree();
	Tree(vec3 mPosition, float mRotate, vec3 mScale);
	void moveLocation(float x, float y, float z);
	void scaleTree(float scale);
	void scaleTreeHeight(float scale);
	vec3 generateColor(int height);
	void Draw(int shaderProgram);
	void SetUniformMat4(GLuint shader_id, const char * uniform_name, mat4 uniform_value);

	void SetUniformVec3(GLuint shader_id, const char * uniform_name, vec3 uniform_value);

	GLuint setupModelEBO(string path, int & vertexCount);

	bool loadOBJ2(const char * path, std::vector<int>& vertexIndices, std::vector<glm::vec3>& temp_vertices, std::vector<glm::vec3>& out_normals, std::vector<glm::vec2>& out_uvs);

	vec3 scale_factor;
	vec3 position;
private:
	//vec3 scale_factor;
	//vec3 position;
	float rotation_angle;
	vec3 color_f;

	int cubeVertices;
	GLuint cubeVAO;
	int sphereVertices;
	GLuint sphereVAO;
	int cylinderVertices;
	GLuint cylinderVAO;

	string cylinderPath = "../Assets/Models/cylinder.obj";
	string cubePath = "../Assets/Models/cube.obj";
	string spherePath = "../Assets/Models/sphere.obj";
};