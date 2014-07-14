#pragma once

class Map{
	unsigned int width, height;
	GLuint texture, vao, vbo, ebo;
	std::vector<uint16_t> mapData;
public:
	static const int SIZE = 1;
	Map(std::string name);
	~Map();
	inline const uint16_t Map::operator()(unsigned int x, unsigned int y) const{
		//assert(x>=0 && x<width && y>=0 && y<height);
		if(!(x>=0 && x<width && y>=0 && y<height))
			return -1;
		return mapData[x+y*width];
	}
	void draw(const glm::mat4 & VP, bool useTex, GLuint prog);
	unsigned int getWidth() const{ return width; }
	unsigned int getHeight() const{ return height; }
	//GLuint getTexture() const{ return texture; }
	//GLuint getVao() const{ return vao; }
};