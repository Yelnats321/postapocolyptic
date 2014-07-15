#include "stdafx.h"
#include "IQMFile.h"
#include "AssetLoader.h"
#include <fstream>
#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2
namespace IQM{
struct iqmheader{
	char magic[16];
	uint32_t version;
	uint32_t filesize;
	uint32_t flags;
	uint32_t num_text, ofs_text;
	uint32_t num_meshes, ofs_meshes;
	uint32_t num_vertexarrays, num_vertexes, ofs_vertexarrays;
	uint32_t num_triangles, ofs_triangles, ofs_adjacency;
	uint32_t num_joints, ofs_joints;
	uint32_t num_poses, ofs_poses;
	uint32_t num_anims, ofs_anims;
	uint32_t num_frames, num_framechannels, ofs_frames, ofs_bounds;
	uint32_t num_comment, ofs_comment;
	uint32_t num_extensions, ofs_extensions;
};



struct vertex{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat tangent[4];
	GLfloat texcoord[2];
	GLubyte blendindex[4];
	GLubyte blendweight[4];
};

struct iqmvertexarray{
	uint32_t type;
	uint32_t flags;
	uint32_t format;
	uint32_t size;
	uint32_t offset;
};

struct iqmtriangle{
	uint32_t vertex[3];
};

enum
{
	IQM_POSITION     = 0,
	IQM_TEXCOORD     = 1,
	IQM_NORMAL       = 2,
	IQM_TANGENT      = 3,
	IQM_BLENDINDEXES = 4,
	IQM_BLENDWEIGHTS = 5,
	IQM_COLOR        = 6,
	IQM_CUSTOM       = 0x10
};

enum
{
	IQM_BYTE   = 0,
	IQM_UBYTE  = 1,
	IQM_SHORT  = 2,
	IQM_USHORT = 3,
	IQM_INT    = 4,
	IQM_UINT   = 5,
	IQM_HALF   = 6,
	IQM_FLOAT  = 7,
	IQM_DOUBLE = 8,
};

struct iqmjoint
{
    uint32_t name;
    int32_t parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmpose
{
    int32_t parent;
    uint32_t mask;
    float channeloffset[10];
    float channelscale[10];
};

struct iqmanim
{
    uint32_t name;
    uint32_t first_frame, num_frames;
    float framerate;
    uint32_t flags;
};
}
using namespace IQM;

std::map<std::string, std::auto_ptr<IQMFile> > IQMFile::files;

const IQMFile * IQMFile::openFile(const std::string & name){
	if(files.find(name) == files.end()){
		files[name] = std::auto_ptr<IQMFile> (new IQMFile(name));
	}
	return files.at(name).get();
}

IQMFile::IQMFile(std::string filename){
	assert(sizeof(IQM::iqmmesh) == sizeof(uint32_t)*6);
	std::ifstream f;
	f.open( filename, std::ios::binary|std::ios::in);
	if(!f) throw;
	IQM::iqmheader hdr;
	f.read((char*)&hdr, sizeof(hdr));
	if(!f.good() || memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic)))
		throw;
	if(hdr.version != IQM_VERSION)
		throw;
	if(hdr.filesize > (16<<20))
		throw;

	meshdata = new unsigned char[hdr.filesize];
	f.read((char*)meshdata+sizeof(hdr), hdr.filesize-sizeof(hdr));
	try{
		loadmeshes(filename, hdr);
		//setScale(0.3, 0.3, 0.3);
		//setPosition(0.5,0,0.5);
		//glm::mat4 M =  glm::rotate(glm::mat4(),90.f, glm::vec3(-1,0,0));
		//M = glm::rotate(M, 90.f, glm::vec3(0,0,-1));
		//oadanims(filename, hdr, buf);
	}catch(const char * e){
		std::cout<<e;
		throw;
	}
}

IQMFile::~IQMFile(){
	delete[] meshdata;
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vao);
	glDeleteVertexArrays(1, &vao);
}


int IQMFile::getNumMeshes() const{return nummeshes;}
int IQMFile::getNumTris() const{return numtris;}
int IQMFile::getNumVerts() const{return numverts;}
GLuint IQMFile::getVao() const{return vao;}
GLuint IQMFile::getVbo() const{return vbo;}
GLuint IQMFile::getTexture(int pos) const{return textures[pos];}
const IQM::iqmmesh & IQMFile::getMesh(int pos) const{return meshes[pos];}


std::array<glm::vec3, 3> IQMFile::getTriangle(int tri) const{
	std::array<glm::vec3, 3> triangle;/* = {inposition[tris[tri].vertex[0]], inposition[tris[tri].vertex[0]+1], inposition[tris[tri].vertex[0]+2], 
		inposition[tris[tri].vertex[1]], inposition[tris[tri].vertex[1]+1], inposition[tris[tri].vertex[1]+2], 
		inposition[tris[tri].vertex[2]], inposition[tris[tri].vertex[2]+1], inposition[tris[tri].vertex[2]+2]};*/
	//std::cout<< tris[tri].vertex[0] << " " <<tris[tri].vertex[1] <<" " << tris[tri].vertex[2] << std::endl;
	memcpy(&triangle[0], &inposition[tris[tri].vertex[0]*3], sizeof(float)*3);
	memcpy(&triangle[1], &inposition[tris[tri].vertex[1]*3], sizeof(float)*3);
	memcpy(&triangle[2], &inposition[tris[tri].vertex[2]*3], sizeof(float)*3);
	return triangle;
}

// Note that this animates all attributes (position, normal, tangent, bitangent)
// for expository purposes, even though this demo does not use all of them for rendering.
/*void IQMFile::animate(float curframe)
{
	if(numframes <= 0) return;

	int frame1 = (int)floor(curframe),
		frame2 = frame1 + 1;
	float frameoffset = curframe - frame1;
	frame1 %= numframes;
	frame2 %= numframes;
	IQM::Matrix3x4 *mat1 = &frames[frame1 * numjoints],
		*mat2 = &frames[frame2 * numjoints];
	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.
	// You would normally do animation blending and inter-frame blending here in a 3D engine.
	for(int i = 0; i < numjoints; i++)
	{
		IQM::Matrix3x4 mat = mat1[i]*(1 - frameoffset) + mat2[i]*frameoffset;
		if(joints[i].parent >= 0) outframe[i] = outframe[joints[i].parent] * mat;
		else outframe[i] = mat;
	}
}*/

void IQMFile::loadmeshes(std::string & filename, const iqmheader &hdr){
	nummeshes = hdr.num_meshes;
	numtris = hdr.num_triangles;
	numverts = hdr.num_vertexes;
	numjoints = hdr.num_joints;
	outframe.resize(hdr.num_joints);
	textures.resize(hdr.num_meshes,0);

	inposition = NULL; 
	const float *innormal = NULL, *intangent = NULL, *intexcoord = NULL;
	const unsigned char *inblendindex = NULL, *inblendweight = NULL;
	const char *str = hdr.ofs_text ? (char *)&meshdata[hdr.ofs_text] : "";
	const iqmvertexarray *vas = (iqmvertexarray *)&meshdata[hdr.ofs_vertexarrays];
	for(int i = 0; i < (int)hdr.num_vertexarrays; i++)
	{
		const iqmvertexarray &va = vas[i];
		switch(va.type)
		{
		case IQM_POSITION:
			if(va.format != IQM_FLOAT || va.size != 3)
				throw "File format is shit";
			inposition = (float *)&meshdata[va.offset]; 
			break;
		case IQM_NORMAL: 
			if(va.format != IQM_FLOAT || va.size != 3)
				throw "File format is shit";
			innormal = (float *)&meshdata[va.offset];
			break;
		case IQM_TANGENT:
			if(va.format != IQM_FLOAT || va.size != 4) 
				throw "File format is shit";
			intangent = (float *)&meshdata[va.offset];
			break;
		case IQM_TEXCOORD: 
			if(va.format != IQM_FLOAT || va.size != 2)
				throw "File format is shit";
			intexcoord = (float *)&meshdata[va.offset];
			break;
		case IQM_BLENDINDEXES:
			if(va.format != IQM_UBYTE || va.size != 4)
				throw "File format is shit";
			inblendindex = (unsigned char *)&meshdata[va.offset]; 
			break;
		case IQM_BLENDWEIGHTS:
			if(va.format != IQM_UBYTE || va.size != 4) 
				throw "File format is shit";
			inblendweight = (unsigned char *)&meshdata[va.offset];
			break;
		}
	}
	meshes = (iqmmesh *)&meshdata[hdr.ofs_meshes];
	joints = (iqmjoint *)&meshdata[hdr.ofs_joints];

	baseframe.resize(hdr.num_joints);
	inversebaseframe.resize(hdr.num_joints);
	for(int i = 0; i < (int)hdr.num_joints; i++)
	{
		const iqmjoint &j = joints[i];
		baseframe[i] = Matrix3x4(Quat(j.rotate).normalize(), Vec3(j.translate), Vec3(j.scale));
		inversebaseframe[i].invert(baseframe[i]);
		if(j.parent >= 0) 
		{
			baseframe[i] = baseframe[j.parent] * baseframe[i];
			inversebaseframe[i] *= inversebaseframe[j.parent];
		}
	}

	for(int i = 0; i < (int)hdr.num_meshes; i++)
	{
		const iqmmesh &m = meshes[i];
		const std::string path = AssetLoader::getFilePath(filename);
		try{
			textures[i] = TextureManager::loadTexture(path+&str[m.material], false);
		}catch(const char * e){
			std::cout << "Error loading texture \"" << path + &str[m.material] << "\" in file \"" << filename << "\" due to " << e << std::endl;
		}
	}

	tris = (iqmtriangle *)&meshdata[hdr.ofs_triangles];

	vertex *verts = new vertex[hdr.num_vertexes];
	memset(verts, 0, hdr.num_vertexes*sizeof(vertex));
	for(int i = 0; i < (int)hdr.num_vertexes; i++){
		vertex &v = verts[i];
		if(inposition){
			memcpy(v.position, &inposition[i*3], sizeof(v.position));
		}
		if(innormal){
			memcpy(v.normal, &innormal[i*3], sizeof(v.normal));
		}
		if(intangent){
			memcpy(v.tangent, &intangent[i*4], sizeof(v.tangent));
		}
		if(intexcoord){
			memcpy(v.texcoord, &intexcoord[i*2], sizeof(v.texcoord));
		}
		if(inblendindex){
			memcpy(v.blendindex, &inblendindex[i*4], sizeof(v.blendindex));
		}
		if(inblendweight){
			memcpy(v.blendweight, &inblendweight[i*4], sizeof(v.blendweight));
		}
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, hdr.num_vertexes*sizeof(vertex), verts, GL_STATIC_DRAW);
	delete[] verts;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(GLfloat)*(3+3+4)));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, hdr.num_triangles*sizeof(iqmtriangle), tris, GL_STATIC_DRAW);
}

/*void IQMFile::loadanims(std::string & filename, const iqmheader &hdr){
	if((int)hdr.num_poses != numjoints) return;

	if(animdata)
	{
		if(animdata != meshdata) delete[] animdata;
		delete[] frames;
		animdata = NULL;
		anims = NULL;
		frames = 0;
		numframes = 0;
		numanims = 0;
	}        

	animdata = buf;
	numanims = hdr.num_anims;
	numframes = hdr.num_frames;

	const char *str = hdr.ofs_text ? (char *)&buf[hdr.ofs_text] : "";
	anims = (iqmanim *)&buf[hdr.ofs_anims];
	poses = (iqmpose *)&buf[hdr.ofs_poses];
	frames = new Matrix3x4[hdr.num_frames * hdr.num_poses];
	uint16_t *framedata = (uint16_t *)&buf[hdr.ofs_frames];

	for(int i = 0; i < (int)hdr.num_frames; i++)
	{
		for(int j = 0; j < (int)hdr.num_poses; j++)
		{
			iqmpose &p = poses[j];
			Quat rotate;
			Vec3 translate, scale;
			translate.x = p.channeloffset[0]; if(p.mask&0x01) translate.x += *framedata++ * p.channelscale[0];
			translate.y = p.channeloffset[1]; if(p.mask&0x02) translate.y += *framedata++ * p.channelscale[1];
			translate.z = p.channeloffset[2]; if(p.mask&0x04) translate.z += *framedata++ * p.channelscale[2];
			rotate.x = p.channeloffset[3]; if(p.mask&0x08) rotate.x += *framedata++ * p.channelscale[3];
			rotate.y = p.channeloffset[4]; if(p.mask&0x10) rotate.y += *framedata++ * p.channelscale[4];
			rotate.z = p.channeloffset[5]; if(p.mask&0x20) rotate.z += *framedata++ * p.channelscale[5];
			rotate.w = p.channeloffset[6]; if(p.mask&0x40) rotate.w += *framedata++ * p.channelscale[6];
			scale.x = p.channeloffset[7]; if(p.mask&0x80) scale.x += *framedata++ * p.channelscale[7];
			scale.y = p.channeloffset[8]; if(p.mask&0x100) scale.y += *framedata++ * p.channelscale[8];
			scale.z = p.channeloffset[9]; if(p.mask&0x200) scale.z += *framedata++ * p.channelscale[9];
			// Concatenate each pose with the inverse base pose to avoid doing this at animation time.
			// If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
			// Thus it all negates at animation time like so: 
			//   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
			//   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
			//   parentPose * childPose * childInverseBasePose
			Matrix3x4 m(rotate.normalize(), translate, scale);
			if(p.parent >= 0) frames[i*hdr.num_poses + j] = baseframe[p.parent] * m * inversebaseframe[j];
			else frames[i*hdr.num_poses + j] = m * inversebaseframe[j];
		}
	}

	for(int i = 0; i < (int)hdr.num_anims; i++)
	{
		iqmanim &a = anims[i];
		printf("%s: loaded anim: %s\n", filename, &str[a.name]);
	}
}*/