#pragma once

/*class Mesh;
class ObjFile;*/

GLuint loadTexture(std::string,bool isSRGB);
//const ObjFile * loadFile(string);
GLuint genShaders(std::string vertex, std::string fragment);