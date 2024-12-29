#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <string>

static GLFWwindow* window;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// OpenGL camera view parameters
static glm::vec3 eye_center;
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);

// View control 
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 600.0f;
static float viewDistanceSkybox = 1.0f;

static GLuint LoadTextureTileBox(const char* texture_file_path) {
	int w, h, channels;
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (img) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture " << texture_file_path << std::endl;
	}
	stbi_image_free(img);

	return texture;
}

static GLuint LoadTextureTileBoxSkybox(const char* texture_file_path) {
	int w, h, channels;
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load the texture image data
	if (img) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cerr << "Failed to load texture " << texture_file_path << std::endl;
	}
	stbi_image_free(img);
	return texture;
}

struct Skybox {
	glm::vec3 positionSkybox;		// positionSkybox of the box 
	glm::vec3 scaleSkybox;		// Size of the box in each axis

	GLfloat vertex_buffer_data_skybox[72] = {
		// Front face
		-1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f, 1.0f,

		// Back face
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,

		 // Left face
		 -1.0f, -1.0f, -1.0f,
		 -1.0f, -1.0f,  1.0f,
		 -1.0f,  1.0f,  1.0f,
		 -1.0f,  1.0f, -1.0f,

		 // Right face
		  1.0f, -1.0f,  1.0f,
		  1.0f, -1.0f, -1.0f,
		  1.0f,  1.0f, -1.0f,
		  1.0f,  1.0f,  1.0f,

		  // Top face
		  -1.0f,  1.0f,  1.0f,
		   1.0f,  1.0f,  1.0f,
		   1.0f,  1.0f, -1.0f,
		  -1.0f,  1.0f, -1.0f,

		  // Bottom face
		  -1.0f, -1.0f, -1.0f,
		   1.0f, -1.0f, -1.0f,
		   1.0f, -1.0f,  1.0f,
		  -1.0f, -1.0f,  1.0f,
	};

	GLfloat color_buffer_data_skybox[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data_skybox[36] = {		// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};

	GLfloat uv_buffer_data_skybox[48] = {
		// +X Face (pos X)
		0.25f, 0.666f,  // Top-right
		0.0f,  0.666f,  // Top-left
		0.0f,  0.333f,  // Bottom-left
		0.25f, 0.333f,  // Bottom-right

		// -X Face (neg X)
		0.75f, 0.666f,  // Top-right
		0.5f,  0.666f,  // Top-left
		0.5f,  0.333f,  // Bottom-left
		0.75f, 0.333f,  // Bottom-right

		// +Z Face (pos Z)
		0.5f,  0.666f,  // Top-right
		0.25f, 0.666f,  // Top-left
		0.25f, 0.333f,  // Bottom-left
		0.5f,  0.333f,  // Bottom-right

		// -Z Face (neg Z)
		1.0f,  0.666f,  // Top-right
		0.75f, 0.666f,  // Top-left
		0.75f, 0.333f,  // Bottom-left
		1.0f,  0.333f,  // Bottom-right

		// -Y Face (neg Y, Bottom)
		0.25f, 0.333f,  // Top-left
		0.25f, 0.0f,    // Bottom-left
		0.5f,  0.0f,    // Bottom-right
		0.5f,  0.333f,  // Top-right

		// +Y Face (pos Y, Top)
		0.5f,  0.666f,
		0.5f,  1.0f,
		0.25f, 1.0f,
		0.25f, 0.666f
	};

	// OpenGL buffers
	GLuint vertexArrayIDskybox;
	GLuint vertexBufferIDskybox;
	GLuint indexBufferIDskybox;
	GLuint colorBufferIDskybox;
	GLuint uvBufferIDskybox;
	GLuint textureIDskybox;

	// Shader variable IDs
	GLuint mvpMatrixIDskybox;
	GLuint textureSamplerIDskybox;
	GLuint programIDskybox;

	void initialize(glm::vec3 positionSkybox, glm::vec3 scaleSkybox) {
		this->positionSkybox = positionSkybox;
		this->scaleSkybox = scaleSkybox;

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayIDskybox);
		glBindVertexArray(vertexArrayIDskybox);

		// Create a vertex buffer object to store the vertex data		
		glGenBuffers(1, &vertexBufferIDskybox);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIDskybox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_skybox), vertex_buffer_data_skybox, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &colorBufferIDskybox);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferIDskybox);
		for (int i = 0; i < 72; ++i) color_buffer_data_skybox[i] = 1.0f;
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_skybox), color_buffer_data_skybox, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the UV data 
		glGenBuffers(1, &uvBufferIDskybox);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferIDskybox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data_skybox), uv_buffer_data_skybox, GL_STATIC_DRAW);
		textureIDskybox = LoadTextureTileBoxSkybox("../../../lab2/sky.png");

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferIDskybox);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferIDskybox);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_skybox), index_buffer_data_skybox, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		programIDskybox = LoadShadersFromFile("../../../lab2/sky.vert", "../../../lab2/sky.frag");
		if (programIDskybox == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixIDskybox = glGetUniformLocation(programIDskybox, "MVP");
		textureSamplerIDskybox = glGetUniformLocation(programIDskybox, "textureSampler");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programIDskybox);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIDskybox);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferIDskybox);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferIDskybox);

		// Model transform
		glm::mat4 modelMatrix = glm::mat4();
		modelMatrix = glm::scale(modelMatrix, scaleSkybox);

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixIDskybox, 1, GL_FALSE, &mvp[0][0]);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferIDskybox);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Set textureSampler to use texture unit 0 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureIDskybox);
		glUniform1i(textureSamplerIDskybox, 0);

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//glDisableVertexAttribArray(2);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferIDskybox);
		glDeleteBuffers(1, &colorBufferIDskybox);
		glDeleteBuffers(1, &indexBufferIDskybox);
		glDeleteVertexArrays(1, &vertexArrayIDskybox);
		//glDeleteBuffers(1, &uvBufferIDskybox);
		//glDeleteTextures(1, &textureIDskybox);
		glDeleteProgram(programIDskybox);
	}
};

// Global Shader Program ID
GLuint globalProgramID;

void static initializeShaders() {
	globalProgramID = LoadShadersFromFile("../../../lab2/box.vert", "../../../lab2/box.frag");
	if (globalProgramID == 0) {
		std::cerr << "Failed to load shaders." << std::endl;
		exit(EXIT_FAILURE);
	}
}

void static cleanupShaders() {
	glDeleteProgram(globalProgramID);
}

struct Building {
	glm::vec3 position;		// Position of the box 
	glm::vec3 scale;		// Size of the box in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face 
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face 
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};

	GLfloat uv_buffer_data[48] = {
		// Front 
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		// Back 
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Left 
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Right 
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Top - we do not want texture the top 
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,

		// Bottom - we do not want texture the bottom 
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
	};

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	void initialize(glm::vec3 position, glm::vec3 scale) {
		this->position = position;
		this->scale = scale;

		// List of texture file paths
		const std::string textureFiles[6] = {
			"../../../lab2/facade0.jpg",
			"../../../lab2/facade1.jpg",
			"../../../lab2/facade2.jpg",
			"../../../lab2/facade3.jpg",
			"../../../lab2/facade4.jpg",
			"../../../lab2/facade5.jpg"
		};

		// Randomly select a texture from the list
		int textureIndex = rand() % 6;
		textureID = LoadTextureTileBox(textureFiles[textureIndex].c_str());

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data		
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		for (int i = 0; i < 72; ++i) color_buffer_data[i] = 1.0f;
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		for (int i = 0; i < 24; ++i) uv_buffer_data[2 * i + 1] *= 5;

		// Create a vertex buffer object to store the UV data 
		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Use the global shader program
		mvpMatrixID = glGetUniformLocation(globalProgramID, "MVP");
		textureSamplerID = glGetUniformLocation(globalProgramID, "textureSampler");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(globalProgramID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		modelMatrix = glm::scale(modelMatrix, scale);

		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteTextures(1, &textureID);
	}
};

std::vector<Building> buildings;
Skybox skybox;

int main(void)
{
	// Seed the random number generator with the current time
	srand(static_cast<unsigned>(time(0)));

	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Lab 2", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	glClearColor(0.68f, 0.85f, 0.90f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	initializeShaders();

	skybox.initialize(glm::vec3(0, 0, 0), glm::vec3(30, 30, 30));

	// Generate buildings in a new pattern without the middle column
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			// Skip the middle column
			if (j == 2) continue;

			Building b;

			// Create a height gradient and random variation
			float baseHeight = 50.0f + (4 - abs(2 - j)) * 20.0f;
			float heightVariation = static_cast<float>(rand() % 21) - 10.0f;
			float randomHeight = baseHeight + heightVariation;

			glm::vec3 scale(16.0f, randomHeight, 16.0f);

			// Adjust position for a more scattered layout
			glm::vec3 position(i * 60.0f - 120.0f, scale.y / 2.0f - 50.0f, j * 60.0f - 120.0f);

			b.initialize(position, scale);
			buildings.push_back(b);
		}
	}

	// Camera setup
	eye_center.y = 100.0f; // Adjust this value based on the average height of buildings
	eye_center.x = viewDistance * cos(viewAzimuth);
	eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix, skyboxViewMatrix;
	glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 1000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Recalculate the camera view matrix
		viewMatrix = glm::lookAt(eye_center, lookat, up);
		// View matrix for the skybox removes translation
		skyboxViewMatrix = glm::mat4(glm::mat3(glm::lookAt(eye_center, lookat, up)));
		// Calculate VP matrices
		glm::mat4 skyboxVP = projectionMatrix * skyboxViewMatrix;
		glm::mat4 buildingsVP = projectionMatrix * viewMatrix;

		// Render the skybox first
		skybox.render(skyboxVP);

		// Render the buildings next
		for (auto& building : buildings) {
			building.render(buildingsVP);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (!glfwWindowShouldClose(window));
	skybox.cleanup();
	for (auto& building : buildings) {
		building.cleanup();
	}
	cleanupShaders();
	glfwTerminate();

	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		viewAzimuth = 0.f;
		viewPolar = 0.f;
		eye_center.y = viewDistance * cos(viewPolar);
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
		std::cout << "Reset." << std::endl;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar -= 0.1f;
		eye_center.y = viewDistance * cos(viewPolar);
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar += 0.1f;
		eye_center.y = viewDistance * cos(viewPolar);
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth -= 0.1f;
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth += 0.1f;
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}