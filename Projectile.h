#pragma once
#include "Drawable.h"

class Projectile: public Drawable{
	static GLuint vao, vbo;
public:
	float speed;
	glm::vec3 position, direction;
	glm::mat4 modelMatrix;
	Projectile(float, glm::vec3 &&, glm::vec3 &&, glm::mat4 &&);
	void draw(const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors = nullptr, int amount = 0) const;
	static void initialize();
	static void close();
	static GLuint getVao(){ return vao; }
};