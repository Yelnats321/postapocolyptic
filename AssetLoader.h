#pragma once

/*class Mesh;
class ObjFile;*/
class TextureManager{
	struct Texture{
		GLuint tex;
		Texture(std::string, bool);
		Texture(Texture &&);
		~Texture();
	};
	static std::unordered_map<std::string, Texture> textures;
public:
	static const GLuint loadTexture(std::string,bool isSRGB);
};
//const ObjFile * loadFile(string);
GLuint genShaders(std::string vertex, std::string fragment);