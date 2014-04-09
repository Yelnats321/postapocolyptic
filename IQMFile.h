#pragma once
#include "IQMGeometry.h"
class Graphics;
namespace IQM{
struct iqmanim;
struct iqmpose;
struct iqmmesh
{
    uint32_t name;
    uint32_t material;
    uint32_t first_vertex, num_vertexes;
    uint32_t first_triangle, num_triangles;
};
struct iqmjoint;
struct Matrix3x4;
struct iqmheader;
struct iqmtriangle;
}

class IQMFile{
	IQMFile(std::string);
	static std::map<std::string, std::auto_ptr<IQMFile> > files;
	//Gotta delete these nerds
	const unsigned char *meshdata, *animdata;
	int nummeshes, numtris, numverts, numjoints, numframes, numanims;
	const IQM::iqmmesh *meshes;
	const IQM::iqmjoint *joints;
	/*IQM::iqmpose *poses;
	IQM::iqmanim *anims;
	IQM::Matrix3x4 *frames;*/
	//some offset shit
	float * inposition;
	IQM::iqmtriangle * tris;
	std::vector<GLuint> textures;
	std::vector<IQM::Matrix3x4> outframe, baseframe, inversebaseframe;
	GLuint ebo, vbo, vao;
	void loadmeshes(std::string & filename, const IQM::iqmheader &hdr);
	//void loadanims(std::string & filename, const IQM::iqmheader &hdr);
public:
	~IQMFile();
	//void animate(float);
	int getNumMeshes() const;
	int getNumTris() const;
	std::array<glm::vec3,  3> getTriangle(int) const;
	GLuint getVao() const;
	const IQM::iqmmesh & getMesh(int) const;
	GLuint getTexture(int) const;
	static const IQMFile * openFile(const std::string&);
	//remove this later
	GLuint getVbo() const;
	int getNumVerts() const;
};