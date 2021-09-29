/*
Class used to create the Rock object for the program
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
#include <list>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string>

using namespace std;
using namespace glm;

class Rock {
public:
	Rock();
	Rock(vec3 mPosition, float mRotate, vec3 mScale);
	void moveLocation(float x, float y, float z);
	void scaleRock(float scale);
	vec3 generateColor();
	void Draw(int shaderProgram);
	void SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value);
	void SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value);

	GLuint setupModelEBO(string path, int& vertexCount);
	bool loadOBJ2(const char* path, std::vector<int>& vertexIndices, std::vector<glm::vec3>& temp_vertices, std::vector<glm::vec3>& out_normals, std::vector<glm::vec2>& out_uvs);
	int Rock::randomNumberFromOneTo(int max);

private:
	vec3 scale_factor;
	vec3 position;
	float rotation_angle;
	vec3 color_f;

	float shape_x;
	float shape_y;
	float shape_z;

	int cubeVertices;
	GLuint cubeVAO;
	string cubePath = "../Assets/Models/cube.obj";
};

