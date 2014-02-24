#pragma once

class Map{
	unsigned int width, height;
	GLuint texture;
	std::vector<uint16_t> mapData;
public:
	Map(std::string name);
	inline const uint16_t Map::operator()(unsigned int x, unsigned int y) const{
		//assert(x>=0 && x<width && y>=0 && y<height);
		if(!(x>=0 && x<width && y>=0 && y<height))
			return -1;
		return mapData[x+y*width];
	}
	unsigned int getWidth();
	unsigned int getHeight();
	GLuint getTexture();
};