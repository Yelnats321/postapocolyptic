#pragma once
#include "Map.h"
class Player;
class Model;
class Graphics{
	GLFWwindow * window;
	GLuint baseProgram;
	glm::mat4 baseView, baseProjection;
	GLuint shadowFrameBuffer, shadowCubemap, shadowProgram;
	GLuint projVao, projVbo;
	glm::mat4 shadowMapViews[6];
	Player * player;
	glm::vec2 getMouseTile(Map &);
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