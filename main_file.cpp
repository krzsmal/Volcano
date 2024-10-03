#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "myModel.h"
#include <vector>
#include <random>
#include <chrono>

// For calculationg FPS
// std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
// int frameCount = 0;
// float fps = 0.0f;

// Initialization of random number generator
std::random_device rd;
std::mt19937 gen(rd());

// Buffers
GLuint alphaVBO;
GLuint translationVBO;
GLuint rotationVBO;
GLuint scaleVBO;

// For collision detection
bool useCollisionVertexPosition = true;
std::vector<glm::vec4> transformedVolcanoVerts;
std::vector<glm::mat4> collM;
std::vector<float> collA;

// Lighting settings
ShaderProgram* sp; // Pointer to shader program
bool useLight1 = true;
bool useLight2 = false;
glm::vec4 lightPos1 = glm::vec4(-40, 40, 0, 1); // Position of the first light source in world space (sun)
glm::vec4 lightPos2 = glm::vec4(0, 13, 0, 1); // Position of the second light source in world space (lava)
glm::vec4 lightColor1 = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
glm::vec4 lightColor2 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
std::uniform_real_distribution<> redLightDist(0.5f, 1.0f);
float timeSinceLastRedChange = 0.0f;
const float redLightChangeInterval = 0.1f;

// Camera movement settings
float aspectRatio = 1;
float camera_move_speed_z = 0.0f;
float camera_move_speed_x = 0.0f;
float camera_move_speed_y = 0.0f;
float camera_speed = 15.0f; // Speed for keyboard movement
float lastX, lastY;
bool firstMouse = true;
float yaw = -90.0f; // Rotation left-right, set to -90 to initially look forward at the volcano (in the -z direction)
float pitch = 0.0f; // Rotation up-down
float sensitivity = 0.1f; // Mouse sensitivity
bool cursorVisible = false;
glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
bool cameraShakeOn = true;
bool cameraShake = false;
float shakeDuration = 0.25f;
float shakeElapsed = 0.0f;
float shakeMagnitude = 0.5f;

// Model declarations
myModel* volcanoModel;
myModel* stoneModel;
myModel* smokeModel;
myModel* lavaModel;
myModel* deadTreeModel;
myModel* oceanModel;
myModel* sunModel;
myModel* collider;
myModel* treePositions;

// Stone settings
bool stonesOn = true;
std::uniform_int_distribution<> stoneSpawnNumber(1, 5);
std::uniform_real_distribution<> stoneSpeedDist(11.0f, 15.0f);
std::uniform_real_distribution<> stoneLaunchAngleDist(70.0f, 80.0f);
std::uniform_real_distribution<> stoneRotationAgnleDist(0.0f, 360.0f);
std::uniform_real_distribution<> stoneScaleDist(0.5f, 1.5f);
const glm::vec3 stoneInitialPosition(0.0f, 6.5f, 0.0f);
const glm::vec3 gravity(0.0f, -9.81f, 0.0f);
const float stoneLaunchInterval = 3.0f;
//float timeSinceLastStoneLaunch = 0.0f;
float timeSinceLastStoneLaunch = stoneLaunchInterval - 1.0f;
const float stoneDisappearTime = 5.0f;
const float stoneTimeToStartDisappearing = 30.0f;

// Smoke settings
bool smokeOn = true;
const glm::vec3 smokeInitialPosition(0.0f, 6.5f, 0.0f);
const float smokeMaxLifetime = 6.0f;
float timeSinceLastSmokeSpawn = 0.0f;
const float smokeSpawnInterval = 0.1f;
const int smokeSpawnNumber = 5;
std::uniform_real_distribution<> smokeXDIst(-0.75f, 0.75f);
std::uniform_real_distribution<> smokeYDIst(-0.75f, 0.75f);
std::uniform_real_distribution<> smokeZDIst(-0.75f, 0.75f);
std::uniform_real_distribution<> smokeRotationAngleDist(-10.0f, 10.0f);
std::uniform_real_distribution<> smokeScaleDist(0.5f, 1.5f);

// Lava settings
bool lavaOn = true;
const glm::vec3 lavaInitialPosition(0.0f, 6.5f, 0.0f);
const float lavaMaxLifetime = 0.3f;
float timeSinceLastLavaSpawn = 0.0f;
const float lavaSpawnInterval = 0.05f;
const int lavaSpawnNumber = 10;
const float lavaTimeToStartDisappearing = 1.5f;
std::uniform_real_distribution<> lavaXDIst(-0.5f, 0.5f);
std::uniform_real_distribution<> lavaYDIst(-0.5f, 0.5f);
std::uniform_real_distribution<> lavaZDIst(-0.5f, 0.5f);
std::uniform_real_distribution<> lavaRotationAngleDist(0.0f, 360.0f);
std::uniform_real_distribution<> lavaScaleDist(0.5f, 3.5f);
std::uniform_real_distribution<> lavaDisappearTime(0.1f, 0.8f);

// Eruption settings
bool eruptionOn = true;
float timeSinceLastEruption = 0.0f;
const float eruptionInterval = 5.0f;
const float eruptionDuration = 10.0f;

// Ocean
std::vector<glm::vec3> oceanPositions;
std::vector<glm::vec3> ocenRotationAngles;
std::vector<glm::vec3> oceanScales;
std::vector<float> oceanAlphas;

// Dead trees
std::vector<glm::vec3> deadTreePositions;
std::vector<glm::vec3> deadTreeRotationAngles;
std::vector<glm::vec3> deadTreeScales;
std::vector<float> deadTreeAlphas;
std::uniform_real_distribution<> treeStandingAngleDist(-10.0f, 10.0f);
std::uniform_real_distribution<> treeScaleDist(0.8f, 1.2f);
std::vector<glm::vec4> allTreePositions;

// Function to rotate collision vertices by a given angle for stones
std::vector<glm::vec4> rotateCollisionVerts(float angle) {
	std::vector<glm::vec4> rotatedCollisionVerts;
	glm::mat4 matrix = glm::mat4(1.0f);
	matrix = glm::rotate(matrix, angle, glm::vec3(0.0f, -1.0f, 0.0f));

	for (const auto& vert : transformedVolcanoVerts) {
		rotatedCollisionVerts.push_back(matrix * vert);
	}
	return rotatedCollisionVerts;
}

// Structure to represent a stone
struct Stone {
	glm::vec3 position;
	float elapsedTime;
	bool stopped;
	float initialSpeed;
	float launchAngle;
	float rotationAngle;
	float timeSinceStopped;
	float alpha;
	float scale;
	std::vector<glm::vec4> collisionVerts;

	Stone() :
		position(stoneInitialPosition),
		elapsedTime(0.0f),
		stopped(false),
		initialSpeed(stoneSpeedDist(gen)),
		launchAngle(glm::radians(stoneLaunchAngleDist(gen))),
		rotationAngle(glm::radians(stoneRotationAgnleDist(gen))),
		timeSinceStopped(0.0f),
		alpha(1.0f),
		scale(stoneScaleDist(gen)),
		collisionVerts(rotateCollisionVerts(rotationAngle)) {}
};

// Vector for stones
std::vector<Stone> stoneVector;

// Collision detection between a stone and its collision vertices
bool checkCollision(glm::vec3& stonePosition, const std::vector<glm::vec4>& volcanoVerts) {
	glm::vec3 nearest{};
	float minDistance;
	bool isFirstIteration = true;

	for (const auto& vert : volcanoVerts) {
		glm::vec3 vertexPosition(vert);
		float distance = glm::distance(stonePosition, vertexPosition);

		if (isFirstIteration) {
			minDistance = distance;
			nearest = vertexPosition;
			isFirstIteration = false;
		}
		else {
			if (distance <= minDistance) {
				minDistance = distance;
				nearest = vertexPosition;
			}
		}

	}
	if (stonePosition.y <= nearest.y) {
		if (useCollisionVertexPosition) {
			stonePosition = nearest; // using the coordinates of the nearest vertex ensures that the stone won't sink below the island
		}
		return true;
	}
	return false;
}

// Updating the stone's position
void updateStone(Stone& stone, float deltaTime) {
	if (stone.stopped) {
		stone.timeSinceStopped += deltaTime;

		// is it time for the stone to start disappearing
		if (stone.timeSinceStopped >= stoneTimeToStartDisappearing) {
			stone.alpha = 1.0f - ((stone.timeSinceStopped - stoneTimeToStartDisappearing) / (stoneDisappearTime));

			// removing the object from the vector
			if (stone.alpha < 0.0f) {
				stone.alpha = 0.0f;
				auto it = std::find_if(stoneVector.begin(), stoneVector.end(), [&](const Stone& s) {
					return &s == &stone;
					});

				if (it != stoneVector.end()) {
					stoneVector.erase(it);
				}
			}
		}
		return;
	}

	// projectile motion
	stone.elapsedTime += deltaTime;
	float vx = stone.initialSpeed * cos(stone.launchAngle); // horizontal speed of the stone
	float vy = stone.initialSpeed * sin(stone.launchAngle); // vertical speed of the stone
	stone.position = stoneInitialPosition + glm::vec3(vx * stone.elapsedTime, vy * stone.elapsedTime, 0.0f) + 0.5f * gravity * stone.elapsedTime * stone.elapsedTime; // calculating new position

	// collision detection
	if (checkCollision(stone.position, stone.collisionVerts) || stone.position.y <= -20.0f) {
		stone.stopped = true;

		cameraShake = true;
		shakeElapsed = 0.0f;
	}
}

// Structure for representing smoke particles
struct Smoke {
	glm::vec3 position;
	glm::vec3 initialPosition;
	float elapsedTime;
	float speed;
	float rotationAngle;
	float alpha;
	float scale;

	Smoke() :
		position(glm::vec3(0, 0, 0)),
		initialPosition(glm::vec3(smokeInitialPosition.x + smokeXDIst(gen), smokeInitialPosition.y + smokeYDIst(gen), smokeInitialPosition.z + smokeZDIst(gen))),
		//initialPosition(smokeInitialPosition),
		elapsedTime(0.0f),
		speed(2.0f),
		rotationAngle(glm::radians(smokeRotationAngleDist(gen))),
		alpha(1.0f),
		scale(smokeScaleDist(gen)) {}
};

// vector for smoke particles
std::vector<Smoke> smokeVector;

// function to update smoke particles
void updateSmoke(Smoke& smoke, float deltaTime) {
	smoke.elapsedTime += deltaTime;

	float adjustedTime = smoke.elapsedTime * smoke.speed;
	
	// calculating position
	float x = smoke.initialPosition.x + adjustedTime;
	float y = smoke.initialPosition.y + 2.5 * std::pow(adjustedTime, 1 / 3.);
	float z = smoke.initialPosition.z + sin(2 * adjustedTime) / 3;

	// updating position and alpha
	smoke.position = glm::vec3(x, y, z);
	smoke.alpha = 1.0f - (smoke.elapsedTime / smokeMaxLifetime);

	// removing the object from the vecto
	if (smoke.alpha < 0.0f) {
		smoke.alpha = 0.0f;
		auto it = std::find_if(smokeVector.begin(), smokeVector.end(), [&](const Smoke& s) {
			return &s == &smoke;
			});

		if (it != smokeVector.end()) {
			smokeVector.erase(it);
		}
	}
}

// Structure for representing lava particles
struct Lava {
	glm::vec3 position;
	glm::vec3 initialPosition;
	float elapsedTime;
	float speed;
	float rotationAngle;
	float alpha;
	float scale;
	float diappearTime;

	Lava() :
		position(glm::vec3(0, 0, 0)),
		initialPosition(glm::vec3(lavaInitialPosition.x + lavaXDIst(gen), lavaInitialPosition.y + lavaYDIst(gen), lavaInitialPosition.z + lavaZDIst(gen))),
		elapsedTime(0.0f),
		speed(2.0f),
		rotationAngle(glm::radians(lavaRotationAngleDist(gen))),
		alpha(1.0f),
		scale(lavaScaleDist(gen)),
		diappearTime(lavaDisappearTime(gen)) {}
};

// vector for lava particles
std::vector<Lava> lavaVector;

// function to update lava particles
void updateLava(Lava& lava, float deltaTime) {
	lava.elapsedTime += deltaTime;

	float adjustedTime = lava.elapsedTime * lava.speed;

	// calculating position
	float x = lava.initialPosition.x + adjustedTime;
	float z = lava.initialPosition.z;

	float a = -1.0f;
	float b = 2.0f;
	float c = 4.0f;
	float y = lava.initialPosition.y + a * (adjustedTime - b) * (adjustedTime - b) + c;

	lava.position = glm::vec3(x, y, z);


	// fading the particle over time
	if (lava.elapsedTime >= lavaTimeToStartDisappearing) {
		lava.alpha = 1.0f - ((lava.elapsedTime - lavaTimeToStartDisappearing) / (lava.diappearTime));

		// removing the element from the vector
		if (lava.alpha < 0.0f) {
			lava.alpha = 0.0f;
			auto it = std::find_if(lavaVector.begin(), lavaVector.end(), [&](const Lava& s) {
				return &s == &lava;
				});

			if (it != lavaVector.end()) {
				lavaVector.erase(it);
			}
		}
	}
}

// function for applying camera shake
void applyCameraShake(glm::vec3& camera_pos) {
	if (cameraShake) {
		std::uniform_real_distribution<float> shakeDist(-shakeMagnitude, shakeMagnitude);
		camera_pos.x += shakeDist(gen);
		camera_pos.y += shakeDist(gen);
		camera_pos.z += shakeDist(gen);
	}
}

//Error processing callback procedure
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

// Key callback procedure
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W) {
			camera_move_speed_z = camera_speed;
		}
		if (key == GLFW_KEY_S) {
			camera_move_speed_z = -camera_speed;
		}
		if (key == GLFW_KEY_A) {
			camera_move_speed_x = -camera_speed;
		}
		if (key == GLFW_KEY_D) {
			camera_move_speed_x = camera_speed;
		}
		if (key == GLFW_KEY_SPACE) {
			camera_move_speed_y = camera_speed;
		}
		if (key == GLFW_KEY_LEFT_CONTROL) {
			camera_move_speed_y = -camera_speed;
		}
		if (key == GLFW_KEY_ESCAPE) {
			cursorVisible = !cursorVisible;
			if (cursorVisible) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // show the cursor
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide the cursor
				firstMouse = true;
			}
		}
		if (key == GLFW_KEY_M) {
			smokeOn = !smokeOn;
		}
		if (key == GLFW_KEY_N) {
			stonesOn = !stonesOn;
		}
		if (key == GLFW_KEY_B) {
			cameraShakeOn = !cameraShakeOn;
		}
		if (key == GLFW_KEY_L) {
			lavaOn = !lavaOn;
			useLight2 = !useLight2;
		}
		if (key == GLFW_KEY_1) {
			useLight1 = !useLight1;
		}
		if (key == GLFW_KEY_2) {
			useLight2 = !useLight2;
		}
		if (key == GLFW_KEY_E) {
			eruptionOn = !eruptionOn;
		}
		if (key == GLFW_KEY_C) {
			useCollisionVertexPosition = !useCollisionVertexPosition;
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W || key == GLFW_KEY_S) {
			camera_move_speed_z = 0.0f;
		}
		if (key == GLFW_KEY_A || key == GLFW_KEY_D) {
			camera_move_speed_x = 0.0f;
		}
		if (key == GLFW_KEY_SPACE || key == GLFW_KEY_LEFT_CONTROL) {
			camera_move_speed_y = 0.0f;
		}
	}
}

// Mouse movement callback procedure
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (cursorVisible) return; // cursor capturing is disabled

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// calculating mouse offset
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // inverted because Y coordinates on the screen increase downward
	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset; // left-right rotation
	pitch += yoffset; // up-down rotation

	// limiting the pitch angle to prevent the screen from flipping upside down
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	// determining and updating the camera direction based on yaw and pitch
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera_front = glm::normalize(front);
}

// function handling window resize to appropriately scale objects
void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

// Initialization
void initOpenGLProgram(GLFWwindow* window) {

	glClearColor(0.333, 0.808, 1.0, 1); // setting background color
	glEnable(GL_DEPTH_TEST); // enables depth test (so objects correctly obscure each other)
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // capture/hide cursor
	glEnable(GL_BLEND); // enable blending/transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // enable blending/transparency

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
	
	// generating buffers
	glGenBuffers(1, &alphaVBO);
	glGenBuffers(1, &translationVBO);
	glGenBuffers(1, &rotationVBO);
	glGenBuffers(1, &scaleVBO);

	// creating objects
	volcanoModel = new myModel("Wyspa_2.fbx", "Wyspa_2.png");
	stoneModel = new myModel("Wulkan_col_Skala.fbx", "Skala_diffuse.png");
	smokeModel = new myModel("Wulkan_col_Skala.fbx", "papier.png");
	lavaModel = new myModel("Lava.fbx", "Lava_rock2.png");
	deadTreeModel = new myModel("Drzewo.fbx", "Drzewo.png");
	sunModel = new myModel("Sonce.fbx", "Sonce.png");
	oceanModel = new myModel("Ocean_plaski.fbx", "ocean.png");
	collider = new myModel("1kv.fbx", "ocean.png"); // a simplified island/volcano model with reduced vertices count
	treePositions = new myModel("treePositions1.fbx", "ocean.png"); // similar to the collider but with more vertices

	std::cout << "Vocano:" << volcanoModel->verts.size() << std::endl;
	std::cout << "Stone:" << stoneModel->verts.size() << std::endl;
	std::cout << "Smoke:" << smokeModel->verts.size() << std::endl;
	std::cout << "Dead tree:" << deadTreeModel->verts.size() << std::endl;
	std::cout << "Ocean:" << oceanModel->verts.size() << std::endl;
	std::cout << "Collider:" << collider->verts.size() << std::endl;
	std::cout << "Tree positions:" << treePositions->verts.size() << std::endl;

	// creating a model matrix for the collider to match the volcano matrix
	glm::mat4 collisionMatrix = glm::mat4(1.0f);
	collisionMatrix = glm::translate(collisionMatrix, glm::vec3(0.0f, -2.3f, 0.0f));
	collisionMatrix = glm::rotate(collisionMatrix, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
	collisionMatrix = glm::scale(collisionMatrix, glm::vec3(10.0f, 10.0f, 10.0f));

	transformedVolcanoVerts.reserve(collider->verts.size());
	allTreePositions.reserve(collider->verts.size());

	// Calculating and adding transformed vertices that are above the ocean level
	for (const auto& vert : collider->verts) {
		glm::vec4 transformedVert = collisionMatrix * vert;
		if (transformedVert.y >= -4.0f) {
			transformedVolcanoVerts.push_back(transformedVert);
		}
	}

	for (const auto& vert : treePositions->verts) {
		glm::vec4 transformedVert = collisionMatrix * vert;
		if (transformedVert.y >= -4.0f) {
			allTreePositions.push_back(transformedVert);
		}
	}

	transformedVolcanoVerts.shrink_to_fit();
	allTreePositions.shrink_to_fit();

	std::cout << "Collision verts:" << transformedVolcanoVerts.size() << std::endl;
	std::cout << "All tree positions:" << allTreePositions.size() << std::endl;

	// Generating instance data for the ocean
	for (float i = 0.0f; i < 30.0f; i += 2.0f)
	{
		for (float j = 0.0f; j < 30.0f; j += 2.0f)
		{
			oceanPositions.push_back(glm::vec3(-60.0f + 5 * i, -4.0f, -60.0f + 5 * j));
			ocenRotationAngles.push_back(glm::vec3(glm::radians(-90.0f), glm::radians(0.0f), glm::radians(0.0f)));
			oceanScales.push_back(glm::vec3(5.0f, 5.0f, 1.0f));
			oceanAlphas.push_back(1.0f);
		}

	}

	// Randomizing tree positions
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	for (int i = 0; i < 30; ++i) {
		int randomIndex = std::rand() % allTreePositions.size();
		deadTreePositions.push_back(glm::vec3(allTreePositions[randomIndex]));
		deadTreeRotationAngles.push_back(glm::vec3(glm::radians(-90.0f + treeStandingAngleDist(gen)), glm::radians(treeStandingAngleDist(gen)), glm::radians(stoneRotationAgnleDist(gen))));
		deadTreeScales.push_back(glm::vec3(0.15f * treeScaleDist(gen)));
		deadTreeAlphas.push_back(1.0f);
	}

	// lastTime = std::chrono::high_resolution_clock::now();
}

// Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	delete sp;
	delete volcanoModel;
	delete stoneModel;
	delete smokeModel;
	delete lavaModel;
	delete deadTreeModel;
	delete oceanModel;
	delete sunModel;
	delete collider;
	delete treePositions;
	glDeleteBuffers(1, &alphaVBO);
	glDeleteBuffers(1, &translationVBO);
	glDeleteBuffers(1, &rotationVBO);
	glDeleteBuffers(1, &scaleVBO);
}

// Drawing the model
void drawModel(myModel* model, ShaderProgram* sp, glm::mat4 P, glm::mat4 V, const std::vector<glm::vec3>& instanceTranslations, const std::vector<glm::vec3>& instanceRotationAngles, const std::vector<glm::vec3>& instanceScales, const std::vector<float>& instanceAlphas, bool useShading, bool rotateFirst) {
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	
	glUniform1i(sp->u("useShading"), useShading); // whether the object should be shaded
	glUniform1i(sp->u("rotateFirst"), rotateFirst); // rotate or translate first

	// Ustawianie atrybutów wierzcho³ków modelu (wierzcho³ki, wspó³rzêdne tekstury, normale)
	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, model->verts.data());

	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, model->texCoords.data());

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, model->norms.data());

	// Preparing texture to be used in the shader
	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model->tex);

	// Translation buffer settings
	glBindBuffer(GL_ARRAY_BUFFER, translationVBO); // setting translationVBO as the currently used buffer
	glBufferData(GL_ARRAY_BUFFER, instanceTranslations.size() * sizeof(glm::vec3), instanceTranslations.data(), GL_DYNAMIC_DRAW); // sending data to the buffer
	glEnableVertexAttribArray(sp->a("instanceTranslations")); // enabling attribute in the shader
	glVertexAttribPointer(sp->a("instanceTranslations"), 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // setting pointer to data in the buffer
	glVertexAttribDivisor(sp->a("instanceTranslations"), 1); // attribute will update for each instance

	// Rotation buffer settings
	glBindBuffer(GL_ARRAY_BUFFER, rotationVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceRotationAngles.size() * sizeof(glm::vec3), instanceRotationAngles.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(sp->a("instanceRotationAngles"));
	glVertexAttribPointer(sp->a("instanceRotationAngles"), 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribDivisor(sp->a("instanceRotationAngles"), 1);

	// Scale buffer settings
	glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceScales.size() * sizeof(glm::vec3), instanceScales.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(sp->a("instanceScale"));
	glVertexAttribPointer(sp->a("instanceScale"), 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribDivisor(sp->a("instanceScale"), 1);

	// Alpha transparency buffer settings
	glBindBuffer(GL_ARRAY_BUFFER, alphaVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceAlphas.size() * sizeof(float), instanceAlphas.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(sp->a("instanceAlpha"));
	glVertexAttribPointer(sp->a("instanceAlpha"), 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glVertexAttribDivisor(sp->a("instanceAlpha"), 1);

	// Drawing the model
	glDrawElementsInstanced(GL_TRIANGLES, model->indices.size(), GL_UNSIGNED_INT, model->indices.data(), instanceTranslations.size());

	// Disabling arrays
	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("texCoord0"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("instanceTranslations"));
	glDisableVertexAttribArray(sp->a("instanceRotationAngle"));
	glDisableVertexAttribArray(sp->a("instanceScale"));
	glDisableVertexAttribArray(sp->a("instanceAlpha"));

	glBindBuffer(GL_ARRAY_BUFFER, 0); // ending the work with buffers
}

void drawScene(GLFWwindow* window, glm::mat4 V) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clearing the color buffer and the depth buffer

	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 1.0f, 100.0f); // calculating the projection matrix


	// Making camera to shake
	glm::vec3 shakenCameraPos = camera_pos;
	if (cameraShakeOn) {
		applyCameraShake(shakenCameraPos);
	}
	V = glm::lookAt(shakenCameraPos, shakenCameraPos + camera_front, camera_up);

	// Activating the shader and setting up light sources
	sp->use();
	glUniform4fv(sp->u("lp1"), 1, glm::value_ptr(lightPos1));
	glUniform4fv(sp->u("lp2"), 1, glm::value_ptr(lightPos2));
	glUniform1i(sp->u("useLight1"), useLight1);
	glUniform1i(sp->u("useLight2"), useLight2);
	glUniform4fv(sp->u("lightColor1"), 1, glm::value_ptr(lightColor1));
	glUniform4fv(sp->u("lightColor2"), 1, glm::value_ptr(lightColor2));

	// Preparing vectors for lava
	std::vector<glm::vec3> lavaPositions;
	std::vector<glm::vec3> lavaRotationAngles;
	std::vector<glm::vec3> lavaScales;
	std::vector<float> lavaAlphas;

	for (int i = lavaVector.size() - 1; i >= 0; --i) {
		const auto& lava = lavaVector[i];
		lavaPositions.push_back(lava.position);
		lavaRotationAngles.push_back(glm::vec3(0.0f, lava.rotationAngle, 0.0f));
		lavaScales.push_back(glm::vec3(0.05f * lava.scale));
		lavaAlphas.push_back(lava.alpha);
	}

	// Preparing vectors for stones
	std::vector<glm::vec3> stonePositions;
	std::vector<glm::vec3> stoneRotationAngles;
	std::vector<glm::vec3> stoneScales;
	std::vector<float> stoneAlphas;

	for (int i = stoneVector.size() - 1; i >= 0; --i) {
		const auto& stone = stoneVector[i];
		stonePositions.push_back(stone.position);
		stoneRotationAngles.push_back(glm::vec3(0.0f, stone.rotationAngle, 0.0f));
		stoneScales.push_back(glm::vec3(0.3f * stone.scale));
		stoneAlphas.push_back(stone.alpha);
	}

	// Preparing vectors for smoke
	std::vector<glm::vec3> smokePositions;
	std::vector<glm::vec3> smokeRotationAngles;
	std::vector<glm::vec3> smokeScales;
	std::vector<float> smokeAlphas;

	for (int i = smokeVector.size() - 1; i >= 0; --i) {
		const auto& smoke = smokeVector[i];
		smokePositions.push_back(smoke.position);
		smokeRotationAngles.push_back(glm::vec3(0.0f, smoke.rotationAngle, 0.0f));
		smokeScales.push_back(glm::vec3(0.15f * smoke.scale));
		smokeAlphas.push_back(smoke.alpha);
	}

	// Drawing models
	drawModel(oceanModel, sp, P, V, oceanPositions, ocenRotationAngles, oceanScales, oceanAlphas, true, false);
	drawModel(volcanoModel, sp, P, V, { glm::vec3(0.0f, -2.3f, 0.0f) }, { glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f) }, { glm::vec3(10.0f, 10.0f, 10.0f) }, { 1.0f }, true, false);
	drawModel(deadTreeModel, sp, P, V, deadTreePositions, deadTreeRotationAngles, deadTreeScales, deadTreeAlphas, true, false);
	drawModel(sunModel, sp, P, V, { glm::vec3(lightPos1) }, { glm::vec3(0.0f, 0.0f, 0.0f) }, { glm::vec3(1.5f, 1.5f, 1.5f) }, { 1.0f }, false, false);
	drawModel(stoneModel, sp, P, V, stonePositions, stoneRotationAngles, stoneScales, stoneAlphas, true, true);
	drawModel(lavaModel, sp, P, V, lavaPositions, lavaRotationAngles, lavaScales, lavaAlphas, false, true);
	drawModel(smokeModel, sp, P, V, smokePositions, smokeRotationAngles, smokeScales, smokeAlphas, true, true);

	// Calculating FPS
	/*
	frameCount++;
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = currentTime - lastTime;

	if (elapsedTime.count() >= 1.0f) {
		fps = frameCount / elapsedTime.count();
		std::cout << "FPS: " << fps << std::endl;
		frameCount = 0;
		lastTime = currentTime;
	}
	*/

	glfwSwapBuffers(window); // Copying the back buffer to the front buffer
}

int main(void)
{
	GLFWwindow* window; // Pointer to the object representing the application window

	glfwSetErrorCallback(error_callback); // Registering the error handling procedure

	if (!glfwInit()) { // Initializing the GLFW library
		fprintf(stderr, "Can't initialize GLFW.\n");
		exit(EXIT_FAILURE);
	}

	// Getting the screen resolution
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int screenWidth = mode->width;
	int screenHeight = mode->height;
	int windowWidth = 1000;
	int windowHeight = 1000;

	// Calculating position to center the window
	int windowPosX = (screenWidth - windowWidth) / 2;
	int windowPosY = (screenHeight - windowHeight) / 2;

	window = glfwCreateWindow(windowWidth, windowHeight, "Volcano", NULL, NULL);  // Creating the window

	if (!window) // If the window cannot be opened, end the program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowPos(window, windowPosX, windowPosY); // Centering the window

	glfwMakeContextCurrent(window); // From this point, the OpenGL context corresponding to the window is active, and all OpenGL calls refer to this context.
	glfwSwapInterval(1); // Vsync

	GLenum err;
	if ((err = glewInit()) != GLEW_OK) { // Initializing the GLEW library
		fprintf(stderr, "Can't initialize GLEW: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); // Invoking the initialization procedure

	glfwSetTime(0); // Resetting the timer

	// Main loop
	while (!glfwWindowShouldClose(window)) // Until the window is closed
	{
		float deltaTime = glfwGetTime(); // Time that has passed since the last reset (last loop execution)
		glfwSetTime(0); // Resetting the timer

		// Handling smoke
		timeSinceLastSmokeSpawn += deltaTime;
		if (smokeOn && timeSinceLastSmokeSpawn >= smokeSpawnInterval) {
			for (int i = 0; i < smokeSpawnNumber; i++) {
				smokeVector.emplace_back(); // Adding a new smoke particle to the vector
			}
			timeSinceLastSmokeSpawn = 0.0f;
		}

		// Handling eruptions
		timeSinceLastEruption += deltaTime;
		if (eruptionOn && timeSinceLastEruption >= eruptionInterval) {
			// Handling the second light source
			if (lavaOn) {
				useLight2 = true;

				timeSinceLastRedChange += deltaTime;
				if (timeSinceLastRedChange >= redLightChangeInterval) {
					lightColor2 = glm::vec4(redLightDist(gen), 0.0f, 0.0f, 1.0f);
					timeSinceLastRedChange = 0.0f;
				}
			}

			// Handling lava
			timeSinceLastLavaSpawn += deltaTime;
			if (lavaOn && timeSinceLastLavaSpawn >= lavaSpawnInterval) {
				for (int i = 0; i < lavaSpawnNumber; i++) {
					lavaVector.emplace_back();
				}
				timeSinceLastLavaSpawn = 0.0f;
			}

			// Handling stones
			timeSinceLastStoneLaunch += deltaTime;
			if (stonesOn && timeSinceLastStoneLaunch >= stoneLaunchInterval) {
				for (int i = 0; i < stoneSpawnNumber(gen); i++) {
					stoneVector.emplace_back();
				}
				timeSinceLastStoneLaunch = 0.0f;
			}

			// Ending the eruption
			if (timeSinceLastEruption >= eruptionDuration + eruptionInterval) {
				timeSinceLastEruption = 0.0f;
				timeSinceLastLavaSpawn = lavaSpawnInterval;
				timeSinceLastStoneLaunch = stoneLaunchInterval - 1.0f;
				//useLight2 = false;
			}
		}
		else if (useLight2) { // If there is no eruption and light 2 is still on
			if (timeSinceLastEruption >= 1.2f) { // 1.2 seconds after the eruption
				useLight2 = false;
			}
		}
		
		// Handling camera shake
		if (cameraShake) {
			shakeElapsed += deltaTime;
			if (shakeElapsed >= shakeDuration) {
				cameraShake = false;
				shakeElapsed = 0.0f;
			}
		}

		// Updating object positions
		for (auto& stone : stoneVector) {
			updateStone(stone, deltaTime);
		}

		for (auto& smoke : smokeVector) {
			updateSmoke(smoke, deltaTime);
		}

		for (auto& lava : lavaVector) {
			updateLava(lava, deltaTime);
		}

		// Handling camera
		camera_pos += camera_move_speed_z * deltaTime * camera_front;
		camera_pos += camera_move_speed_x * deltaTime * glm::normalize(glm::cross(camera_front, camera_up));
		camera_pos += camera_move_speed_y * deltaTime * camera_up;
		glm::mat4 V = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

		drawScene(window, V); // Execute drawing procedure
		glfwPollEvents(); // Process callback procedures corresponding to the events that took place up to now
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); // Delete OpenGL context and the window.
	glfwTerminate(); // Free GLFW resources
	exit(EXIT_SUCCESS);
}
