#pragma once
#include "cBase.h"
#include "Entity.h"
class IQMFile;

class cModel:public cBase{
	cModel(const cModel &) = delete;
	static const glm::quat BASEROTATION;
	glm::mat4 modelMatrix;
	glm::vec3 scale, position;
	glm::quat rotation;
	void updateMatrix();
public:
	cModel(const std::string &);
	cModel(Entity * a, const std::string & b):cBase(a), data(nullptr){
		std::cout << a << " " << b << std::endl;
	}
	const IQMFile * const data;
	void setScale(float);
	void setScale(float, float, float);
	void setPosition(float, float, float);
	void setPosition(const glm::vec3 &);
	void setRotation(float, float, float);
	const glm::vec3 & getPosition() const;
	const glm::mat4 & getModelMatrix() const;
	const void draw(const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors = nullptr, int amount = 0);
};