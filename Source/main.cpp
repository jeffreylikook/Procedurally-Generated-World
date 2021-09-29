//
// COMP 371 Lab 8 - Shadow Mapping
//
// Created by Simon Demeule on 10/11/2019.
//
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shaderloader.h"
#include "Camera.h"
#include "Tree.h"
#include "Bush.h"
#include "Ground.h"
#include "Bridge.h"
#include <list>

using namespace glm;
using namespace std;

// window dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;

//GLuint setupModelVBO(string path, int& vertexCount);

//Sets up a model using an Element Buffer Object to refer to vertex data
//GLuint setupModelEBO(string path, int& vertexCount);


// shader variable setters
void SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value)
{
	glUseProgram(shader_id);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value)
{
	glUseProgram(shader_id);
	glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

template <class T>
void SetUniform1Value(GLuint shader_id, const char* uniform_name, T uniform_value)
{
	glUseProgram(shader_id);
	glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
	glUseProgram(0);
}

GLFWwindow* window = nullptr;
bool InitContext();
GLuint loadTexture(const char *filename);
GLboolean CheckCollision(Camera &one, vector<Tree> &two);
GLboolean CheckCollision(Camera &one, Ground &two);
int sizeOfTheMap = 60;

int main(int argc, char* argv[])
{
	if (!InitContext()) return -1;

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	GLuint grassTextureID = loadTexture("../Assets/Textures/grass.jpg");
	GLuint cementTextureID = loadTexture("../Assets/Textures/cement.jpg");
	GLuint moonTextureID = loadTexture("../Assets/Textures/moon.jpg");
	GLuint leavesTextureID = loadTexture("../Assets/Textures/leaves.jpg");
	GLuint rockTextureID = loadTexture("../Assets/Textures/rock.jpg");
	GLuint bushTextureID = loadTexture("../Assets/Textures/bush.jpg");

#if defined(PLATFORM_OSX)
	std::string shaderPathPrefix = "Shaders/";
#else
	std::string shaderPathPrefix = "../Assets/Shaders/";
#endif

	GLuint shaderScene = loadSHADER(shaderPathPrefix + "scene_vertex.glsl", shaderPathPrefix + "scene_fragment.glsl");
	GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");
	

	// Setup texture and framebuffer for creating shadow map
	// Dimensions of the shadow texture, which should cover the viewport window size and shouldn't be oversized and waste resources
	const unsigned int DEPTH_MAP_TEXTURE_SIZE = 1024;

	// Variable storing index to texture used for shadow mapping
	GLuint depth_map_texture;
	// Get the texture
	glGenTextures(1, &depth_map_texture);
	// Bind the texture so the next glTex calls affect it
	glBindTexture(GL_TEXTURE_2D, depth_map_texture);
	// Create the texture and specify it's attributes, including widthn height, components (only depth is stored, no color information)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		NULL);
	// Set texture sampler parameters.
	// The two calls below tell the texture sampler inside the shader how to upsample and downsample the texture. Here we choose the nearest filtering option, which means we just use the value of the closest pixel to the chosen image coordinate.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// The two calls below tell the texture sampler inside the shader how it should deal with texture coordinates outside of the [0, 1] range. Here we decide to just tile the image.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	// Variable storing index to framebuffer used for shadow mapping
	GLuint depth_map_fbo;  // fbo: framebuffer object
						   // Get the framebuffer
	glGenFramebuffers(1, &depth_map_fbo);
	// Bind the framebuffer so the next glFramebuffer calls affect it
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	// Attach the depth map texture to the depth map framebuffer
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_map_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_texture, 0);
	glDrawBuffer(GL_NONE); //disable rendering colors, only write depth values
	
	/****************** CAMERAS **********************/
	Camera camera(vec3(0.0f, 10.0f, 20.0f));
	Camera camera2(vec3(0.0f, 100.0f, 0.0f));
	camera2.Pitch = -90.0f;
	Camera camera3(vec3(45.0f, 50.0f, 50.0f));
	camera3.Pitch = -45.0f;
	camera3.Yaw = -130.0f;
	Camera camera4(vec3(0.0f, 20.0f, 60.0f));

	vector<Camera> cameraVector;
	cameraVector.push_back(camera);
	cameraVector.push_back(camera2);
	cameraVector.push_back(camera3);
	cameraVector.push_back(camera4);

	int currentCamera = 0;

	int toggleModels = 0;

	// Set projection matrix for shader, this won't change
	mat4 projectionMatrix = glm::perspective(radians(cameraVector[currentCamera].Zoom),           // field of view in degrees
		WIDTH * 1.0f / HEIGHT, // aspect ratio
		0.01f, 800.0f);  // near and far (near > 0)

						 // Set initial view matrix on both shaders
	mat4 viewMatrix = cameraVector[currentCamera].GetViewMatrix();

									   // Set projection matrix on both shaders
	SetUniformMat4(shaderScene, "projection_matrix", projectionMatrix);

	// Set view matrix on both shaders
	SetUniformMat4(shaderScene, "view_matrix", viewMatrix);

	// Set light color on scene shader
	SetUniformVec3(shaderScene, "light_color", vec3(1.0, 1.0, 1.0));

	// Set object color on scene shader
	SetUniformVec3(shaderScene, "object_color", vec3(0.0, 1.0, 0.0));

	// For frame time
	float lastFrameTime = glfwGetTime();
	int lastMouseLeftState = GLFW_RELEASE;
	double lastMousePosX, lastMousePosY;
	glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

	// Other OpenGL states to set once
	// Enable Backface culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	/************************* SETUP MODELS  **************************/
	Ground ground(vec3(0.0f, 0.0f, 0.0f), 0.0f, vec3(1.0f, 1.0f, 1.0f), sizeOfTheMap);
	vector<Tree> treeList;
	for (int i = 0; i < sizeOfTheMap; i++) {
		treeList.push_back(Tree());
	}
	srand((unsigned)time(NULL));
	for (int i = 0; i < treeList.size() /2; ++i) {
		float scale = rand() % 3 + 1;
		int j = (rand() % 26) + 1;
		int k = (rand() % sizeOfTheMap) + 1;
		float t = ground.terrainHeight[j][k] + 2;
		treeList[i].scaleTreeHeight(scale);
		treeList[i].moveLocation(-30.0 + j*1.0f, t, -30.0 + k*1.0f);
	}
	for (int i = treeList.size()/2; i < treeList.size(); ++i) {
		float scale = rand() % 3 + 1;
		int j = 34 + (rand() % (sizeOfTheMap - 34));
		int k = (rand() % sizeOfTheMap) + 1;
		float t = ground.terrainHeight[j][k] + 2;
		treeList[i].scaleTreeHeight(scale);
		treeList[i].moveLocation(-30.0 + j*1.0f, t, -30.0 + k*1.0f);
	}

	vector<Bush> bushList;
	for (int i = 0; i < sizeOfTheMap; i++) {
		bushList.push_back(Bush());
	}
	srand((unsigned)time(0));
	for (int i = 0; i < bushList.size() / 2; ++i) {
		int j = (rand() % 26) + 1;
		int k = (rand() % sizeOfTheMap);
		float t = ground.terrainHeight[j][k] + 1;
		bushList[i].moveLocation(-30.0 + j * 1.0f, t, -30.0 + k * 1.0f);
	}
	for (int i = bushList.size() / 2; i < bushList.size(); ++i) {
		int j = 34 + (rand() % (sizeOfTheMap - 34));
		int k = (rand() % sizeOfTheMap) + 1;
		float t = ground.terrainHeight[j][k] + 1;
		bushList[i].moveLocation(-30.0 + j * 1.0f, t, -30.0 + k * 1.0f);
	}

	vector<Rock> rockList;
	for (int i = 0; i < sizeOfTheMap; i++) {
		rockList.push_back(Rock());
	}
	srand((unsigned)time(NULL));
	for (int i = 0; i < rockList.size() / 2; ++i) {
		int j = (rand() % 26) + 1;
		int k = (rand() % sizeOfTheMap);
		float t = ground.terrainHeight[j][k] + 1;
		rockList[i].moveLocation(-30.0 + j * 1.0f, t, -30.0 + k * 1.0f);
	}
	for (int i = rockList.size() / 2; i < rockList.size(); ++i) {
		int j = 34 + (rand() % (sizeOfTheMap - 34));
		int k = (rand() % sizeOfTheMap);
		float t = ground.terrainHeight[j][k] + 1;
		rockList[i].moveLocation(-30.0 + j * 1.0f, t, -30.0 + k * 1.0f);
	}

	
	// Entering Main Loop
	while (!glfwWindowShouldClose(window)) {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Frame time calculation
		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime = glfwGetTime();

		/*************************** LIGHTING ********************************/

		// light parameters
		vec3 lightPosition = vec3(-15.0f, 60.0f, -40.0f); // the location of the light in 3D space
		vec3 lightFocus(0.0, 0.0, -1.0);      // the point in 3D space the light "looks" at
		vec3 lightDirection = normalize(lightFocus - lightPosition);

		float lightNearPlane = 1.0f;
		float lightFarPlane = 180.0f;

		mat4 lightProjectionMatrix = perspective(50.0f, (float)DEPTH_MAP_TEXTURE_SIZE / (float)DEPTH_MAP_TEXTURE_SIZE, lightNearPlane, lightFarPlane);
		mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(0.0f, 1.0f, 0.0f));
		mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

		// Set light space matrix on both shaders
		SetUniformMat4(shaderShadow, "light_view_proj_matrix", lightSpaceMatrix);
		SetUniformMat4(shaderScene, "light_view_proj_matrix", lightSpaceMatrix);
		// Set light position on scene shader
		SetUniformVec3(shaderScene, "light_position", lightPosition);
		// Set light direction on scene shader
		SetUniformVec3(shaderScene, "light_direction", lightDirection);

		// Set the view matrix for first person camera and send to both shaders
		mat4 viewMatrix = cameraVector[currentCamera].GetViewMatrix();
		SetUniformMat4(shaderScene, "view_matrix", viewMatrix);

		// Set view position on scene shader
		SetUniformVec3(shaderScene, "view_position", cameraVector[currentCamera].Position);

		// Render shadow in 2 passes: 1- Render depth map, 2- Render scene
		// 1- Render shadow map:
		// a- use program for shadows
		// b- resize window coordinates to fix depth map output size
		// c- bind depth map framebuffer to output the depth values
		{
			// Use proper shader
			glUseProgram(shaderShadow);
			// Use proper image output size
			glViewport(0, 0, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE);
			// Bind depth map texture as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
			// Clear depth data on the framebuffer
			glClear(GL_DEPTH_BUFFER_BIT);

			ground.Draw(shaderShadow, grassTextureID, cementTextureID);
			glBindTexture(GL_TEXTURE_2D, depth_map_texture);
			//tree.Draw(shaderShadow);
			//tree.moveLocation(0.0f, 10.0f, 0.0f);

			if (toggleModels == 0) {
				ground.GenerateForest(shaderShadow, &treeList, &bushList, &rockList, rockTextureID, bushTextureID);
				glBindTexture(GL_TEXTURE_2D, depth_map_texture);
			}
			// Unbind geometry
			glBindVertexArray(0);
		}


		//2- Render scene: a- bind the default framebuffer and b- just render like what we do normally
		{
			// Use proper shader
			glUseProgram(shaderScene);
			// Use proper image output size
			// Side note: we get the size from the framebuffer instead of using WIDTH and HEIGHT because of a bug with highDPI displays
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			glViewport(0, 0, width, height);
			// Bind screen as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Clear color and depth data on framebuffer
			//glClearColor(0.53, 0.81, 0.92, 1.0f);
			glClearColor(0.0, 0.0, 0.0, 1.0f);

			// Bind depth map texture: not needed, by default it is active
			//glActiveTexture(GL_TEXTURE0);
			// Bind geometry
			if (toggleModels == 0) {
				ground.GenerateForest(shaderScene, &treeList, &bushList, &rockList, rockTextureID, bushTextureID);
			}

			ground.Draw(shaderScene, grassTextureID, cementTextureID);
			glBindTexture(GL_TEXTURE_2D, depth_map_texture);
			ground.DrawSky(shaderScene, moonTextureID);
			glBindTexture(GL_TEXTURE_2D, depth_map_texture);

			//tree.moveLocation(0.0f, 10.0f, 0.0f);
			//glBindTexture(GL_TEXTURE_2D, depth_map_texture);
			//tree.Draw(shaderScene);

			// Unbind geometry
			glBindVertexArray(0);
		}
		/**/

		glfwSwapBuffers(window);
		glfwPollEvents();

		// Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// - Calculate mouse motion dx and dy
		// - Update camera horizontal and vertical angle
		double mousePosX, mousePosY;
		glfwGetCursorPos(window, &mousePosX, &mousePosY);

		double dx = mousePosX - lastMousePosX;
		double dy = mousePosY - lastMousePosY;

		float xoffset = mousePosX - lastMousePosX;
		float yoffset = lastMousePosY - mousePosY; // reversed since y-coordinates go from bottom to top

		lastMousePosX = mousePosX;
		lastMousePosY = mousePosY;

		cameraVector[currentCamera].ProcessMouseMovement(xoffset, yoffset);

		// Use camera lookat and side vectors to update positions with ASDW
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

			if (CheckCollision(cameraVector[currentCamera], treeList) == true || CheckCollision(cameraVector[currentCamera], ground)) {
				cout << "Collision " << endl;
			}
			else {
				cameraVector[currentCamera].ProcessKeyboard(FORWARD, dt);
			}
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraVector[currentCamera].ProcessKeyboard(BACKWARD, dt);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraVector[currentCamera].ProcessKeyboard(LEFT, dt);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraVector[currentCamera].ProcessKeyboard(RIGHT, dt);

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
			currentCamera = 0;
			viewMatrix = cameraVector[currentCamera].GetViewMatrix();
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			currentCamera = 1;
			viewMatrix = cameraVector[currentCamera].GetViewMatrix();
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			currentCamera = 2;
			viewMatrix = cameraVector[currentCamera].GetViewMatrix();
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			currentCamera = 3;
			viewMatrix = cameraVector[currentCamera].GetViewMatrix();
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			toggleModels = !toggleModels;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			ground.rotateGround(0.2f);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			ground.rotateGround(-0.2f);
		}

		SetUniformMat4(shaderScene, "projection_matrix", projectionMatrix);
		SetUniformMat4(shaderScene, "view_matrix", viewMatrix);

	}

	glfwTerminate();

	return 0;
}

GLboolean CheckCollision(Camera &one, vector<Tree> &two) // AABB - AABB collision
{
	bool check;
	for (int i = 0; i < two.size(); i++) {
		// Collision x-axis?
		bool collisionX = one.Position.x + 1.0f >= two[i].position.x && two[i].position.x + two[i].scale_factor.x >= one.Position.x;
		// Collision y-axis?
		bool collisionY = one.Position.y + 1.0f >= two[i].position.y && two[i].position.y + two[i].scale_factor.y + 5 >= one.Position.y;
		// Collision only if on both axes
		bool collisionZ = one.Position.z + 1.0f >= two[i].position.z && two[i].position.z + two[i].scale_factor.z >= one.Position.z;

		check = collisionX && collisionY && collisionZ;
		if (check == true) {
			return true;
		}
	}
	return check;
}

GLboolean CheckCollision(Camera &one, Ground &two) // AABB - AABB collision
{
	// Collision x-axis?
	bool collisionX = one.Position.x + 1.0f >= -30 + two.position.x && -30 + two.position.x + two.size >= one.Position.x;
	// Collision y-axis?
	bool collisionY = one.Position.y + 1.0f >= two.position.y && two.position.y + two.scale_factor.y + 5 >= one.Position.y;
	// Collision only if on both axes
	bool collisionZ = one.Position.z + 1.0f >= -30 + two.position.z && -30 + two.position.z + two.size >= one.Position.z;

	return collisionX && collisionY && collisionZ;
}

bool InitContext()
{
	// Initialize GLFW and OpenGL version
	glfwInit();

#if defined(PLATFORM_OSX)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// On windows, we set OpenGL version to 2.1, to support more hardware
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

	// Create Window and rendering context using GLFW, resolution is 800x600
	window = glfwCreateWindow(WIDTH, HEIGHT, "Final Project", NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to create GLEW" << std::endl;
		glfwTerminate();
		return false;
	}

	return true;
}

GLuint loadTexture(const char *filename)
{
	// Step1 Create and bind textures
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	assert(textureId != 0);


	glBindTexture(GL_TEXTURE_2D, textureId);

	// Step2 Set filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Step3 Load Textures with dimension data
	int width, height, nrChannels;
	unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
		return 0;
	}

	// Step4 Upload the texture to the PU
	GLenum format = 0;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
		0, format, GL_UNSIGNED_BYTE, data);

	// Step5 Free resources
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureId;
}


