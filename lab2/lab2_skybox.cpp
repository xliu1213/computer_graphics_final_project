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
static float viewDistanceSkybox = 1.0f;

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
		programIDskybox = LoadShadersFromFile("../../../lab2/box.vert", "../../../lab2/box.frag");
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

int main(void)
{
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
	glDisable(GL_CULL_FACE);

	Skybox skybox;
	skybox.initialize(glm::vec3(0, 0, 0), glm::vec3(30, 30, 30));

	// Camera setup
	eye_center.y = viewDistanceSkybox * cos(viewPolar);
	eye_center.x = viewDistanceSkybox * cos(viewAzimuth);
	eye_center.z = viewDistanceSkybox * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix;
	glm::float32 FoV = 90;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 1000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		skybox.render(vp);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	skybox.cleanup();

	// Close OpenGL window and terminate GLFW
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
		eye_center.y = viewDistanceSkybox * cos(viewPolar);
		eye_center.x = viewDistanceSkybox * cos(viewAzimuth);
		eye_center.z = viewDistanceSkybox * sin(viewAzimuth);
		std::cout << "Reset." << std::endl;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar -= 0.1f;
		eye_center.y = viewDistanceSkybox * cos(viewPolar);
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar += 0.1f;
		eye_center.y = viewDistanceSkybox * cos(viewPolar);
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth -= 0.1f;
		eye_center.x = viewDistanceSkybox * cos(viewAzimuth);
		eye_center.z = viewDistanceSkybox * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth += 0.1f;
		eye_center.x = viewDistanceSkybox * cos(viewAzimuth);
		eye_center.z = viewDistanceSkybox * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
