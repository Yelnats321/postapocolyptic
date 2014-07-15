#pragma once

/*class Mesh;
class ObjFile;*/
class TextureManager{
	struct Texture{
		GLuint tex;
		Texture(const std::string &, bool);
		Texture(Texture &&);
		~Texture();
	};
	TextureManager() = delete;
	static std::unordered_map<std::string, Texture> textures;
public:
	static const GLuint loadTexture(const std::string &, bool isSRGB);
};
//const ObjFile * loadFile(string);
namespace AssetLoader{
GLuint genShaders(const std::string & vertex, const std::string & fragment);
std::string getFilePath(const std::string &);
}