#include "Ground.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "PerlinNoise.h"

/* Default Constructor */
Ground::Ground(int size)
{
	position = vec3(0.0f, 0.0f, 0.0f);
	rotation_angle = 0.0f;
	scale_factor = vec3(1.0f, 1.0f, 1.0f) * (float)(size / 60);

	this->size = size;
	vector<float> v(size + 1);
	vector<vector<float>> v2(size + 1, v);
	terrainHeight = v2;

	cubeVAO = setupModelEBO(cubePath, cubeVertices);
	sphereVAO = setupModelEBO(spherePath, sphereVertices);
	cylinderVAO = setupModelEBO(cylinderPath, cylinderVertices);

	unsigned int seed = 256;
	PerlinNoise pn(seed);

	for (int row = 0; row <= size; row++) {
		//Draw Horizontal lines
		for (int column = 0; column <= size; column++) {
			double x = (double)column / ((double)size + 1);
			double y = (double)row / ((double)size + 1);
			perlNoise = pn.noise(10 * x, 10 * y, 20.0);
			cout << perlNoise << endl;
			terrainHeight[row][column] = perlNoise;
		}
	}
}

/* Constructor init position, rotation, and scale of ground */
Ground::Ground(vec3 mPosition, float mRotate, vec3 mScale, int size) : position(mPosition), rotation_angle(mRotate), scale_factor(mScale)
{
	//Load VAO
	cubeVAO = setupModelEBO(cubePath, cubeVertices);
	sphereVAO = setupModelEBO(spherePath, sphereVertices);
	cylinderVAO = setupModelEBO(cylinderPath, cylinderVertices);

	this->size = size;
	vector<float> v(size + 1);
	vector<vector<float>> v2(size + 1, v);
	terrainHeight = v2;

	//Add perlin noise
	srand((unsigned)time(NULL));
	
	unsigned int seed = rand() % 200 + 1;
	PerlinNoise pn(seed);

	//Create grid
	for (int row = 0; row <= size; row++) {
		//Draw Horizontal lines
		for (int column = 0; column <= size; column++) {
			double x = (double)column / ((double)size + 1);
			double y = (double)row / ((double)size + 1);
			perlNoise = pn.noise(10 * x, 10 * y, 0.8) * 6.0f;
			cout << perlNoise << endl;

			//assign height of cube to perlin noise
			terrainHeight[row][column] = perlNoise;
			cout << terrainHeight[row][column] << endl;
		}
	}
}

//Move location of grid
void Ground::moveLocation(float x, float y)
{
	position.x = x;
	position.z = y;
}

//Scale Grid
void Ground::scaleGround(float scale)
{
	scale_factor += scale;
}

//Scale Grid
void Ground::rotateGround(float angle)
{
	rotation_angle += angle;
}

//Draw method
void Ground::Draw(int shaderProgram, GLuint grassTextureID, GLuint cementTextureID)
{
	glBindVertexArray(cubeVAO);

	mat4 partMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
	mat4 groupMatrix = translate(mat4(1.0f), position) * rotate(mat4(1.0f), rotation_angle, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), scale_factor);
	mat4 worldMatrix = groupMatrix * partMatrix;

	SetUniformVec3(shaderProgram, "object_color", vec3(0.0, 1.0, 0.0));
	SetUniformMat4(shaderProgram, "model_matrix", worldMatrix);

	glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);

	//Draw the grid
	for (int row = 0; row <= size; row++) {
		//Draw Horizontal lines
		for (int column = 0; column <= size; column++) {
			partMatrix = translate(mat4(1.0f), vec3(-30.0f + column * 1.0f, 1.0*(terrainHeight[row][column]), -30.0f + row * 1.0f)) * scale(mat4(1.0f), vec3(1.0f, 4.0f, 1.0f));
			worldMatrix = groupMatrix * partMatrix;
			SetUniformMat4(shaderProgram, "model_matrix", worldMatrix);
			if (column > 27 && column < 33) {
				glBindTexture(GL_TEXTURE_2D, cementTextureID);
				SetUniformVec3(shaderProgram, "object_color", vec3(1.0, 1.0, 1.0));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, grassTextureID);
				SetUniformVec3(shaderProgram, "object_color", generateColor(terrainHeight[row][column]));
			}
			glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}		
	}
	
}

//Draw method
void Ground::DrawSky(int shaderProgram, GLuint moonTextureId)
{
	glBindVertexArray(sphereVAO);

	mat4 partMatrix = translate(mat4(1.0f), vec3(-10.0f, 4.0f, 0.0f)) * rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(3.0f, 3.0f, 3.0f));
	mat4 groupMatrix = translate(mat4(1.0f), position) * rotate(mat4(1.0f), rotation_angle, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), scale_factor);
	mat4 worldMatrix = groupMatrix * partMatrix;

	SetUniformVec3(shaderProgram, "object_color", vec3(1.0, 1.0, 1.0));
	SetUniformMat4(shaderProgram, "model_matrix", worldMatrix);
	glPointSize(4);
	glDrawElements(GL_POINTS, sphereVertices, GL_UNSIGNED_INT, 0);

	glBindVertexArray(sphereVAO);

	 partMatrix = translate(mat4(1.0f), vec3(10.0f, -5.0f, 0.0f)) * rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(3.0f, 3.0f, 3.0f));
	 worldMatrix = groupMatrix * partMatrix;

	SetUniformVec3(shaderProgram, "object_color", vec3(1.0, 1.0, 1.0));
	SetUniformMat4(shaderProgram, "model_matrix", worldMatrix);
	glPointSize(4);
	glDrawElements(GL_POINTS, sphereVertices, GL_UNSIGNED_INT, 0);

	//Draw the moon
	glBindVertexArray(sphereVAO);

	worldMatrix = translate(mat4(1.0f), vec3(-15.0f, 70.0f, -50.0f)) * scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f)) * rotate(mat4(1.0f), -15.0f, vec3(1.0f, 0.0f, 1.0f));

	SetUniformVec3(shaderProgram, "object_color", vec3(1.0, 1.0, 1.0));
	SetUniformMat4(shaderProgram, "model_matrix", worldMatrix);
	glBindTexture(GL_TEXTURE_2D, moonTextureId);
	glDrawElements(GL_TRIANGLES, sphereVertices, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//Generate the forest
void Ground::GenerateForest(int shaderProgram, vector<Tree>* treeList, vector<Bush>* bushList, vector<Rock>* rockList, GLuint rockTextureID, GLuint bushTextureID) {
	for (vector<Tree>::iterator it = treeList->begin(); it != treeList->end(); ++it) {
		it->Draw(shaderProgram);
	}
	for (vector<Bush>::iterator it = bushList->begin(); it != bushList->end(); ++it) {
		it->Draw(shaderProgram, bushTextureID);
	}
	glBindTexture(GL_TEXTURE_2D, rockTextureID);
	for (vector<Rock>::iterator it = rockList->begin(); it != rockList->end(); ++it) {
		it->Draw(shaderProgram);
	}
}

//Generate color based on height
vec3 Ground::generateColor(float height) {
	/*if (height >= 0.0f && height < 2.0f) {
		return vec3(0.5, 0.0, 0.0);
	}
	else if (height >= 2.0f && height < 3.2f) {
		return vec3(1.0, 0.0, 0.0);
	}
	else if (height >= 3.2f && height < 4.8f) {
		return vec3(1.0, 0.65, 0.0);
	}
	else if (height >= 4.8f && height < 6.4f) {
		return vec3(1.0, 1.0, 0.0);
	}
	else{
		return vec3(0.0, 1.0, 0.0);
	}*/

	if (height >= 0.0f && height < 1.0f) {
		return vec3(0.45, 0.46, 0.266);
	}
	else if (height >= 1.0f && height < 1.94f) {
		return vec3(0.42, 0.41, 0.235);
	}
	else if (height >= 1.94f && height < 2.88f) {
		return vec3(0.309, 0.33, 0.168);

	}
	else if (height >= 2.88f && height < 3.82f) {
		return vec3(0.32, 0.427, 0.19);
	}
	else {
		return vec3(0.25, 0.35, 0.15);
	}
}

// shader variable setters
void Ground::SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value)
{
	glUseProgram(shader_id);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void Ground::SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value)
{
	glUseProgram(shader_id);
	glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

//Setup model EBO
GLuint Ground::setupModelEBO(string path, int& vertexCount)
{
	vector<int> vertexIndices;
	//The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
							// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

							//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	//EBO setup
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	// Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

//Helper function
bool Ground::loadOBJ2(
	const char * path,
	std::vector<int> & vertexIndices,
	std::vector<glm::vec3> & temp_vertices,
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec2> & out_uvs) {

	std::vector<int> uvIndices, normalIndices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE * file;
	file = fopen(path, "r");
	if (!file) {
		printf("Impossible to open the file ! Are you in the right path ground ?\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			res = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);

			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			res = fscanf(file, "%f %f\n", &uv.x, &uv.y);
			if (res != 2) {
				printf("Missing uv information!\n");
			}
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			res = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			if (res != 3) {
				printf("Missing normal information!\n");
			}
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			char* getRes;
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			bool uv = true;
			bool norm = true;
			char line[128];
			getRes = fgets(line, 128, file);
			if (getRes == 0) {
				printf("incomplete face\n");
			}

			//vertex, uv, norm
			int matches = sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				//vertex, norm
				matches = sscanf(line, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					//vertex, uv
					matches = sscanf(line, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
					if (matches != 6) {
						//vertex
						matches = sscanf(line, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
						if (matches != 3) {
							printf("File can't be read by our simple parser. 'f' format expected: d/d/d d/d/d d/d/d || d/d d/d d/d || d//d d//d d//d\n");
							printf("Character at %ld", ftell(file));
							return false;
						}
						uv, norm = false;
					}
					else {
						norm = false;
					}
				}
				else {
					uv = false;
				}
			}
			vertexIndices.push_back(abs(vertexIndex[0]) - 1);
			vertexIndices.push_back(abs(vertexIndex[1]) - 1);
			vertexIndices.push_back(abs(vertexIndex[2]) - 1);
			if (norm) {
				normalIndices.push_back(abs(normalIndex[0]) - 1);
				normalIndices.push_back(abs(normalIndex[1]) - 1);
				normalIndices.push_back(abs(normalIndex[2]) - 1);
			}
			if (uv) {
				uvIndices.push_back(abs(uvIndex[0]) - 1);
				uvIndices.push_back(abs(uvIndex[1]) - 1);
				uvIndices.push_back(abs(uvIndex[2]) - 1);
			}
		}
		else {
			char clear[1000];
			char* getsRes = fgets(clear, 1000, file);
		}
	}
	if (normalIndices.size() != 0)
		out_normals.resize(temp_normals.size());
	if (uvIndices.size() != 0)
		out_uvs.resize(temp_normals.size());
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {
		int vi = vertexIndices[i];
		if (normalIndices.size() != 0) {
			int ni = normalIndices[i];
			out_normals[vi] = temp_normals[ni];
		}
		if (uvIndices.size() != 0 && i < uvIndices.size()) {
			int ui = uvIndices[i];
			out_uvs[vi] = temp_uvs[ui];
		}
	}

	return true;
}

