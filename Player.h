#pragma once
#include "Model.h"
class Player: public Model{
public:
	Player();
	~Player();
	GLuint frameBuffer, cubemap, proggy;
	glm::mat4 sideViews[6];
};