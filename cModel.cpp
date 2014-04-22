#include "stdafx.h"
#include "cModel.h"
#include "IQMFile.h"

const glm::quat cModel::BASEROTATION(glm::vec3(-M_PI/2, -M_PI/2, 0.0));

cModel::cModel(const std::string & name):cBase(nullptr),scale(1), data(IQMFile::openFile(name)){
	updateMatrix();
}

//THE ORDER IS TRANSLATE, ROTATE, SCALE
//TRANSLATE TO ORIGIN BEFORE APPLYING ANYTHING
void cModel::updateMatrix(){
	modelMatrix = glm::translate(glm::mat4(), position);
	modelMatrix *= glm::toMat4(rotation*BASEROTATION);
	modelMatrix = glm::scale(modelMatrix, scale);
}
void cModel::setScale(float sc){
	setScale(sc, sc, sc);
}
void cModel::setScale(float x, float y, float z){
	scale = glm::vec3(x,y,z);
	updateMatrix();
}
void cModel::setPosition(const glm::vec3 & pos){
	position = pos;
	updateMatrix();
}
void cModel::setPosition(float x, float y, float z){
	setPosition(glm::vec3(x,y,z));
}
void cModel::setRotation(float x, float y, float z){
	rotation = glm::quat(glm::vec3(x,y,z));
	updateMatrix();
}
const glm::mat4 & cModel::getModelMatrix() const{return modelMatrix;}
const glm::vec3 & cModel::getPosition() const{return position;}