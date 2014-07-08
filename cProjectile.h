#pragma once

struct cProjectile{
	float speed;
	glm::vec3 position, direction;
	glm::mat4 modelMatrix;
};