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

const void cModel::draw(const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors, int amount){
	glBindVertexArray(data->getVao());
	glUniformMatrix4fv(glGetUniformLocation(prog, "M"), 1, GL_FALSE, glm::value_ptr(getModelMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(prog, "MVP"), 1, GL_FALSE, glm::value_ptr(VP*getModelMatrix()));
	for (int i = 0; i < data->getNumMeshes(); i++){
		const IQM::iqmmesh &m = data->getMesh(i);
		if (useTex){
			glUniform3fv(glGetUniformLocation(prog, "color"), 1, glm::value_ptr(colors[std::min(i, amount - 1)]));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, data->getTexture(i));
		}
		glDrawElements(GL_TRIANGLES, m.num_triangles * 3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int)* m.first_triangle * 3));
	}
}