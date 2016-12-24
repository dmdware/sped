

#include "modelholder.h"
#include "../render/vertexarray.h"
#include "../tool/rendersprite.h"
#include "../math/brush.h"
#include "../math/quaternion.h"
#include "../math/vec4f.h"
#include "compilemap.h"
#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../tool/rendertopo.h"

std::list<ModelHolder> g_modelholder;

ModelHolder::ModelHolder()
{
	//nframes = 0;
	//frames = NULL;
	modeli = -1;
	//model.m_fullpath = "";
}

void VAsMinMax(VertexArray** frames, int nframes, Vec3f* pvmin, Vec3f* pvmax)
{
	Vec3f vmin(0,0,0);
	Vec3f vmax(0,0,0);

	for(int i=0; i<nframes; i++)
	{
		VertexArray* frame = &(*frames)[i];

		for(int vertidx = 0; vertidx < frame->numverts; vertidx++)
		{
			Vec3f v = frame->vertices[vertidx];

			if(v.x < vmin.x)
				vmin.x = v.x;
			if(v.y < vmin.y)
				vmin.y = v.y;
			if(v.z < vmin.z)
				vmin.z = v.z;
			if(v.x > vmax.x)
				vmax.x = v.x;
			if(v.y > vmax.y)
				vmax.y = v.y;
			if(v.z > vmax.z)
				vmax.z = v.z;
		}
	}

	*pvmin = vmin;
	*pvmax = vmax;
}

ModelHolder::ModelHolder(int model, Vec3f pos)
{
	//nframes = 0;
	//frames = NULL;

	this->modeli = model;
	this->model.m_fullpath = g_model2[this->modeli].m_fullpath;
	translation = pos;
	rotdegrees = Vec3f(0,0,0);
	scale = Vec3f(1,1,1);

	retransform();
}

ModelHolder::~ModelHolder()
{
	destroy();
}


ModelHolder::ModelHolder(const ModelHolder& original)
{
	//nframes = 0;
	//frames = NULL;
	*this = original;
}

ModelHolder& ModelHolder::operator=(const ModelHolder &original)
{
#if 0
	int model;
	Vec3f rotdegrees;
	Vec3f translation;
	Vec3f scale;
	Vec3f absmin;
	Vec3f absmax;
	Matrix transform;
	VertexArray* frames;
	int nframes;
#endif

	destroy();
	//model = original.model;
	rotdegrees = original.rotdegrees;
	translation = original.translation;
	scale = original.scale;
	absmin = original.absmin;
	absmax = original.absmax;
	rotationmat = original.rotationmat;
	modeli = original.modeli;
	model.m_pScene = original.model.m_pScene;
	model.m_fullpath = original.model.m_fullpath;
	retransform();
	//CopyVAs(&frames, &nframes, &original.frames, original.nframes);
	//model = original.model;

	return *this;
}

void ModelHolder::retransform()
{
	destroy();

	Model2* m = &g_model2[modeli];

	model.Clear();
	model.m_GlobalInverseTransform = m->m_GlobalInverseTransform;
	model.m_pScene = m->m_pScene;
	model.m_fullpath = m->m_fullpath;
	model.m_Textures = m->m_Textures;
	model.Positions = m->Positions;
	model.TexCoords = m->TexCoords;
	model.Normals = m->Normals;
	model.Indices = m->Indices;
	model.m_BoneInfo = m->m_BoneInfo;
	model.m_BoneMapping = m->m_BoneMapping;
	model.m_Entries = m->m_Entries;
	model.Bones = m->Bones;
	model.m_NumBones = m->m_NumBones;
	//model.InitFromScene(m->m_pScene, m->m_fullpath);

#if 0
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
#endif

	m = &model;

	//Quaternion rotquat;
	Vec3f rotrads;
	rotrads.x = DEGTORAD(rotdegrees.x);
	rotrads.y = DEGTORAD(rotdegrees.y);
	rotrads.z = DEGTORAD(rotdegrees.z);
	//rotquat.fromAngles((float*)&rotrads);
	rotationmat.reset();
	rotationmat.setRotationRadians((float*)&rotrads);

#if 0	//TODO
	for(int frameidx = 0; frameidx < nframes; frameidx++)
	{
		VertexArray* pframe = &frames[frameidx];
		Vec3f normal;

		for(int vertidx = 0; vertidx < pframe->numverts; vertidx++)
		{
			pframe->vertices[vertidx].transform(rotationmat);
			pframe->vertices[vertidx] = pframe->vertices[vertidx] * scale;
		}
	}

	regennormals();

	VAsMinMax(&frames, nframes, &absmin, &absmax);
	absmin = absmin + translation;
	absmax = absmax + translation;
#else

	absmin = Vec3f(0,0,0);
	absmax = Vec3f(0,0,0);

	//gets abs min max and apply transform

	bool minset[3] = {false,false,false};
	bool maxset[3] = {false,false,false};

	for(int i=0; i<model.Positions.size(); i++)
	{
		model.Positions[i].transform(rotationmat);
		model.Positions[i] = model.Positions[i] * scale;

		model.Normals[i].transform(rotationmat);	//hopefully works correctly

		if(!minset[0] || absmin.x > model.Positions[i].x)
		{
			minset[0] = true;
			absmin.x = model.Positions[i].x;
		}

		if(!minset[1] || absmin.y > model.Positions[i].y)
		{
			minset[1] = true;
			absmin.y = model.Positions[i].y;
		}

		if(!minset[2] || absmin.z > model.Positions[i].z)
		{
			minset[2] = true;
			absmin.z = model.Positions[i].z;
		}

		if(!maxset[0] || absmax.x < model.Positions[i].x)
		{
			maxset[0] = true;
			absmax.x = model.Positions[i].x;
		}

		if(!maxset[1] || absmax.y < model.Positions[i].y)
		{
			maxset[1] = true;
			absmax.y = model.Positions[i].y;
		}

		if(!maxset[2] || absmax.z < model.Positions[i].z)
		{
			maxset[2] = true;
			absmax.z = model.Positions[i].z;
		}
	}

	absmin = absmin + translation;
	absmax = absmax + translation;

	//regennormals();
#endif
}

void ModelHolder::regennormals()
{
	//TODO
#if 0
	Model* m = &g_model[model];
	MS3DModel* ms3d = &m->m_ms3d;

	std::vector<Vec3f>* normalweights;

	normalweights = new std::vector<Vec3f>[ms3d->m_numVertices];

	for(int f = 0; f < nframes; f++)
	{
		for(int index = 0; index < ms3d->m_numVertices; index++)
		{
			normalweights[index].clear();
		}

		Vec3f* vertices = frames[f].vertices;
		//Vec2f* texcoords = frames[f].texcoords;
		Vec3f* normals = frames[f].normals;

		int vert = 0;

		for(int i = 0; i < ms3d->m_numMeshes; i++)
		{
			for(int j = 0; j < ms3d->m_pMeshes[i].m_numTriangles; j++)
			{
				int triangleIndex = ms3d->m_pMeshes[i].m_pTriangleIndices[j];
				const MS3DModel::Triangle* pTri = &ms3d->m_pTriangles[triangleIndex];

				Vec3f normal;
				Vec3f tri[3];
				tri[0] = vertices[vert+0];
				tri[1] = vertices[vert+1];
				tri[2] = vertices[vert+2];
				//normal = Normal2(tri);
				normal = Normal(tri);	//Reverse order
				//normals[i] = normal;
				//normals[i+1] = normal;
				//normals[i+2] = normal;

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->m_vertexIndices[k];
					normalweights[index].push_back(normal);

					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;
				}
			}
		}

		vert = 0;

		for(int i = 0; i < ms3d->m_numMeshes; i++)
		{
			for(int j = 0; j < ms3d->m_pMeshes[i].m_numTriangles; j++)
			{
				int triangleIndex = ms3d->m_pMeshes[i].m_pTriangleIndices[j];
				const MS3DModel::Triangle* pTri = &ms3d->m_pTriangles[triangleIndex];

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->m_vertexIndices[k];

					Vec3f weighsum(0, 0, 0);

					for(int l=0; l<normalweights[index].size(); l++)
					{
						weighsum = weighsum + normalweights[index][l] / (float)normalweights[index].size();
					}

					normals[vert] = weighsum;

					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;
				}
			}
		}
	}

	delete [] normalweights;
#endif
}

void ModelHolder::destroy()
{
#if 0
	nframes = 0;

	if(frames)
	{
		delete [] frames;
		frames = NULL;
	}
#endif

	model.destroy();
}

void ModelHolder::addclipmesh(Surf *surf)
{
	int lastvi = surf->pts.size();

	if(g_renderframes <= 0)
		g_renderframes = 1;

	return model.addclipmesh(surf,
		(g_renderframe % g_renderframes),
		translation, modeli,
		scale, rotationmat);
}

Vec3f ModelHolder::traceray(Vec3f line[], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a,
		unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa,
		unsigned char *nr, unsigned char *ng, unsigned char *nb, unsigned char *na,
		Texture **retex, Texture **retexs, Texture **retexn,
		Vec2f *retexc,
							Vec3f* nearnorm, bool exact)
{
#if 1
	if(!exact)
	{
		Vec3f planenorms[6];
		float planedists[6];
		MakeHull(planenorms, planedists, Vec3f(0,0,0), absmin, absmax);

	#if 0
		for(int i=0; i<6; i++)
		{
			Log("mh pl ("<<planenorms[i].x<<","<<planenorms[i].y<<","<<planenorms[i].z<<"),"<<planedists[i]<<std::endl;
		}
	#endif

		if(LineInterHull(line, planenorms, planedists, 6))
		{
			return (absmin+absmax)/2.0f;
		}
		return line[1];
	}
#endif

	if(g_renderframes <= 0)
		g_renderframes = 1;

	return model.TraceRay((g_renderframe % g_renderframes),
		translation, modeli,
		scale, rotationmat,
		line,
		r,g,b,a, 
		sr,sg,sb,sa,
		nr,ng,nb,na,
		retex, retexs, retexn,
		retexc,
		nearnorm);

	//return line[1];
}

void FreeModelHolders()
{
	g_modelholder.clear();
}

void DrawModelHolders()
{
	for(std::list<ModelHolder>::iterator iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model2* m = &g_model2[h->modeli];

#if 0 //TODO
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->usedifftex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->usespectex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->usenormtex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->useteamtex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//DrawVA(&m->m_va[rand()%10], h->translation);
		DrawVA(&h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ], h->translation);
#ifdef DEBUG
		CHECKGLERROR();
#endif
#endif
		int frames = 1;

		if(m->m_pScene->mNumAnimations > 0)
		{
			double duration = m->m_pScene->mAnimations[0]->mDuration;
			double tickspersec = (float)(m->m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			double dframes = duration / tickspersec;
			if(dframes >= 1.0f)
				frames = (int)dframes;
		}
		
		//h->model.Render(g_renderframe % frames, h->translation, h->modeli, h->scale, h->rotationmat);
		h->model.Render(g_renderframe, h->translation, h->modeli, h->scale, h->rotationmat);
	}
}

void DrawModelHoldersDepth()
{
	for(std::list<ModelHolder>::iterator iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model2* m = &g_model2[h->modeli];

#if 0	//TODO
		m->usedifftex();
		//DrawVA(&m->m_va[rand()%10], h->translation);
		DrawVADepth(&h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ], h->translation);
#else

		int frames = 1;

		if(m->m_pScene->mNumAnimations > 0)
		{
			double duration = m->m_pScene->mAnimations[0]->mDuration;
			double tickspersec = (float)(m->m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			double dframes = duration / tickspersec;
			if(dframes >= 1.0f)
				frames = (int)dframes;
		}
		
		//h->model.RenderDepth(g_renderframe % frames, h->translation, h->modeli, h->scale, h->rotationmat);
		h->model.RenderDepth(g_renderframe, h->translation, h->modeli, h->scale, h->rotationmat);
#endif
	}
}
