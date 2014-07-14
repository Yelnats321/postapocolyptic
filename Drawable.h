#pragma once

class Drawable{
public:
	virtual void draw(const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors, int amount) const = 0;
};