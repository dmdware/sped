
#ifndef MS3D_H
#define MS3D_H

#pragma warning( disable : 4160 )

#include "../platform.h"
#include "../math/math3d.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../math/matrix.h"

// byte-align structures
#pragma pack(push, 1)

typedef unsigned char byte;
typedef unsigned short word;

// File header
struct MS3DHeader
{
	char m_ID[10];
	int m_version;
};

// Vertex information
struct MS3DVertex
{
	byte m_flags;
	float m_vertex[3];
	char m_boneID;
	byte m_refCount;
};

// Triangle information
struct MS3DTriangle
{
	word m_flags;
	word m_vertexIndices[3];
	float m_vertexNormals[3][3];
	float m_s[3], m_t[3];
	byte m_smoothingGroup;
	byte m_groupIndex;
};

// Material information
struct MS3DMaterial
{
    char m_name[32];
    float m_ambient[4];
    float m_diffuse[4];
    float m_specular[4];
    float m_emissive[4];
    float m_shininess;	// 0.0f - 128.0f
    float m_transparency;	// 0.0f - 1.0f
    byte m_mode;	// 0, 1, 2 is unused now
    char m_diffusem[128];
    char m_alphabldg[128];
};

//	Joint information
struct MS3DJoint
{
	byte m_flags;
	char m_name[32];
	char m_parentName[32];
	float m_rotation[3];
	float m_translation[3];
	word m_numRotationKeyframes;
	word m_numTranslationKeyframes;
};

// Keyframe data
struct MS3DKeyframe
{
	float m_time;
	float m_parameter[3];
};

// Default alignment
#pragma pack(pop)

class VertexArray;

class MS3DModel
{
	public:
		char m_relative[SPE_MAX_PATH+1];
		int m_frame;

		//	Mesh
		struct Mesh
		{
			int m_materialIndex;
			int m_numTriangles;
			int *m_pTriangleIndices;
		};

		//	Material properties
		struct Material
		{
			float m_ambient[4], m_diffuse[4], m_specular[4], m_emissive[4];
			float m_shininess;
			unsigned int m_diffusem;
			char *m_pTextureFilename;
		};

		//	Triangle structure
		struct Triangle
		{
			float m_vertexNormals[3][3];
			float m_s[3], m_t[3];
			int m_vertexIndices[3];
		};

		//	Vertex structure
		struct Vertex
		{
			char m_boneID;	// for skeletal animation
			float m_location[3];
		};

		//	Animation keyframe information
		struct Keyframe
		{
			int m_jointIndex;
			float m_time;	// in milliseconds
			float m_parameter[3];
		};

		//	Skeleton bone joint
		struct Joint
		{
			float m_localRotation[3];
			float m_localTranslation[3];
			Matrix m_absolute, m_relative;

			int m_numRotationKeyframes, m_numTranslationKeyframes;
			Keyframe *m_pTranslationKeyframes;
			Keyframe *m_pRotationKeyframes;

			int m_currentTranslationKeyframe, m_currentRotationKeyframe;
			Matrix m_final;

			int m_parent;
		};

	public:
		MS3DModel();
		~MS3DModel();

		bool load(const char *relative, unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, bool dontqueue);
		void destroy();

		void loadtex(unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, bool dontqueue);
		void genva(VertexArray** vertexArrays, Vec3f scale, Vec3f translate, const char* filepath);

	//protected:
		/*
			Set the values of a particular keyframe for a particular joint.
				jointIndex		The joint to setup the keyframe for
				keyframeIndex	The maximum number of keyframes
				time			The time in milliseconds of the keyframe
				parameter		The rotation/translation values for the keyframe
				isRotation		Whether it is a rotation or a translation keyframe
		*/
		void setjointkf( int jointIndex, int keyframeIndex, float time, float *parameter, bool isRotation );

		//	Setup joint matrices
		void setupjoints();

		//	Advance animation by a frame
		void advanceanim();

		//	Restart animation
		void restart();

		//	Meshes used
		int m_numMeshes;
		Mesh *m_pMeshes;

		//	Materials used
		int m_numMaterials;
		Material *m_pMaterials;

		//	Triangles used
		int m_numTriangles;
		Triangle *m_pTriangles;

		//	Vertices Used
		int m_numVertices;
		Vertex *m_pVertices;

		int m_numJoints;
		Joint *m_pJoints;

		//	Total animation time
		double m_totalTime;
		int m_totalFrames;
};

#endif
