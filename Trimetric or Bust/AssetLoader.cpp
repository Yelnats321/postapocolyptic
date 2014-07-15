﻿#include "stdafx.h"
#include "AssetLoader.h"
#include <fstream>
#include <sstream>

extern "C"{
#include <SOIL.h>
}
//
//#include "Material.h"
//#include "Mesh.h"
//#include "ObjFile.h"
//
//
//map<string, unique_ptr<ObjFile> > files;

std::string getShaderError(GLuint shader){
	GLint maxLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	//char * errorLog = new char[maxLength];
	std::vector<char> errorLog(maxLength);
	std::cout<<maxLength<<std::endl;
	glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
	return std::string(errorLog.begin(), errorLog.end());
}

GLuint AssetLoader::genShaders(const std::string & vert, const std::string & frag){
	std::ifstream file;

	std::string vertexSource, fragmentSource;
	file.open(vert);
	if(file){
		file.seekg(0, std::ios::end);
		vertexSource.resize((unsigned int)file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&vertexSource[0], vertexSource.size());
		file.close();
	}
	else throw "No vertex shader";
	file.open(frag);
	if(file){
		file.seekg(0, std::ios::end);
		fragmentSource.resize((unsigned int)file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&fragmentSource[0], fragmentSource.size());
		file.close();
	}
	else throw "No fragment shader";
	// Create and compile the vertex shader
	GLuint vertShader = glCreateShader( GL_VERTEX_SHADER );
	const char *c_str1 = vertexSource.c_str();
	glShaderSource( vertShader, 1, &c_str1, NULL );
	glCompileShader( vertShader );

	GLint statusV;
	glGetShaderiv( vertShader, GL_COMPILE_STATUS, &statusV ); 
	if(statusV == 0)
		std::cout<<getShaderError(vertShader);
	// Create and compile the fragment shader
	GLuint fragShader = glCreateShader( GL_FRAGMENT_SHADER );
	const char *c_str2 = fragmentSource.c_str();
	glShaderSource( fragShader, 1, &c_str2, NULL );
	glCompileShader( fragShader );

	GLint statusF;
	glGetShaderiv( fragShader, GL_COMPILE_STATUS, &statusF ); 
	if(statusF == 0)
		std::cout<<getShaderError(vertShader);


	if(statusV == 0 || statusF==0){
		std::cout<<"Shader status: " <<statusV<< " " << statusF << std::endl;
		std::string error;
		if(statusV)
			error= getShaderError(vertShader);
		if(statusF)
			error = error + getShaderError(fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
		std::cout<<error<<std::endl;
		throw error.c_str();
	}

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProg = glCreateProgram();
	glAttachShader( shaderProg, vertShader );
	glAttachShader( shaderProg, fragShader );
	glLinkProgram( shaderProg );
	glDetachShader(shaderProg, vertShader);
	glDetachShader(shaderProg, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return shaderProg;
}
//this probably doesn't work for something like "C:/file.txt/"
std::string AssetLoader::getFilePath(const std::string & file){
	size_t pos = file.find_first_of("\\/");
	return(std::string::npos == pos) ? "" : file.substr(0, pos+1);
}

TextureManager::Texture::Texture(const std::string & name, bool sRGB){
	tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	int width, height, channels;
	unsigned char* image = SOIL_load_image(name.c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
	if(sRGB)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
	//TODO: this doesn't work on radeon cards maybe
	glEnable(GL_TEXTURE_2D);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	if(image == nullptr){
		throw SOIL_last_result();
	}
	std::cout << "Loaded texture \"" + name + "\" to the location " << tex<<std::endl;
}
TextureManager::Texture::Texture(Texture && other){
	tex = other.tex;
	other.tex = 0;
}
TextureManager::Texture::~Texture(){
	glDeleteTextures(1, &tex);
}
std::unordered_map<std::string, TextureManager::Texture> TextureManager::textures;
const GLuint TextureManager::loadTexture(const std::string & name, bool sRGB){
	if(textures.find(name) != textures.end()){
		std::cout<<"Returned loaded texture \"" +name + "\" at location "<<textures.at(name).tex<<std::endl;
	}
	else{
		Texture tex(name, sRGB);
		textures.emplace(name, std::move(tex));
	}
	return textures.at(name).tex;
}
//
//void locationAppend(string origin, string & file){
//	std::replace(origin.begin(), origin.end(), '/', '\\');
//	if(origin.find_last_of('\\')!= string::npos){
//		file = origin.substr(0, origin.find_last_of('\\') +1) + file;
//	}
//}
//
////remember to extract curr location 
//map<string, Material> loadMaterialLibrary(string name){
//	std::cout << "-Loading the material library " + name<<std::endl;
//	map<string, Material> mats;
//
//	std::ifstream file;
//	file.open(name);
//	if(file.fail()){
//		std::cout << "***Error reading materials file";
//		return mats;
//	}
//	vector<string> line;
//	string buf;
//	while(!file.eof()){
//		std::getline(file, buf);
//		line.emplace_back(std::move(buf));
//	}
//	file.close();
//	Material * currMat;
//
//	for(const auto & itr:line){
//		string key;
//		std::istringstream ss(itr);
//		ss >> std::ws>> key >> std::ws;
//
//		if(key.length() == 0)
//			continue;
//
//		if(key.at(0) == '#')
//			continue;
//
//		if(key == "newmtl"){	
//			string matName;
//			ss >> matName;
//			std::cout << " New material: "+matName<<std::endl;
//			mats.emplace(matName, Material());
//			currMat= &mats.find(matName)->second;
//		}
//
//		else if(key == "Ns" || key == "Ni" || key == "illum" || key == "Tr" || key == "d"){
//			float value = 0;
//			ss >> value;
//
//			if(key== "Ns")
//				currMat->Ns = value;
//
//
//			else if(key == "Ni")
//				currMat->Ni = value;
//
//
//			else if(key == "illum")
//				currMat->illum = (int)value;
//
//			else 
//				currMat->Tr = value;
//		}
//
//		else if(key == "Tf" || key == "Ka" || key == "Kd" || key == "Ks"){
//			float a, b, c;
//			a = b=c=0;
//			ss >> a >> std::ws>>b>>std::ws>>c;		
//
//			if(key == "Tf"){
//				currMat->Tf[0] = a;
//				currMat->Tf[1] = b;
//				currMat->Tf[2] = c;
//			}
//
//			else if(key == "Ka"){
//				currMat->Ka[0] = a;
//				currMat->Ka[1] = b;
//				currMat->Ka[2] = c;
//			}
//
//			else if(key == "Kd"){
//				currMat->Kd[0] = a;
//				currMat->Kd[1] = b;
//				currMat->Kd[2] = c;
//			}
//
//			else{
//				currMat->Ks[0] = a;
//				currMat->Ks[1] = b;
//				currMat->Ks[2] = c;
//			}
//		}
//
//		else if(key == "map_Ka" || key == "map_Kd" || key == "map_d" || key == "bump" || key == "map_bump"){
//			string mapName;
//			ss>>mapName;
//			locationAppend(name, mapName);
//			bool sRGB =!( key=="bump" || key == "map_bump");
//			GLuint texture = loadTexture(mapName, sRGB);
//
//			if(key == "map_Ka")
//				currMat->map_Ka = texture;
//
//			else if(key == "map_Kd")
//				currMat->map_Kd = texture;
//
//			else if(key == "map_d")
//				currMat->map_d = texture;
//
//			//if it isn't any of those maps, it must be bump map
//			else
//				currMat->map_bump = texture;
//		}
//
//		else{
//			std::cout << itr<<std::endl;
//		}
//	}
//
//	return std::move(mats);
//}
//
//struct VertexData{
//	GLuint pos, tex, norm;
//	VertexData(GLuint a, GLuint b, GLuint c):pos(a), tex(b), norm(c){}
//	inline bool operator==(const VertexData &rhs) const{
//		return(pos==rhs.pos && tex == rhs.tex && norm ==rhs.norm);
//	}
//};
//
//bool parseObj(string name, vector<glm::vec3> & positions, vector<glm::vec2> & textures, vector<glm::vec3> & normals, vector<VertexData> & vertexData, vector<std::pair<string, int> > & matCalls, map<string, Material> & matLib, vector<std::pair<string, int> > & groups){
//	std::cout<<"Loading file " + name<<std::endl;
//	std::ifstream file;
//	file.open(name);
//	if(file.fail()){
//		std::cout << "Error reading file "+ name << std::endl;;
//		return false;
//	}
//	vector<string> line;
//	string buf;
//	while(!file.eof()){
//		std::getline(file, buf);
//		line.emplace_back(std::move(buf));
//	}
//	file.close();
//
//	for(const auto & itr : line){
//		string key;
//		std::istringstream ss(itr);
//		ss >> std::ws>> key >> std::ws;
//
//		if(key.length() == 0)
//			continue;
//
//		if(key.at(0) == '#')
//			continue;
//
//		if(key == "v" || key == "vt" || key == "vn"){
//			float x, y, z;
//			ss >> x >> std::ws>> y>>std::ws>> z;
//			if(key=="v")
//				positions.emplace_back(x,y,z);
//			else if(key=="vt")
//				textures.emplace_back(x,y);
//			else
//				normals.emplace_back(x,y,z);
//		}
//
//		else if(key == "g"){
//			string groupName;
//			ss>>groupName;
//			if(groups.size() > 0 && groups.back().second == vertexData.size())
//				groups.back().first = std::move(groupName);
//			else
//				groups.emplace_back(std::move(groupName), vertexData.size());
//		}
//
//		else if(key == "mtllib"){
//			string matName;
//			ss >> matName;
//			locationAppend(name, matName);
//			matLib = std::move(loadMaterialLibrary(matName));
//		}
//
//		else if(key == "usemtl"){
//			string mtlName;
//			ss >> mtlName;
//			if(matCalls.size() > 0 && matCalls.back().second == vertexData.size())
//				matCalls.back().first= std::move(mtlName);
//			else
//				matCalls.emplace_back(std::move(mtlName), vertexData.size());
//		}
//		//faces
//		else if(key == "f"){
//			for(int i = 0; i < 3; ++i){
//				GLuint a,b,c;
//				a=b=c=0;
//
//				ss >> a;
//				if(ss.get() == '/'){
//					if(ss.peek()!='/'){
//						ss >> b;
//					}
//					if(ss.get() == '/'){
//						ss>> c;
//					}
//				}
//				vertexData.emplace_back(a,b,c);
//				if(i ==2){
//					ss>>std::ws;
//					if(!ss.eof()){
//						vertexData.emplace_back(a,b,c);
//						a=b=c=0;
//
//						ss >> a;
//						if(ss.get() == '/'){
//							if(ss.peek()!='/'){
//								ss >> b;
//							}
//							if(ss.get() == '/'){
//								ss>> c;
//							}
//						}	
//
//						vertexData.emplace_back(a,b,c);
//
//						vertexData.emplace_back(vertexData[vertexData.size()-5]);
//					}
//				}
//			}
//		}
//	}
//	if(groups.size()==0)
//		groups.emplace_back("default",0);
//	return true;
//}
//
//void calculateTangents(vector<glm::vec3> & positions, vector<glm::vec2> & textures, vector<glm::vec3> & normals, vector<VertexData> & vertexData, vector<glm::vec3>& tangents, vector<glm::vec3> & bitangents){
//	for(int i = 0; i <vertexData.size(); i+=3){
//		//std::cout<<"triangle "<<i/3<<std::endl;
//		glm::vec3 & v0 = positions[vertexData[i].pos-1];
//		glm::vec3 & v1 = positions[vertexData[i+1].pos-1];
//		glm::vec3 & v2 = positions[vertexData[i+2].pos-1];
//
//		glm::vec2 & uv0 = textures[vertexData[i].tex-1];
//		glm::vec2 & uv1 = textures[vertexData[i+1].tex-1];
//		glm::vec2 & uv2 = textures[vertexData[i+2].tex-1];
//
//		glm::vec3 deltaPos1 = v1-v0;
//		glm::vec3 deltaPos2 = v2-v0;
//
//		glm::vec2 deltaUV1 = uv1-uv0;
//		glm::vec2 deltaUV2 = uv2-uv0;
//
//		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
//		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
//		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
//		for(int j =0; j < 3; j++){
//			tangents.emplace_back(tangent);
//			bitangents.emplace_back(bitangent);
//		}
//	}
//	for(int i = 0; i < vertexData.size(); i++){
//		glm::vec3 & n = normals[vertexData[i].norm-1];
//		glm::vec3 & t = tangents[i];
//		glm::vec3 & b = bitangents[i];
//
//		// Gram-Schmidt orthogonalize
//		t = glm::normalize(t - n * glm::dot(n, t));
//
//		// Calculate handedness
//		if (glm::dot(glm::cross(n, t), b) < 0.0f){
//			t = t * -1.0f;
//		}
//	}
//}
//
//const ObjFile * loadFile(string name){
//	if(files.find(name) != files.end()){
//		std::cout<<"Retrieved file " + name <<std::endl;
//		return files.find(name)->second.get();
//	}
//
//	vector<glm::vec3> positions;
//	vector<glm::vec2> textures;
//	vector<glm::vec3> normals;
//
//	vector<VertexData> vertexData;
//	vector<std::pair<string, int> > masterMatCalls;
//	map<string, Material>matLib;
//	vector<std::pair<string, int> > groups;
//	if(!parseObj(name, positions, textures, normals, vertexData, masterMatCalls, matLib, groups))
//		return nullptr;
//
//	/*for(const auto & i : groups)
//		std::cout<<i.first << " " <<i.second<<std::endl;*/
//	bool useTextures = false, useNormals =false;
//
//	if(vertexData[0].tex != 0)
//		useTextures = true;
//	if(vertexData[0].norm !=0)
//		useNormals = true;
//	//REMEMBER SPONGEBOB
//	//RAVIOLI RAVIOLI
//	//DON'T FORGET THE SUBTRACTIOLI
//	//OBJ COUNTS FROM 1 NOT 0
//
//
//	vector<glm::vec3> tangents;
//	vector<glm::vec3> bitangents;
//	if(useNormals && useTextures){
//		calculateTangents(positions, textures, normals, vertexData, tangents, bitangents);
//	}
//
//	int nbOfVertices = vertexData.size();
//	vector<GLuint> elements(nbOfVertices);
//	for(int i = 0; i < nbOfVertices; ++i)
//		elements[i] = i;
//
//	std::cout << glfwGetTime() << std::endl;
//	//Assume all elements are unique, until you check out if they aren't
//	for(int i =0; i<nbOfVertices; ++i){
//		/*std::cout << tangents[i].x << " " <<tangents[i].y<<" "<<tangents[i].z<<std::endl;
//		std::cout << bitangents[i].x << " " <<bitangents[i].y<<" "<<bitangents[i].z<<std::endl;
//		std::cout<< normals[vertexData[i].x-1].z<< " "<< normals[vertexData[i].z-1].y<< " "<< normals[vertexData[i].z-1].z<< std::endl<<std::endl;*/
//		if(elements[i] == i){
//			for(int j = i+1; j <nbOfVertices; ++j){
//				if(vertexData[i] == vertexData[j]){
//					if(useTextures && useNormals){
//						tangents[i] += tangents[j];
//						bitangents[i] += bitangents[j];
//					}
//					elements[j] = i;
//				}
//			}
//		}
//	}
//
//	//these are just the elements, since they kept their position as their value, they have to be compressed
//	vector<GLuint> sortElem(elements);
//	std::sort(sortElem.begin(), sortElem.end());
//	sortElem.erase(std::unique(sortElem.begin(), sortElem.end()), sortElem.end());
//
//	std::cout << glfwGetTime() << std::endl;
//
//	int blockSize = 3+(useTextures?2:0) + (useNormals?3:0);
//	if(blockSize == 8)
//		blockSize = 14;
//	int sortedElemSize = sortElem.size();
//	vector<float> data(sortedElemSize * blockSize, 0);
//	std::cout<<nbOfVertices << " " <<sortedElemSize << std::endl<<std::endl<<std::endl;
//	for(int k =0; k < sortedElemSize; ++k){
//		GLuint pos = sortElem[k];
//		//std::cout<<pos<<std::endl;
//		int dataPos = k*blockSize;
//		//pack all the elements that are unique
//		//&positions[vertexData[elemPos]-1][0], &positions[vertexData[elemPos]][0],
//		//std::copy(&positions[vertexData[elemPos]-1], &positions[vertexData[elemPos]], &data[dataPos]);
//		data[dataPos] = positions[vertexData[pos].pos-1].x;
//		data[dataPos+1] = positions[vertexData[pos].pos-1].y;
//		data[dataPos+2] = positions[vertexData[pos].pos-1].z;
//		if(useTextures){
//			data[dataPos+3] = textures[vertexData[pos].tex -1].x;
//			data[dataPos+4] = textures[vertexData[pos].tex -1].y;
//			dataPos+=2;
//		}
//
//		if(useNormals){
//			//std::copy(normals.begin() + (vertexData[elemPos+2]-1)*3, normals.begin()+(vertexData[elemPos+2]-1)*3+3, data.begin()+dataPos+5);
//			//std::copy(&normals[(vertexData[elemPos+2]-1)*3], &normals[(vertexData[elemPos+2]-1)*3+3], &data[dataPos+5]);
//			data[dataPos+3] = normals[vertexData[pos].norm-1].x;
//			data[dataPos+4] = normals[vertexData[pos].norm-1].y;
//			data[dataPos+5] = normals[vertexData[pos].norm-1].z;
//		}
//
//		if(useTextures && useNormals){
//			data[dataPos+6] = tangents[pos].x;
//			data[dataPos+7] = tangents[pos].y;
//			data[dataPos+8] = tangents[pos].z;
//			data[dataPos+ 9] = bitangents[pos].x;
//			data[dataPos+10] = bitangents[pos].y;
//			data[dataPos+11] = bitangents[pos].z;
//		}
//
//		//point elements to their true location, rather than their previous pos location
//		if(k != pos){
//			std::replace(elements.begin(), elements.end(), pos, (GLuint)k);
//		}
//	}
//	std::cout << glfwGetTime() << std::endl;
//
//	GLuint vbo = 0;
//	glGenBuffers(1, &vbo);
//	glBindBuffer( GL_ARRAY_BUFFER, vbo );
//	glBufferData( GL_ARRAY_BUFFER, sizeof(float) * data.size(), &data[0], GL_STATIC_DRAW );
//	//ObjFile * objFile = new ObjFile(vbo, useTextures, std::move(matLib));
//	files.emplace(name,unique_ptr<ObjFile>(new ObjFile(vbo, useTextures, std::move(matLib))));
//	ObjFile * objFile = files[name].get();
//
//	for(int j = 0; j<groups.size(); ++j){
//		const int currPos = groups[j].second;
//		int nbGroupVertices;
//		if(j+1 == groups.size())
//			nbGroupVertices = nbOfVertices - currPos;
//		else
//			nbGroupVertices = groups[j+1].second - currPos;
//
//		std::vector<std::pair<string, int>> matCalls;
//		for(int mat = 0; mat < masterMatCalls.size(); ++mat){
//			//if this material call is the less than or equal to the group start AND the next one is too big for the group start OR the next one is the end
//			if(masterMatCalls[mat].second <= currPos && (mat+1==masterMatCalls.size() ||masterMatCalls[mat+1].second > currPos))
//				matCalls.emplace_back(masterMatCalls[mat].first, 0);
//			else if(masterMatCalls[mat].second > currPos && masterMatCalls[mat].second < currPos + nbGroupVertices)
//				matCalls.emplace_back(masterMatCalls[mat].first, masterMatCalls[mat].second-currPos);
//		}
//		GLuint ebo = 0, vao =0;
//		glGenVertexArrays(1, &vao);
//		glBindVertexArray(vao);
//
//		glGenBuffers(1, &ebo);
//		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo);
//		glBufferData( GL_ELEMENT_ARRAY_BUFFER, nbGroupVertices * sizeof(GLuint), &elements[currPos], GL_STATIC_DRAW);
//
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize*sizeof(float), 0);
//		glEnableVertexAttribArray(0);
//		if(useTextures){
//			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockSize*sizeof(float), (void*)(sizeof(float)*3));
//			glEnableVertexAttribArray(1);
//		}
//		if(useNormals){
//			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, blockSize*sizeof(float), (void*)(sizeof(float)*(useTextures?5:3)));
//			glEnableVertexAttribArray(2);
//		}
//		if(useNormals && useTextures){
//			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, blockSize*sizeof(float), (void*)(sizeof(float)*8));
//			glEnableVertexAttribArray(3);
//			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, blockSize*sizeof(float), (void*)(sizeof(float)*11));
//			glEnableVertexAttribArray(4);
//		}
//		objFile->addMesh(groups[j].first, unique_ptr<Mesh>(new Mesh(objFile, vao, ebo, nbGroupVertices, std::move(matCalls))));
//	}
//
//	return objFile; 
//}