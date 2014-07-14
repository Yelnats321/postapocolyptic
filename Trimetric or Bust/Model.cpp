#include "stdafx.h"
#include "Model.h"
#include "IQMFile.h"

const glm::quat Model::BASEROTATION(glm::vec3(-M_PI/2, -M_PI/2, 0.0));

Model::Model( const std::string & name):scale(1), data(IQMFile::openFile(name)){
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

void Model::draw(const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors, int amount) const{
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