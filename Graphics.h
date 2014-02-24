#pragma once
#include "Map.h"
class Player;
class Graphics{
	GLFWwindow * window;
	GLuint vao, vbo, ebo, program;
	glm::mat4 view, projection;
	Player * player;
public:
	Graphics();
	~Graphics();
	bool isOpen();
	void processInput();
	void update(double );
	glm::vec2 getMouseTile(Map &);
	void setPlayer(Player *);
};