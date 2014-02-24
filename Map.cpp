#include "stdafx.h"
#include "Map.h"
#include "AssetLoader.h"
#include <fstream>

Map::Map(std::string name){
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

unsigned int Map::getWidth(){return width;}
unsigned int Map::getHeight(){return height;}
GLuint Map::getTexture(){return texture;}