#include "stdafx.h"
#include "Map.h"
#include "AssetLoader.h"
#include <fstream>

Map::Map(std::string name){
	const GLfloat vertices[] = {0, 0, 0, 0, 0,// 1, 1, 1,
		1, 0, 0, 1, 0,// 1, 0, 0,
		0, 0, 1, 0, 1,// 0, 1, 0,
		1, 0, 1, 1, 1,// 0, 0, 1,

		/*-0.5f, 0,-0.5f, 1, 1, 1,
		0.5f, 0,-0.5f, 1, 0, 0,
		-0.5f, 1.f,-0.5f, 0, 1, 0,
		0.5f, 1.f,-0.5f, 0, 0, 1,*/
	};
	const GLuint indices[] = {0,1,2,
		1,3,2,
		/*4,5,6,
		5,7,6,*/};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
	//3 because we need 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) *5, (void*)(sizeof(GLfloat)*3));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

	//THE ACTUAL MAP READING SHIT
	std::ifstream file(name,std::ios::in|std::ios::binary);
	if(!file.good()){
		throw;
	}
	uint8_t size = 0;
	file.read((char *)&size, 1);
	std::string imgName;
	imgName.resize(size);
	file.read((char*)&imgName[0], size);
	texture = loadTexture(imgName, false);
	file.read((char *)&width, 2);
	file.read((char *)&height, 2);
	mapData.resize(width*height);
	file.read((char *)&mapData[0], mapData.size() * sizeof(uint16_t));

}

Map::~Map(){
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

unsigned int Map::getWidth() const{return width;}
unsigned int Map::getHeight() const{return height;}
GLuint Map::getTexture() const{return texture;}
GLuint Map::getVao() const{return vao;}