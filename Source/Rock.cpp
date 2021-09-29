#include "Rock.h"
#include "Ground.h"

/*
	For the rocks models, we used the obj files from this url.
	https://www.turbosquid.com/FullPreview/Index.cfm/ID/925665
*/
/* Default Constructor */
Rock::Rock()
{
	int height = rand() % 10 + 1;
	cout << "Rock Color " << height << endl;
	vec3 rockColor = generateColor();

	shape_x = 1.0f * (float)randomNumberFromOneTo(4);
	shape_z = 1.0f * (float)randomNumberFromOneTo(4);
	shape_y = 1.0f * (float)randomNumberFromOneTo(4);

	position = vec3(0.0f, 0.0f, 0.0f);
	rotation_angle = (float)(rand() % 360);
	scale_factor = vec3(1.0f, 1.0f, 1.0f);

	color_f = rockColor;

	cubeVAO = setupModelEBO(cubePath, cubeVertices);
}

//Constructor to init the position, rotation and scale
Rock::Rock(vec3 mPosition, float mRotate, vec3 mScale) : position(mPosition), rotation_angle(mRotate), scale_factor(mScale)
{
	srand((unsigned)time(NULL));
	int height = rand() % 10 + 1;

	shape_x = 1.0f * (float)randomNumberFromOneTo(4);
	shape_z = 1.0f * (float)randomNumberFromOneTo(4);
	shape_y = 1.0f * (float)randomNumberFromOneTo(4);

	cout << "Rock Color " << height << endl;
	vec3 rockColor = generateColor();
	color_f = rockColor;

	cubeVAO = setupModelEBO(cubePath, cubeVertices);
}

//Move Location of Rock
void Rock::moveLocation(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

//Scale Rock
void Rock::scaleRock(float scale)
{
	scale_factor += scale;
}

vec3 Rock::generateColor() {
	return vec3(0.26f, 0.26f, 0.26f);
}

//Draw Rock
void Rock::Draw(int shaderProgram)
{
	/* Base */
	glBindVertexArray(cubeVAO);

	mat4 partMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(shape_x, shape_y, shape_z));
	mat4 groupMatrix = translate(mat4(1.0f), position) * rotate(mat4(1.0f), rotation_angle, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), scale_factor);
	mat4 worldMatrix = groupMatrix * partMatrix;

	SetUniformVec3(shaderProgram, "object_color", color_f);
	SetUniformMat4(shaderProgram, "model_matrix", worldMatrix);

	glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
}

// shader variable setters
void Rock::SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value)
{
	glUseProgram(shader_id);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void Rock::SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value)
{
	glUseProgram(shader_id);
	glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

GLuint Rock::setupModelEBO(string path, int& vertexCount)
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

	glBindVertexArray(0);
	// Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

bool Rock::loadOBJ2(
	const char* path,
	std::vector<int>& vertexIndices,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& out_normals,
	std::vector<glm::vec2>& out_uvs) {

	std::vector<int> uvIndices, normalIndices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE* file;
	int count = 0;
	file = fopen(path, "r");
	while(!file)
	{
		file = fopen(path, "r");
		count++;
		if(count = 10) {
			printf("Impossible to open the file ! Are you in the right path Rock?\n");
			getchar();
			return false;
		}
	}
	file = fopen(path, "r");
	if (!file) {
		printf("Impossible to open the file ! Are you in the right path Rock?\n");
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
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
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

int Rock::randomNumberFromOneTo(int max)
{
	return ((rand() % (max - 1)) + 1);
}