#pragma once
#include "Map.h"
#include "EntityManager.h"

class Entity;
//If the const matrixes break, it's cuz you defined the consts it relies on AFTER it, becasue of some magic shit in C++
class Graphics{
	const float Z_DEPTH, X_Y_DEPTH , WINDOW_WIDTH , WINDOW_HEIGHT, SHADOW_CUBE_SIZE, SHADOW_NEAR, SHADOW_FAR;
	GLFWwindow * window;
	GLuint firstPassProgram, secondPassProgram;
	const glm::mat4 baseView, baseProjection;
	GLuint shadowFrameBuffer, shadowCubemap, shadowProgram;
	//GLuint projVao, projVbo;
	/*glm::vec3 projHit;
	glm::mat4 projOffset;*/
	glm::mat4 shadowMapViews[6];
	glm::vec2 getMouseTile(float);
	EntityManager * entityManager;
	Map* map;
	void drawMap(const glm::mat4 & VP, bool useTex, GLuint prog);
	void fire();
public:
	Graphics();
	~Graphics();
	void processInput();
	void update(double );
	void setManager(EntityManager * em){ entityManager = em; }
	bool isOpen(){ return !glfwWindowShouldClose(window); }
};