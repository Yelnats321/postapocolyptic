#pragma once
#include "cBase.h"
#include "Entity.h"
class IQMFile;

class cModel:public cBase{
	static const glm::quat BASEROTATION;
	glm::mat4 modelMatrix;
	glm::vec3 scale, position;
	glm::quat rotation;
	void updateMatrix();
public:
	cModel(const std::string &);
	const IQMFile * const data;
	void setScale(float);
	void setScale(float, float, float);
	void setPosition(float, float, float);
	void setPosition(const glm::vec3 &);
	void setRotation(float, float, float);
	const glm::vec3 & getPosition() const;
	const glm::mat4 & getModelMatrix() const;
};