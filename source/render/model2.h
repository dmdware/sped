

#ifndef MODEL2_H
#define MODEL2_H

#include "../platform.h"
#include "ms3d.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "vertexarray.h"

//class VertexArray;
class MS3DModel;
class Shader;

class Material
{
public:
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	unsigned int m_ownerm;
	std::string m_file;
};

class Texture;
class Vec2f;

class Model2
{
public:
	bool m_on;
#if 0
	MS3DModel m_ms3d;
	VertexArray* m_va;
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	unsigned int m_ownerm;
#endif

#if 1
	std::string m_fullpath;
#endif

#define NUM_BONES_PER_VERTEX 4

    struct BoneInfo
    {
        Matrix BoneOffset;
        Matrix FinalTransformation;

        BoneInfo()
        {
            BoneOffset.SetZero();
            FinalTransformation.SetZero();
        }
    };

    struct VertexBoneData
    {
        uint IDs[NUM_BONES_PER_VERTEX];
        float Weights[NUM_BONES_PER_VERTEX];

        VertexBoneData()
        {
            Reset();
        };

        void Reset()
        {
            ZERO_MEM(IDs);
            ZERO_MEM(Weights);
        }

        void AddBoneData(uint BoneID, float Weight);
    };

#define INVALID_MATERIAL 0xFFFFFFFF

enum VB_TYPES {
    INDEX_BUFFER,
    POS_VB,
    NORMAL_VB,
    TEXCOORD_VB,
    BONE_VB,
    NUM_VBs
};

    GLuint m_VAO;
    GLuint m_Buffers[NUM_VBs];

    struct MeshEntry
    {
        MeshEntry()
        {
            NumIndices    = 0;
            BaseVertex    = 0;
            BaseIndex     = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

		unsigned int NumUniqueVerts;
        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<Material> m_Textures;

    std::map<std::string,uint> m_BoneMapping; // maps a bone name to its index
    uint m_NumBones;
    std::vector<BoneInfo> m_BoneInfo;
    Matrix m_GlobalInverseTransform;

	const aiScene* m_pScene;
    Assimp::Importer m_Importer;

	std::vector<Vec3f> Positions;
	std::vector<Vec3f> Normals;
	std::vector<Vec2f> TexCoords;
	std::vector<VertexBoneData> Bones;
	std::vector<uint> Indices;

	Model2();
	~Model2();
	Model2(const Model2& original);
	Model2& operator=(const Model2 &original);

    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const Matrix& ParentTransform);
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(uint MeshIndex,
                  const aiMesh* paiMesh,
                  std::vector<Vec3f>& Positions,
                  std::vector<Vec3f>& Normals,
                  std::vector<Vec2f>& TexCoords,
                  std::vector<VertexBoneData>& Bones,
                  std::vector<unsigned int>& Indices,
					  const aiScene* paiScene);
	void LoadMeshMat(const aiMesh* paiMesh,
					  const aiScene* pScene,
					  uint MeshIndex);
    void LoadBones(uint MeshIndex, const aiMesh* paiMesh, std::vector<VertexBoneData>& Bones);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();
    Vec3f TraceRay(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat,
		Vec3f line[2], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a,
		unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa,
		unsigned char *nr, unsigned char *ng, unsigned char *nb, unsigned char *na,
		Texture **retex, Texture **retexs, Texture **retexn,
		Vec2f *retexc,
		Vec3f* nearnorm);
	bool addclipmesh(Surf *surf,
					  int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat);
    void Render(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat);
    void RenderDepth(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat);
    uint NumBones() const
    {
        return m_NumBones;
    }
    void BoneTransform(float TimeInSeconds, std::vector<Matrix>& Transforms);

	bool load(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue);
#if 0
	void usedifftex();
	void usespectex();
	void usenormtex();
	void useteamtex();
#endif
	void draw(int frame, Vec3f pos, float yaw);
	void drawdepth(int frame, Vec3f pos, float yaw);
	void destroy()
	{
		Clear();
		m_on = false;
	}
};

#define MODELS2	512
extern Model2 g_model2[MODELS2];

int NewModel2();
int FindModel2(const char* relative);
void QueueModel2(int* id, const char* relative, Vec3f scale, Vec3f translate);
int LoadModel2(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue);
void FreeModels2();

#endif
