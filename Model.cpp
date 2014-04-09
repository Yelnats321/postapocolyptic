#include "stdafx.h"
#include "Model.h"
#include "IQMFile.h"

const glm::quat Model::BASEROTATION(glm::vec3(-M_PI/2, -M_PI/2, 0.0));

Model::Model(const std::string & name):scale(1), data(IQMFile::openFile(name)){
	updateMatrix();
}

//THE ORDER IS TRANSLATE, ROTATE, SCALE
//TRANSLATE TO ORIGIN BEFORE APPLYING ANYTHING
void Model::updateMatrix(){
	modelMatrix = glm::translate(glm::mat4(), position);
	modelMatrix *= glm::toMat4(rotation*BASEROTATION);
	modelMatrix = glm::scale(modelMatrix, scale);
}
void Model::setScale(float sc){
	setScale(sc, sc, sc);
}
void Model::setScale(float x, float y, float z){
	scale = glm::vec3(x,y,z);
	updateMatrix();
}
void Model::setPosition(const glm::vec3 & pos){
	position = pos;
	updateMatrix();
}
void Model::setPosition(float x, float y, float z){
	setPosition(glm::vec3(x,y,z));
}
void Model::setRotation(float x, float y, float z){
	rotation = glm::quat(glm::vec3(x,y,z));
	updateMatrix();
}
const glm::mat4 & Model::getModelMatrix() const{return modelMatrix;}
const glm::vec3 & Model::getPosition() const{return position;}