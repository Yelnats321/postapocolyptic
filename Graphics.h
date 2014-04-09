#pragma once
#include "Map.h"
class Player;
class Model;
//If the const matrixes break, it's cuz you defined the consts it relies on AFTER it, becasue of some magic shit in C++
class Graphics{
	const float Z_DEPTH, X_Y_DEPTH , WINDOW_WIDTH , WINDOW_HEIGHT, SHADOW_CUBE_SIZE;
	GLFWwindow * window;
	GLuint baseProgram;
	const glm::mat4 baseView, baseProjection;
	GLuint shadowFrameBuffer, shadowCubemap, shadowProgram;
	GLuint projVao, projVbo;
	glm::vec3 projHit;
	glm::mat4 projOffset;
	glm::mat4 shadowMapViews[6];
	Player * player;
	glm::vec2 getMouseTile(Map &, float);
	void drawModel(const Model *, const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 *, int amount);
	void drawMap(const Map *,const glm::mat4 & VP, bool useTex, GLuint prog);
public:
	Graphics();
	~Graphics();
	bool isOpen();
	void processInput();
	void update(double );
	void setPlayer(Player *);
};