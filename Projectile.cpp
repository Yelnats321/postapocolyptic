#include "stdafx.h"
#include "Projectile.h"

Projectile::Projectile(float s, glm::vec3 && p, glm::vec3 && d, glm::mat4 && m):
position(std::move(p)),direction(std::move(d)),modelMatrix(std::move(m)){
	speed = s;
}

void Projectile::draw(const glm::mat4 & VP, bool useTex, GLuint prog, const glm::vec3 * colors, int amount) const{
	if(useTex) glUniform3fv(glGetUniformLocation(prog, "color"), 1, glm::value_ptr(glm::vec3(1, 1, 0)));
	glUniformMatrix4fv(glGetUniformLocation(prog, "M"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(prog, "MVP"), 1, GL_FALSE, glm::value_ptr(VP*modelMatrix));
	glDrawArrays(GL_LINES, 0, 6);
}

GLuint Projectile::vao, Projectile::vbo;
void Projectile::initialize(){
	//projectile test shit
	GLfloat vert[] = {0, 0, 0,
		1, 1, 1};
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}
void Projectile::close(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}