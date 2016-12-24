#include "../platform.h"
#include "../math/math3d.h"
#include "../math/polygon.h"
#include "model2.h"
#include "../texture.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "shader.h"
#include "../debug.h"
#include "vertexarray.h"
#include "shadow.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "../sim/tile.h"
#include "../tool/rendersprite.h"
#include "../tool/rendertopo.h"
#include "../app/appmain.h"
#include "../app/segui.h"

Model2 g_model2[MODELS2];

int NewModel2()
{
	for(int i=0; i<MODELS2; i++)
		if(!g_model2[i].m_on)
			return i;

	return -1;
}

int FindModel2(const char* relative)
{
	char full[SPE_MAX_PATH+1];
	FullPath(relative, full);
	char corrected[SPE_MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);

	for(int i=0; i<MODELS2; i++)
	{
		Model2* m = &g_model2[i];

		if(!m->m_on)
			continue;

		if(stricmp(m->m_fullpath.c_str(), corrected) == 0)
			return i;
	}

	return -1;
}

int LoadModel2(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue)
{
	int i = FindModel2(relative);

	if(i >= 0)
		return i;

	i = NewModel2();

	if(i < 0)
		return i;

	if(g_model2[i].load(relative, scale, translate, dontqueue))
		return i;

	return -1;
}

#define GLCheckError() (glGetError() == GL_NO_ERROR)

void Model2::VertexBoneData::AddBoneData(uint BoneID, float Weight)
{
	for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(IDs) ; i++) {
		if (Weights[i] == 0.0) {
			IDs[i]     = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

Model2::Model2()
{
	m_on = false;
	m_VAO = 0;
	ZERO_MEM(m_Buffers);
	m_NumBones = 0;
	m_pScene = NULL;
}

Model2::~Model2()
{
	Clear();
}


void Model2::Clear()
{
	m_on = false;

	Positions.clear();
	Normals.clear();
	TexCoords.clear();
	Bones.clear();
	Indices.clear();

	//for (uint i = 0 ; i < m_Textures.size() ; i++) {
	//	SAFE_DELETE(m_Textures[i]);
	//}

	m_Textures.clear();

	if (m_Buffers[0] != 0) {
		glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
	}

	if (m_VAO != 0) {
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}

bool Model2::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
	//Clear();

	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);

	Log("Num embedded textures: %d", (int)pScene->mNumTextures);

	uint NumVertices = 0;
	uint NumIndices = 0;

	// Count the number of vertices and indices
	for (uint i = 0 ; i < m_Entries.size() ; i++) {
		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * 3;
		m_Entries[i].BaseVertex    = NumVertices;
		m_Entries[i].BaseIndex     = NumIndices;
		m_Entries[i].NumUniqueVerts = pScene->mMeshes[i]->mNumVertices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices  += m_Entries[i].NumIndices;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Bones.resize(NumVertices);
	//Bones.reserve(NumVertices);
	Indices.reserve(NumIndices);

	// Initialize the meshes in the scene one by one
	for (uint i = 0 ; i < m_Entries.size() ; i++) {
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices, pScene);
	}

#if 0
	if (!InitMaterials(pScene, Filename)) {
		return false;
	}
#endif

#if 0
	// Generate and populate the buffers with vertex attributes and the indices
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
#endif

	return true;
	//return GLCheckError();
}

bool Model2::load(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue)
{

	// Release the previously loaded mesh (if it exists)
	Clear();

	bool result = false;

	char full[SPE_MAX_PATH+1];
	FullPath(relative, full);
	char corrected[SPE_MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);
	m_fullpath = corrected;
	//m_scene = aiImportFile(corrected, aiProcessPreset_TargetRealtime_MaxQuality);

	//std::string rel = MakeRelative(m_fullpath.c_str());

	//InfoMess("m_fullpath", m_fullpath.c_str());
	//InfoMess("rel", rel.c_str());

	// Create the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Create the buffers for the vertices attributes
	glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

	bool Ret = false;

	//Assimp::Importer imp;

#if 1
	m_pScene = m_Importer.ReadFile(corrected,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs);
#endif

#if 0
	aiPropertyStore* props = aiCreatePropertyStore();

	m_pScene = (aiScene*)aiImportFileExWithProperties(corrected,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs,
		NULL,
		props);

	aiReleasePropertyStore(props);
#endif

	if (m_pScene) {
		m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
		m_GlobalInverseTransform.Inverse();
		std::string dir = StripFile(m_fullpath);
		//InfoMess("dir", dir.c_str());
		Ret = InitFromScene(m_pScene, corrected);
		m_on = true;
	}
	else {
		char msg[1280];
		sprintf(msg, "Error parsing '%s': '%s'\n", corrected, m_Importer.GetErrorString());
		ErrMess("ASSIMP Error", msg);
		glBindVertexArray(0);
		return false;
	}


#if 0
	Log("properties: ");

	for(int mi=0; mi<m_pScene->mNumMaterials; mi++)
		for(int pi=0; pi<m_pScene->mMaterials[mi]->mNumProperties; pi++)
		{
			//m_pScene->mMaterials[mi]->mProperties[pi]->
			m_pScene->mMaterials[mi]->mProperties[pi]->mData[m_pScene->mMaterials[mi]->mProperties[pi]->mDataLength-1]=0;

			Log("\t prop"<<pi<<" (l"<<m_pScene->mMaterials[mi]->mProperties[pi]->mDataLength<<"): "<<
				m_pScene->mMaterials[mi]->mProperties[pi]->mKey.data<<" = "<<
				"\""<<m_pScene->mMaterials[mi]->mProperties[pi]->mData<<"\"");
		}
#endif
		

		// Make sure the VAO is not changed from the outside
		glBindVertexArray(0);

		m_on = Ret;

		return Ret;

#if 0
		// Extract the directory part from the file name
		std::string::size_type slashindex = filename.find_last_of("/");
		std::string dir;

		if (SlashIndex == std::string::npos)
		{
			dir = ".";
		}
		else if (SlashIndex == 0)
		{
			dir = "/";
		}
		else
		{
			dir = filename.substr(0, slashindex);
		}
#endif

#if 0
		if(result)
			//if(m_scene)
		{
			m_on = true;
			m_ms3d.genva(&m_va, scale, translate, relative);
			char full[SPE_MAX_PATH+1];
			FullPath(relative, full);
			char corrected[SPE_MAX_PATH+1];
			strcpy(corrected, full);
			CorrectSlashes(corrected);
			m_fullpath = corrected;
		}

		/*
		if(result)
		{
		//CreateTex(spectex, specfile);
		//QueueTexture(&spectex, specfile, true);
		CorrectNormals();
		}*/

		return result;
#endif
}



void Model2::InitMesh(uint MeshIndex,
					  const aiMesh* paiMesh,
					  std::vector<Vec3f>& Positions,
					  std::vector<Vec3f>& Normals,
					  std::vector<Vec2f>& TexCoords,
					  std::vector<VertexBoneData>& Bones,
					  std::vector<uint>& Indices,
					  const aiScene* paiScene)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (uint i = 0 ; i < paiMesh->mNumVertices ; i++) {
		const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(Vec3f(pPos->x, pPos->y, pPos->z));
		Normals.push_back(Vec3f(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(Vec2f(pTexCoord->x, pTexCoord->y));
	}

	LoadBones(MeshIndex, paiMesh, Bones);

	// Populate the index buffer
	for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		//assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}

	LoadMeshMat(paiMesh, paiScene, MeshIndex);
}

void Model2::LoadMeshMat(const aiMesh* paiMesh,
						 const aiScene* pScene,
						 uint MeshIndex)
{

	bool dontqueue = true;
	std::string dir = StripFile(m_fullpath);
	dir = MakeRelative(dir.c_str());

#if 0
	for (uint i = 0 ; i < pScene->mNumMaterials ; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		Material* pmat = &m_Textures[i];

		std::string diffrel;
		std::string specrel;
		std::string normrel;
		std::string teamrel;
		std::string baserel;
#if 0
		char cbasefull[SPE_MAX_PATH+1];
		FullPath(relative, cbasefull);
		basefull = StripFile(std::string(cbasefull));
#endif

		Log("Material #"<<i<<std::endl;


		for (unsigned int pi = 0; pi < pMaterial->mNumProperties;++pi) {
			aiMaterialProperty* prop = pMaterial->mProperties[pi];

			g_applog <<"\tMaterial property: \""<<prop->mKey.data<<"\" = \""<<prop->mData<<"\"");
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			Log("Have diffuse texture");

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Diffuse texture filepath: %s", path.data);

				diffrel = dir + path.data;
				diffrel = MakeRelative(diffrel.c_str());
				//InfoMess("path.data", path.data);
				//InfoMess("diffrel", diffrel.c_str());

#if 0
				if(strstr(relative, ".ms3d") >= 0)
				{
					char cdifffull[SPE_MAX_PATH+1];
					strcpy(cdifffull, path.data);
					StripPath(cdifffull);
					difffull = dir + cdifffull;
				}
#endif
#if 1
				char cbaserel[SPE_MAX_PATH+1];
				sprintf(cbaserel, "%s", diffrel.c_str());
				StripExt(cbaserel);
				baserel = cbaserel;
#endif
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Specular texture filepath: %s", path.data);
				specrel = dir + path.data;
				specrel = MakeRelative(specrel.c_str());
			}
		}
		else
		{
			char cspecrel[SPE_MAX_PATH+1];
			SpecPath(baserel.c_str(), cspecrel);
			specrel = cspecrel;
			Log("Default specular texture filepath: %s", specrel);
		}

		if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Normal texture filepath: %s", path.data);
				normrel = dir + path.data;
				normrel = MakeRelative(normrel.c_str());
			}
		}
		else
		{
			char cnormrel[SPE_MAX_PATH+1];
			NormPath(baserel.c_str(), cnormrel);
			normrel = cnormrel;
			Log("Default normal texture filepath: %s", normrel);
		}

		char cteamrel[SPE_MAX_PATH+1];
		OwnPath(baserel.c_str(), cteamrel);
		teamrel = cteamrel;

		if(dontqueue)
		{
			CreateTex(pmat->m_diffusem, diffrel.c_str(), false, false);
			CreateTex(pmat->m_specularm, specrel.c_str(), false, false);
			CreateTex(pmat->m_normalm, normrel.c_str(), false, false);
			CreateTex(pmat->m_ownerm, teamrel.c_str(), false, false);
		}
		else
		{
			QueueTexture(&pmat->m_diffusem, diffrel.c_str(), false, false);
			QueueTexture(&pmat->m_specularm, specrel.c_str(), false, false);
			QueueTexture(&pmat->m_normalm, normrel.c_str(), false, false);
			QueueTexture(&pmat->m_ownerm, teamrel.c_str(), false, false);
		}
	}
#endif

#if 0
	pmat->m_diffusem = 0;
	pmat->m_specularm = 0;
	pmat->m_normalm = 0;
	pmat->m_ownerm = 0;
#endif

#if 1
	//correct way from assimp_view
	if (paiMesh->mTextureCoords[0])
	{

//		Log("Mesh material "<<paiMesh->mMaterialIndex<<"/"<<pScene->mNumMaterials<<std::endl;

		// extract all properties from the ASSIMP material structure
		const aiMaterial* pcMat = pScene->mMaterials[paiMesh->mMaterialIndex];

		Material* pmat = &m_Textures[paiMesh->mMaterialIndex];

		pmat->m_diffusem = 0;
		pmat->m_specularm = 0;
		pmat->m_normalm = 0;
		pmat->m_ownerm = 0;

		std::string diffrel;
		std::string specrel;
		std::string normrel;
		std::string teamrel;
		std::string baserel;

		aiString szPath;
		aiTextureMapMode mapU, mapV;

		//
		// DIFFUSE TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_DIFFUSE(0),&szPath))
		{
			//FindValidPath(&szPath, dir.c_str());
			//LoadTexture(&pcMesh->piDiffuseTexture,&szPath);

			diffrel = dir + szPath.data;
			//diffrel = MakeRelative(diffrel.c_str());

//			Log("\tDiffuse: "<<szPath.data<<std::endl;

			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0),(int*)&mapU);
			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0),(int*)&mapV);
			//aiTextureMapMode
			//aiTextureMapMode_Wrap;
			//aiTextureMapMode_Clamp;

#if 1
			char cbaserel[SPE_MAX_PATH+1];
			//if(SmartPath((char*)diffrel.c_str(), &diffrel, dir.c_str()))
				;//diffrel = MakeRelative( (dir + diffrel).c_str() );
			sprintf(cbaserel, "%s", diffrel.c_str());
			StripExt(cbaserel);
			baserel = cbaserel;

			if(!TryRelative(diffrel.c_str()))
			{
				Log("Failed: %s", diffrel.c_str());

				diffrel = dir + std::string("tex/") + std::string(szPath.data);
				//if(SmartPath((char*)diffrel.c_str(), &diffrel, dir.c_str()))
					;//diffrel = MakeRelative( (dir + diffrel).c_str() );
				sprintf(cbaserel, "%s", diffrel.c_str());
				StripExt(cbaserel);
				baserel = cbaserel;
				
				if(!TryRelative(baserel.c_str()))
				{
					Log("Failed: %s", diffrel.c_str());

					diffrel = dir + std::string("textures/") + std::string(szPath.data);
					//if(SmartPath((char*)diffrel.c_str(), &diffrel, dir.c_str()))
						;//diffrel = MakeRelative( (dir + diffrel).c_str() );
					sprintf(cbaserel, "%s", diffrel.c_str());
					StripExt(cbaserel);
					baserel = cbaserel;
				}
			}
#endif
//			Log("\tDiffuse after adjustment: "<<diffrel<<std::endl;
		}
		else
		{
//			Log("\tDiffuse not found! ");
		}

		//
		// SPECULAR TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_SPECULAR(0),&szPath))
		{
			//FindValidPath(&szPath, dir.c_str());
			//LoadTexture(&pcMesh->piSpecularTexture,&szPath);
//			Log("\tSpecular texture filepath: " <<szPath.data<<std::endl;
			specrel = dir + szPath.data;
			//specrel = MakeRelative(specrel.c_str());
			//if(SmartPath((char*)specrel.c_str(), &specrel, dir.c_str()))
				;//specrel = MakeRelative( (dir + specrel).c_str() );
			
			if(!TryRelative(specrel.c_str()))
			{
//				Log("Failed: %s", specrel.c_str());

				specrel = dir + std::string("tex/") + std::string(szPath.data);
				//if(SmartPath((char*)specrel.c_str(), &specrel, dir.c_str()))
					;//specrel = MakeRelative( (dir + specrel).c_str() );
				
				if(!TryRelative(specrel.c_str()))
				{
					Log("Failed: %s", specrel.c_str());

					specrel = dir + std::string("textures/") + std::string(szPath.data);
					//if(SmartPath((char*)specrel.c_str(), &specrel, dir.c_str()))
						;//specrel = MakeRelative( (dir + specrel).c_str() );
				}
			}

			Log("\tSpecular texture filepath after adjustment: %s", specrel.c_str());
		}
		else
		{
			char cspecrel[SPE_MAX_PATH+1];
			SpecPath(baserel.c_str(), cspecrel);
			cspecrel[SPE_MAX_PATH] = 0;
			specrel = cspecrel;
			Log("\tDefault specular texture filepath: %s", specrel.c_str());
		}

		//
		// NORMAL/HEIGHT MAP ------------------------------------------------
		//
		bool bHM = false;
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_NORMALS(0),&szPath))
		{
			//FindValidPath(&szPath, dir.c_str());
			//LoadTexture(&pcMesh->piNormalTexture,&szPath);

			//LoadTexture(&pcMesh->piSpecularTexture,&szPath);
//			Log("\tNormal texture filepath: " <<szPath.data<<std::endl;
			normrel = dir + szPath.data;
			//normrel = MakeRelative(normrel.c_str());
			//if(SmartPath((char*)normrel.c_str(), &normrel, dir.c_str()))
				;//normrel = MakeRelative( (dir + normrel).c_str() );
			
			if(!TryRelative(normrel.c_str()))
			{
//				Log("Failed: %s", normrel.c_str());

				normrel = dir + std::string("tex/") + std::string(szPath.data);
				//if(SmartPath((char*)normrel.c_str(), &normrel, dir.c_str()))
					;//specrel = MakeRelative( (dir + specrel).c_str() );
				
				if(!TryRelative(normrel.c_str()))
				{
					Log("Failed: %s", normrel.c_str());

					normrel = dir + std::string("textures/") + std::string(szPath.data);
					//if(SmartPath((char*)normrel.c_str(), &normrel, dir.c_str()))
						;//specrel = MakeRelative( (dir + specrel).c_str() );
				}
			}

			Log("\tNormal texture filepath after adjustment: %s", normrel.c_str());
		}
		else
		{
			char cnormrel[SPE_MAX_PATH+1];
			NormPath(baserel.c_str(), cnormrel);
			cnormrel[SPE_MAX_PATH] = 0;
			normrel = cnormrel;
			Log("\tDefault normal texture filepath: %s", normrel.c_str());
		}

		char cownrel[SPE_MAX_PATH+1];
		OwnPath(baserel.c_str(), cownrel);
		teamrel = cownrel;
		//if(SmartPath((char*)teamrel.c_str(), &teamrel, dir.c_str()))
			;//teamrel = MakeRelative( (dir + teamrel).c_str() );

		if(!TryRelative(teamrel.c_str()))
		{
			Log("Failed: %s", teamrel.c_str());

			teamrel = dir + std::string("tex/") + std::string(szPath.data);
			//if(SmartPath((char*)teamrel.c_str(), &teamrel, dir.c_str()))
				;//specrel = MakeRelative( (dir + specrel).c_str() );

			if(!TryRelative(teamrel.c_str()))
			{
				Log("Failed: %s", teamrel.c_str());

				teamrel = dir + std::string("textures/") + std::string(szPath.data);
				//if(SmartPath((char*)teamrel.c_str(), &teamrel, dir.c_str()))
					;//specrel = MakeRelative( (dir + specrel).c_str() );
			}
		}

		Log("\tDefault team texture filepath: %s", normrel.c_str());

		if(dontqueue)
		{
			CreateTex(pmat->m_diffusem, diffrel.c_str(), false, false);
			CreateTex(pmat->m_specularm, specrel.c_str(), false, false);
			CreateTex(pmat->m_normalm, normrel.c_str(), false, false);
			CreateTex(pmat->m_ownerm, teamrel.c_str(), false, false);
		}
		else
		{
			QueueTexture(&pmat->m_diffusem, diffrel.c_str(), false, false);
			QueueTexture(&pmat->m_specularm, specrel.c_str(), false, false);
			QueueTexture(&pmat->m_normalm, normrel.c_str(), false, false);
			QueueTexture(&pmat->m_ownerm, teamrel.c_str(), false, false);
		}
	}
	else
	{
		//ErrMess("Error", "Model without textures not supported");
		//return;

//		Log("Mesh material "<<paiMesh->mMaterialIndex<<"/"<<pScene->mNumMaterials<<std::endl;

		// extract all properties from the ASSIMP material structure
		const aiMaterial* pcMat = pScene->mMaterials[paiMesh->mMaterialIndex];

		Material* pmat = &m_Textures[paiMesh->mMaterialIndex];

		pmat->m_diffusem = 0;
		pmat->m_specularm = 0;
		pmat->m_normalm = 0;
		pmat->m_ownerm = 0;
	}
#endif
}

void Model2::LoadBones(uint MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
		uint BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		//InfoMess("bone", pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			// Allocate an index for a new bone
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (uint j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
			uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}

	//if(m_NumBones <= 0)
	//	InfoMess("m_NumBones <= 0", "m_NumBones <= 0");
}


bool Model2::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
	// Extract the directory part from the file name
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos) {
		Dir = ".";
	}
	else if (SlashIndex == 0) {
		Dir = "/";
	}
	else {
		Dir = Filename.substr(0, SlashIndex);
	}

	Dir = MakeRelative(Dir.c_str());

	bool Ret = true;

#if 0

	// Initialize the materials
	for (uint i = 0 ; i < pScene->mNumMaterials ; i++) {
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		m_Textures[i] = NULL;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string p(Path.data);

				if (p.substr(0, 2) == ".\\") {
					p = p.substr(2, p.size() - 2);
				}

				std::string FullPath = Dir + "/" + p;

				//TODO load tex
				//m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());
#if 0
				if (!m_Textures[i]->Load()) {
					printf("Error loading texture '%s'\n", FullPath.c_str());
					delete m_Textures[i];
					m_Textures[i] = NULL;
					Ret = false;
				}
				else {
					printf("%d - loaded texture '%s'\n", i, FullPath.c_str());
				}
#endif
			}
		}
	}
#endif


#if 0
	//correct way from assimp_view
	if (pcSource->mTextureCoords[0])
	{

		//
		// DIFFUSE TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_DIFFUSE(0),&szPath))
		{
			LoadTexture(&pcMesh->piDiffuseTexture,&szPath);

			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0),(int*)&mapU);
			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0),(int*)&mapV);
		}

		//
		// SPECULAR TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_SPECULAR(0),&szPath))
		{
			LoadTexture(&pcMesh->piSpecularTexture,&szPath);
		}

		//
		// OPACITY TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_OPACITY(0),&szPath))
		{
			LoadTexture(&pcMesh->piOpacityTexture,&szPath);
		}
		else
		{
			int flags = 0;
			aiGetMaterialInteger(pcMat,AI_MATKEY_TEXFLAGS_DIFFUSE(0),&flags);

			// try to find out whether the diffuse texture has any
			// non-opaque pixels. If we find a few, use it as opacity texture
			if (pcMesh->piDiffuseTexture && !(flags & aiTextureFlags_IgnoreAlpha) && HasAlphaPixels(pcMesh->piDiffuseTexture))
			{
				int iVal;

				// NOTE: This special value is set by the tree view if the user
				// manually removes the alpha texture from the view ...
				if (AI_SUCCESS != aiGetMaterialInteger(pcMat,"no_a_from_d",0,0,&iVal))
				{
					pcMesh->piOpacityTexture = pcMesh->piDiffuseTexture;
					pcMesh->piOpacityTexture->AddRef();
				}
			}
		}

		//
		// AMBIENT TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_AMBIENT(0),&szPath))
		{
			LoadTexture(&pcMesh->piAmbientTexture,&szPath);
		}

		//
		// EMISSIVE TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_EMISSIVE(0),&szPath))
		{
			LoadTexture(&pcMesh->piEmissiveTexture,&szPath);
		}

		//
		// Shininess TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_SHININESS(0),&szPath))
		{
			LoadTexture(&pcMesh->piShininessTexture,&szPath);
		}

		//
		// Lightmap TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_LIGHTMAP(0),&szPath))
		{
			LoadTexture(&pcMesh->piLightmapTexture,&szPath);
		}


		//
		// NORMAL/HEIGHT MAP ------------------------------------------------
		//
		bool bHM = false;
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_NORMALS(0),&szPath))
		{
			LoadTexture(&pcMesh->piNormalTexture,&szPath);
		}
		else
		{
			if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_HEIGHT(0),&szPath))
			{
				LoadTexture(&pcMesh->piNormalTexture,&szPath);
			}
			else bib = true;
			bHM = true;
		}

		// normal/height maps are sometimes mixed up. Try to detect the type
		// of the texture std::list<Brush>::iteratormatically
		if (pcMesh->piNormalTexture)
		{
			HMtoNMIfNecessary(pcMesh->piNormalTexture, &pcMesh->piNormalTexture,bHM);
		}
	}
#endif

	bool dontqueue = true;
	std::string dir = StripFile(m_fullpath);

#if 1
	for (uint i = 0 ; i < pScene->mNumMaterials ; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		Material* pmat = &m_Textures[i];

		std::string diffrel;
		std::string specrel;
		std::string normrel;
		std::string teamrel;
		std::string baserel;
#if 0
		char cbasefull[SPE_MAX_PATH+1];
		FullPath(relative, cbasefull);
		basefull = StripFile(std::string(cbasefull));
#endif

//		Log("Material #"<<i<<std::endl;


		for (unsigned int pi = 0; pi < pMaterial->mNumProperties;++pi) {
			aiMaterialProperty* prop = pMaterial->mProperties[pi];

//			g_applog <<"\tMaterial property: \""<<prop->mKey.data<<"\" = \""<<prop->mData<<"\"");
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			Log("Have diffuse texture");

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Diffuse texture filepath: %s", path.data);

				diffrel = dir + path.data;
				diffrel = MakeRelative(diffrel.c_str());
				//InfoMess("path.data", path.data);
				//InfoMess("diffrel", diffrel.c_str());

#if 0
				if(strstr(relative, ".ms3d") >= 0)
				{
					char cdifffull[SPE_MAX_PATH+1];
					strcpy(cdifffull, path.data);
					StripPath(cdifffull);
					difffull = dir + cdifffull;
				}
#endif
#if 1
				char cbaserel[SPE_MAX_PATH+1];
				sprintf(cbaserel, "%s", diffrel.c_str());
				cbaserel[SPE_MAX_PATH] = 0;
				StripExt(cbaserel);
				baserel = cbaserel;
#endif
			}
		}
		else
		{
			ErrMess("Error", "Material without diffuse texture! Choose a texture...");

			char path[SPE_MAX_PATH+1];
			char initdir[SPE_MAX_PATH+1];
			FullPath("textures/", initdir);

			if(!OpenFileDialog(initdir, path))
			{
				ErrMess("Error", "No texture chosen");
				return false;
			}

			diffrel = path;
			diffrel = MakeRelative(diffrel.c_str());
			//InfoMess("path.data", path.data);
			//InfoMess("diffrel", diffrel.c_str());

#if 0
			if(strstr(relative, ".ms3d") >= 0)
			{
				char cdifffull[SPE_MAX_PATH+1];
				strcpy(cdifffull, path.data);
				StripPath(cdifffull);
				difffull = dir + cdifffull;
			}
#endif
#if 1
			char cbaserel[SPE_MAX_PATH+1];
			sprintf(cbaserel, "%s", diffrel.c_str());
			StripExt(cbaserel);
			baserel = cbaserel;
#endif
		}

		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Specular texture filepath: %s", path.data);
				specrel = dir + path.data;
				specrel = MakeRelative(specrel.c_str());
			}
		}
		else
		{
			char cspecrel[SPE_MAX_PATH+1];
			SpecPath(baserel.c_str(), cspecrel);
			cspecrel[SPE_MAX_PATH] = 0;
			specrel = cspecrel;
			Log("Default specular texture filepath: %s", specrel.c_str());
		}

		if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Normal texture filepath: %s", path.data);
				normrel = dir + path.data;
				normrel = MakeRelative(normrel.c_str());
			}
		}
		else
		{
			char cnormrel[SPE_MAX_PATH+1];
			NormPath(baserel.c_str(), cnormrel);
			cnormrel[SPE_MAX_PATH] = 0;
			normrel = cnormrel;
			Log("Default normal texture filepath: %s", normrel.c_str());
		}

		char cteamrel[SPE_MAX_PATH+1];
		OwnPath(baserel.c_str(), cteamrel);
		teamrel = cteamrel;

		if(dontqueue)
		{
			CreateTex(pmat->m_diffusem, diffrel.c_str(), false, false);
			CreateTex(pmat->m_specularm, specrel.c_str(), false, false);
			CreateTex(pmat->m_normalm, normrel.c_str(), false, false);
			CreateTex(pmat->m_ownerm, teamrel.c_str(), false, false);
		}
		else
		{
			QueueTexture(&pmat->m_diffusem, diffrel.c_str(), false, false);
			QueueTexture(&pmat->m_specularm, specrel.c_str(), false, false);
			QueueTexture(&pmat->m_normalm, normrel.c_str(), false, false);
			QueueTexture(&pmat->m_ownerm, teamrel.c_str(), false, false);
		}
	}
#endif

	return Ret;
}

bool Model2::addclipmesh(Surf *surf,
					  int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat)
{
	int lastvi = surf->pts.size();


	Matrix modelmat;
	modelmat.setTranslation((const float*)&pos);
	////glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	//rotmat.postmult(modelmat);
	//Matrix modelmat2;
	//modelmat2 = modelmat;
	///modelmat2.postmult(rotmat);
	///rotmat = modelmat2;
 
	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	/////glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	//////glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	//perform frame transformation on-the-fly
	std::vector<Matrix> BoneTransforms;

	//if(!m_pScene)
	//	ErrMess("No scene", "No scene");

#if 1
	if(m_pScene->mNumAnimations > 0)
		//if(0)
	{
		float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float frames = TicksPerSecond * (float)m_pScene->mAnimations[0]->mDuration;
		float percentage = (float)frame / frames;
		float RunningTime = percentage * (float)m_pScene->mAnimations[0]->mDuration;

		BoneTransform(RunningTime, BoneTransforms);

		//Log("bone transform ");
		//Log("m_pScene->mAnimations[0]->mTicksPerSecond "<<m_pScene->mAnimations[0]->mTicksPerSecond<<std::endl;
		//Log("m_pScene->mAnimations[0]->mDuration "<<m_pScene->mAnimations[0]->mDuration<<std::endl;
		//Log("RunningTime "<<RunningTime<<std::endl;
		//Log("BoneTransforms.size() "<<BoneTransforms.size()<<std::endl;
	}
	else
	{
		BoneTransforms.resize( m_BoneInfo.size() );

		for(int i=0; i<m_BoneInfo.size(); i++)
		{
			BoneTransforms[i].InitIdentity();
		}
	}
#else
	//BoneTransform(0, BoneTransforms);
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;
	//Log("m_NumBones = "<<m_NumBones<<std::endl;
#endif

	//Log("m_NumBones = "<<m_NumBones<<std::endl;
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;

	std::vector<Vec3f> TransformedPos;
	std::vector<Vec3f> TransformedNorm;
	TransformedPos.resize(Positions.size());
	TransformedNorm.resize(Normals.size());

	Model2* origm = &g_model2[origmodeli];

	for(uint i=0; i<Positions.size(); i++)
	{
		Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
		Transform.InitIdentity();

		//bool influenced = false;

		for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
			//for(int bi=0; bi<1; bi++)
		{
			if(Bones[i].IDs[bi] < 0)
				continue;

			if(Bones[i].Weights[bi] == 0.0f)
				continue;

			if(bi == 0)
				Transform = BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
			else
				Transform += BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
		}

		Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
		Transformed.transform(Transform);
		Transformed.transform(rotmat);
		//Transformed.w = 1;
		//Transformed.transform3(modelmat);
		//Transformed.x = Transformed.x * scale.x;
		//Transformed.y = Transformed.y * scale.y;
		//Transformed.z = Transformed.z * scale.z;
		//Transformed = Transformed * scale;
		TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z);
		TransformedPos[i]    = TransformedPos[i] * scale + pos;

		static int lasti = -1;
		//if(i>lasti)
//fprintf(g_applog, "\tpos[%d]=%f,%f,%f\r\n", (int)i, TransformedPos[i].x,TransformedPos[i].y,TransformedPos[i].z);
//fflush(g_applog);

lasti=(i>lasti)?i:lasti;

		TransformedNorm[i] = origm->Normals[i];
		TransformedNorm[i].transform(Transform);
		TransformedNorm[i].transform(rotmat);
		//TransformedNorm[i].transform(modelmat);
	}

	//If we're including inclines, adjust vertex heights
	if((g_mode == RENDERING || g_mode == PREREND_ADJFRAME) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0] * TILE_RISE;
		heights[1] = g_cornerinc[g_currincline][1] * TILE_RISE;
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2] * TILE_RISE;
		heights[3] = g_cornerinc[g_currincline][3] * TILE_RISE;

		Heightmap hm;
		hm.allocate(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();

		//TODO need to take into account rotation matrix also
		for(uint i=0; i<TransformedPos.size(); i++)
		{
			//TransformedPos[i].y += Bilerp(&hm,
			//	g_tilesize/2.0f + pos.x + TransformedPos[i].x,
			//	g_tilesize/2.0f + pos.z + TransformedPos[i].z);
			TransformedPos[i].y += hm.accheight(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z);
		}

		//Regenerate normals:
		//Not possible, based on vertices alone, because we would also need to blend shared faces,
		//so leave this inaccuracy for now. TODO
	}

	for (uint i = 0 ; i < m_Entries.size() ; i++)
	{
		const uint MaterialIndex = m_Entries[i].MaterialIndex;

		if(MaterialIndex < m_Textures.size() && m_Textures.size())
		{
			Material* mat = &m_Textures[MaterialIndex];

		}
		else
		{
			static unsigned int notex = 0;
		}

		const unsigned int numindices = m_Entries[i].NumIndices;
		const unsigned int basevertex = m_Entries[i].BaseVertex;
		const unsigned int baseindex = m_Entries[i].BaseIndex;
		const unsigned int numunique = m_Entries[i].NumUniqueVerts;

		int tv=0;

		Vec3f tr[3];
		Vec3f tr2[3];
		Vec2f txc[3];
		Vec3f vnorm[3];
		for(int inin=0; inin<numindices; ++inin)
		{
			unsigned int vin = Indices[baseindex+inin];
			tr[tv] = TransformedPos[basevertex + vin];
			tr2[2-tv] = TransformedPos[basevertex + vin];
			txc[tv] = TexCoords[basevertex + vin];
			vnorm[tv] = TransformedNorm[basevertex + vin];

			tv=(tv+1)%3;

			if(!tv)
			{
				SurfPt* lastvs[3];
				lastvs[0] = new SurfPt;
				lastvs[1] = new SurfPt;
				lastvs[2] = new SurfPt;
				Tet* tet = new Tet;

				tet->approved = false;
				tet->hidden = false;

				surf->pts2.push_back(lastvs[0]);
				surf->pts2.push_back(lastvs[1]);
				surf->pts2.push_back(lastvs[2]);
				surf->tets2.push_back(tet);

				tet->neib[0] = lastvs[0];
				tet->neib[1] = lastvs[1];
				tet->neib[2] = lastvs[2];
				tet->neib[3] = NULL;

				lastvs[0]->pos = tr[0];
				lastvs[1]->pos = tr[1];
				lastvs[2]->pos = tr[2];
				
				lastvs[0]->texc = txc[0];
				lastvs[1]->texc = txc[1];
				lastvs[2]->texc = txc[2];
				
				lastvs[0]->norm = vnorm[0];
				lastvs[1]->norm = vnorm[1];
				lastvs[2]->norm = vnorm[2];

				lastvs[0]->holder.push_back(tet);
				lastvs[1]->holder.push_back(tet);
				lastvs[2]->holder.push_back(tet);

				Texture *diff = NULL;
				Texture *spec = NULL;
				Texture *norm = NULL;

				if(MaterialIndex < m_Textures.size() && m_Textures.size())
				{
					Material* dmat = &m_Textures[MaterialIndex];
					diff = &g_texture[ dmat->m_diffusem ];

					Material* smat = &m_Textures[MaterialIndex];
					spec = &g_texture[ smat->m_specularm ];

					Material* nmat = &m_Textures[MaterialIndex];
					norm = &g_texture[ nmat->m_normalm ];
				}
				else
				{
					//static unsigned int notex = 0;

					//if(!notex)
					//	CreateTex(notex, "textures/notex.jpg", false, true);

					//diff = &g_texture[ notex ];
					continue;
				}

				//LoadedTex* difpx = LoadTexture(diff->fullpath.c_str());
				//LoadedTex* difpx = diff->pixels;
				//LoadedTex* specpx = spec->pixels;
				//LoadedTex* normpx = norm->pixels;

				//if(!difpx)
				//	continue;

				tet->tex = diff;
				tet->stex = spec;
				tet->ntex = norm;
			}
		}
	}

	// Make sure the VAO is not changed from the outside
	//glBindVertexArray(0);
	return true;
}

Vec3f Model2::TraceRay(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat,
		Vec3f line[2], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a,
		unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa,
		unsigned char *nr, unsigned char *ng, unsigned char *nb, unsigned char *na,
		Texture **retex, Texture **retexs, Texture **retexn,
		Vec2f *retexc,
		Vec3f* nearnorm)
{
	//glBindVertexArray(m_VAO);
	//Log("entries "<<m_Entries.size()<<std::endl;

//	return Vec3f();

	//float nearestd = Magnitude(line[1] - line[0]);
	//Vec3f nearestp = line[1];

	
			//	if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!!2","!23!");

	Vec3f nearestv = line[1];
	float nearestd = Magnitude(nearestv-line[0]);

	Shader* s = &g_shader[g_curS];

	//Vec3f pos(0,0,0);

	Matrix modelmat;
	modelmat.setTranslation((const float*)&pos);
	////glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	//rotmat.postmult(modelmat);
	//Matrix modelmat2;
	//modelmat2 = modelmat;
	///modelmat2.postmult(rotmat);
	///rotmat = modelmat2;
 
	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	/////glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	//////glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	//perform frame transformation on-the-fly
	std::vector<Matrix> BoneTransforms;

	//if(!m_pScene)
	//	ErrMess("No scene", "No scene");

#if 1
	if(m_pScene->mNumAnimations > 0)
		//if(0)
	{
		float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float frames = TicksPerSecond * (float)m_pScene->mAnimations[0]->mDuration;
		float percentage = (float)frame / frames;
		float RunningTime = percentage * (float)m_pScene->mAnimations[0]->mDuration;

		BoneTransform(RunningTime, BoneTransforms);

		//Log("bone transform ");
		//Log("m_pScene->mAnimations[0]->mTicksPerSecond "<<m_pScene->mAnimations[0]->mTicksPerSecond<<std::endl;
		//Log("m_pScene->mAnimations[0]->mDuration "<<m_pScene->mAnimations[0]->mDuration<<std::endl;
		//Log("RunningTime "<<RunningTime<<std::endl;
		//Log("BoneTransforms.size() "<<BoneTransforms.size()<<std::endl;
	}
	else
	{
		BoneTransforms.resize( m_BoneInfo.size() );

		for(int i=0; i<m_BoneInfo.size(); i++)
		{
			BoneTransforms[i].InitIdentity();
		}
	}
#else
	//BoneTransform(0, BoneTransforms);
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;
	//Log("m_NumBones = "<<m_NumBones<<std::endl;
#endif

	//Log("m_NumBones = "<<m_NumBones<<std::endl;
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;

	std::vector<Vec3f> TransformedPos;
	std::vector<Vec3f> TransformedNorm;
	TransformedPos.resize(Positions.size());
	TransformedNorm.resize(Normals.size());

	Model2* origm = &g_model2[origmodeli];

	for(uint i=0; i<Positions.size(); i++)
	{
		Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
		Transform.InitIdentity();

		//bool influenced = false;

		for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
			//for(int bi=0; bi<1; bi++)
		{
			if(Bones[i].IDs[bi] < 0)
				continue;

			if(Bones[i].Weights[bi] == 0.0f)
				continue;

			if(bi == 0)
				Transform = BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
			else
				Transform += BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
		}

		Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
		Transformed.transform(Transform);
		Transformed.transform(rotmat);
		//Transformed.w = 1;
		//Transformed.transform3(modelmat);
		//Transformed.x = Transformed.x * scale.x;
		//Transformed.y = Transformed.y * scale.y;
		//Transformed.z = Transformed.z * scale.z;
		//Transformed = Transformed * scale;
		TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z);
		TransformedPos[i]    = TransformedPos[i] * scale + pos;

		static int lasti = -1;
		//if(i>lasti)
//fprintf(g_applog, "\tpos[%d]=%f,%f,%f\r\n", (int)i, TransformedPos[i].x,TransformedPos[i].y,TransformedPos[i].z);
//fflush(g_applog);

lasti=(i>lasti)?i:lasti;

		TransformedNorm[i] = origm->Normals[i];
		TransformedNorm[i].transform(Transform);
		TransformedNorm[i].transform(rotmat);
		//TransformedNorm[i].transform(modelmat);
	}

	//If we're including inclines, adjust vertex heights
	if((g_mode == RENDERING || g_mode == PREREND_ADJFRAME) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0] * TILE_RISE;
		heights[1] = g_cornerinc[g_currincline][1] * TILE_RISE;
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2] * TILE_RISE;
		heights[3] = g_cornerinc[g_currincline][3] * TILE_RISE;

		Heightmap hm;
		hm.allocate(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();

		//TODO need to take into account rotation matrix also
		for(uint i=0; i<TransformedPos.size(); i++)
		{
			//TransformedPos[i].y += Bilerp(&hm,
			//	g_tilesize/2.0f + pos.x + TransformedPos[i].x,
			//	g_tilesize/2.0f + pos.z + TransformedPos[i].z);
			TransformedPos[i].y += hm.accheight(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z);
		}

		//Regenerate normals:
		//Not possible, based on vertices alone, because we would also need to blend shared faces,
		//so leave this inaccuracy for now. TODO
	}

	for (uint i = 0 ; i < m_Entries.size() ; i++)
	{
		const uint MaterialIndex = m_Entries[i].MaterialIndex;

		if(MaterialIndex < m_Textures.size() && m_Textures.size())
		{
			Material* mat = &m_Textures[MaterialIndex];

		}
		else
		{
			static unsigned int notex = 0;
		}

		const unsigned int numindices = m_Entries[i].NumIndices;
		const unsigned int basevertex = m_Entries[i].BaseVertex;
		const unsigned int baseindex = m_Entries[i].BaseIndex;
		const unsigned int numunique = m_Entries[i].NumUniqueVerts;

		int tv=0;

		Vec3f tr[3];
		Vec3f tr2[3];
		Vec2f txc[3];
		for(int inin=0; inin<numindices; ++inin)
		{
			unsigned int vin = Indices[baseindex+inin];
			tr[tv] = TransformedPos[basevertex + vin];
			tr2[2-tv] = TransformedPos[basevertex + vin];
			txc[tv] = TexCoords[basevertex + vin];
	
			tv=(tv+1)%3;

			if(!tv)
			{
				Vec3f ir;
				Vec4f rgba;
				Vec3f ratp[3];

				Vec3f opline[2];
				opline[0] = line[1];
				opline[1] = line[0];
#if 0
				if(line[0] == Vec3f(0,-30000,30000))
				{
					if(!IntersectedPolygon(tr, line, 3, &ir) &&
						IntersectedPolygon(tr, opline, 3, &ir) )
						{
							ErrMess("!!", "!!");
						}
				}
#endif
				//Vec3f(0,-30000,30000)

				Vec3f ir2 = ir;

				//if(!IntersectedPolygon(tr, line, 3, &ir2) &&
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!!2","!2!");
/*
					fprintf(g_applog, " t\r\n (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)",
							tr[0].x, tr[0].y, tr[0].z,
							tr[1].x, tr[1].y, tr[1].z,
							tr[2].x, tr[2].y, tr[2].z);
*/
				if(IntersectedPolygon(tr, line, 3, &ir) ||
					IntersectedPolygon(tr2, line, 3, &ir)
					)
				{
					//if(line[0] == Vec3f(0,-30000,30000))
					//	ErrMess("!!","!!");
#if 0
					fprintf(g_applog, "\r\nint t\r\n (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)",
							tr[0].x, tr[0].y, tr[0].z,
							tr[1].x, tr[1].y, tr[1].z,
							tr[2].x, tr[2].y, tr[2].z);

					if(line[0].y < 0 && line[0].z > 0)
					{
						char m[123];
						sprintf(m,"\r\nasdasd\r\n (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)",
							tr[0].x, tr[0].y, tr[0].z,
							tr[1].x, tr[1].y, tr[1].z,
							tr[2].x, tr[2].y, tr[2].z);
						ErrMess(m,m);
					}
#endif
					float newd = Magnitude(ir - line[0]);

					if(newd < nearestd)
					{
						//fprintf(g_applog,
						//	"newd<nearestd %f,%f",
						//	newd, nearestd);

						Texture *diff;
						Texture *spec;
						Texture *norm;
							
						if(MaterialIndex < m_Textures.size() && m_Textures.size())
						{
							Material* dmat = &m_Textures[MaterialIndex];
							diff = &g_texture[ dmat->m_diffusem ];
							
							Material* smat = &m_Textures[MaterialIndex];
							spec = &g_texture[ smat->m_specularm ];
							
							Material* nmat = &m_Textures[MaterialIndex];
							norm = &g_texture[ nmat->m_normalm ];
						}
						else
						{
							//static unsigned int notex = 0;

							//if(!notex)
							//	CreateTex(notex, "textures/notex.jpg", false, true);

							//diff = &g_texture[ notex ];
							continue;
						}

						//LoadedTex* difpx = LoadTexture(diff->fullpath.c_str());
						LoadedTex* difpx = diff->pixels;
						LoadedTex* specpx = spec->pixels;
						LoadedTex* normpx = norm->pixels;
						
						if(!difpx)
							continue;

#if 1
					//	http://gamedev.stackexchange.com/questions/62651/finding-pixels-within-uv-coordinates

						// compute vectors
						Vec3f v0 = tr[1] - tr[0], 
							v1 = tr[2] - tr[0],
							v2 = ir - tr[0];

						// do bounds test for each position
						double f00 = Dot( v0, v0 );
						double f01 = Dot( v0, v1 );
						double f11 = Dot( v1, v1 );

						double f02 = Dot( v0, v2 );
						double f12 = Dot( v1, v2 );

						// Compute barycentric coordinates
						double invDenom = 1 / ( f00 * f11 - f01 * f01 );
						double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
						double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

						// Check if point is in triangle
						///if( ( fU >= 0 ) && ( fV >= 0 ) && ( fU + fV <= 1 ) )
						///	goto dotex;
						//	return true;
						
						///continue;
dotex:
						// TODO check transparency pass
						
						*retex = diff;
						*retexs = spec;
						*retexn = norm;

						nearestd = newd;
						nearestv = ir;
						line[1] = nearestv;

						float ffU = txc[0].x * (1 - fU - fV) + 
							txc[1].x * (fU) + 
							txc[2].x * (fV);
						
						float ffV = txc[0].y * (1 - fU - fV) + 
							txc[1].y * (fU) + 
							txc[2].y * (fV);

						retexc->x = ffU;
						retexc->y = ffV;

						int tx = ffU * difpx->sizex;
						int ty = ffV * difpx->sizey;
						
						int stx = ffU * specpx->sizex;
						int sty = ffV * specpx->sizey;
						
						int ntx = ffU * normpx->sizex;
						int nty = ffV * normpx->sizey;

						while(tx < 0)
							tx += difpx->sizex;
						while(tx >= difpx->sizex)
							tx %= difpx->sizex;
						while(ty < 0)
							ty += difpx->sizey;
						while(ty >= difpx->sizey)
							ty %= difpx->sizey;

						while(stx < 0)
							stx += specpx->sizex;
						while(stx >= specpx->sizex)
							stx %= specpx->sizex;
						while(sty < 0)
							sty += specpx->sizey;
						while(sty >= specpx->sizey)
							sty %= specpx->sizey;
						
						while(ntx < 0)
							ntx += normpx->sizex;
						while(ntx >= normpx->sizex)
							ntx %= normpx->sizex;
						while(nty < 0)
							nty += normpx->sizey;
						while(nty >= normpx->sizey)
							nty %= normpx->sizey;

						int pxin = (tx + difpx->sizex * ty);
						int spxin = (stx + specpx->sizex * sty);
						int npxin = (ntx + normpx->sizex * nty);

						Vec4f rgba;
						Vec4f srgba;
						Vec4f nrgba;

						rgba.x = difpx->data[ pxin * difpx->channels + 0 ];
						rgba.y = difpx->data[ pxin * difpx->channels + 1 ];
						rgba.z = difpx->data[ pxin * difpx->channels + 2 ];
						rgba.w = 255;
						
						srgba.x = specpx->data[ spxin * specpx->channels + 0 ];
						srgba.y = specpx->data[ spxin * specpx->channels + 1 ];
						srgba.z = specpx->data[ spxin * specpx->channels + 2 ];
						srgba.w = 255;

						nrgba.x = normpx->data[ npxin * normpx->channels + 0 ];
						nrgba.y = normpx->data[ npxin * normpx->channels + 1 ];
						nrgba.z = normpx->data[ npxin * normpx->channels + 2 ];
						nrgba.w = 255;

						*r = (int)(rgba.x);
						*g = (int)(rgba.y);
						*b = (int)(rgba.z);
						*a = (int)(rgba.w);
						
						*sr = (int)(srgba.x);
						*sg = (int)(srgba.y);
						*sb = (int)(srgba.z);
						*sa = (int)(srgba.w);
						
						*nr = (int)(nrgba.x);
						*ng = (int)(nrgba.y);
						*nb = (int)(nrgba.z);
						*na = (int)(nrgba.w);

						*nearnorm = Normal(tr);
#endif

						// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
//#define EDGEFUN(a,b,c)	((c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]))

						

						//delete difpx;
					}
				}
			}
		}
	}

	// Make sure the VAO is not changed from the outside
	//glBindVertexArray(0);
	return nearestv;
}

void Model2::Render(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat)
{
	//glBindVertexArray(m_VAO);
	//Log("entries "<<m_Entries.size()<<std::endl;

	Shader* s = &g_shader[g_curS];

	//Vec3f pos(0,0,0);

	Matrix modelmat;
	modelmat.setTranslation((const float*)&pos);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	//perform frame transformation on-the-fly
	std::vector<Matrix> BoneTransforms;

	//if(!m_pScene)
	//	ErrMess("No scene", "No scene");

#if 1
	if(m_pScene->mNumAnimations > 0)
		//if(0)
	{
		float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float frames = TicksPerSecond * (float)m_pScene->mAnimations[0]->mDuration;
		float percentage = (float)frame / frames;
		float RunningTime = percentage * (float)m_pScene->mAnimations[0]->mDuration;

		BoneTransform(RunningTime, BoneTransforms);

		//Log("bone transform ");
		//Log("m_pScene->mAnimations[0]->mTicksPerSecond "<<m_pScene->mAnimations[0]->mTicksPerSecond<<std::endl;
		//Log("m_pScene->mAnimations[0]->mDuration "<<m_pScene->mAnimations[0]->mDuration<<std::endl;
		//Log("RunningTime "<<RunningTime<<std::endl;
		//Log("BoneTransforms.size() "<<BoneTransforms.size()<<std::endl;
	}
	else
	{
		BoneTransforms.resize( m_BoneInfo.size() );

		for(int i=0; i<m_BoneInfo.size(); i++)
		{
			BoneTransforms[i].InitIdentity();
		}
	}
#else
	//BoneTransform(0, BoneTransforms);
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;
	//Log("m_NumBones = "<<m_NumBones<<std::endl;
#endif

	//Log("m_NumBones = "<<m_NumBones<<std::endl;
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;

	std::vector<Vec3f> TransformedPos;
	std::vector<Vec3f> TransformedNorm;
	TransformedPos.resize(Positions.size());
	TransformedNorm.resize(Normals.size());

#if 0
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
#endif

	Model2* origm = &g_model2[origmodeli];

	for(uint i=0; i<Positions.size(); i++)
	{
#if 0
		Matrix BoneTransform = Bones[BoneIDs[0]] * Weights[0];
		BoneTransform     += Bones[BoneIDs[1]] * Weights[1];
		BoneTransform     += Bones[BoneIDs[2]] * Weights[2];
		BoneTransform     += Bones[BoneIDs[3]] * Weights[3];
#else
		Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
		Transform.InitIdentity();

		//bool influenced = false;

		for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
			//for(int bi=0; bi<1; bi++)
		{
			if(Bones[i].IDs[bi] < 0)
				continue;

			//if(Bones[i].IDs[bi] >= m_BoneInfo.size())
			//	continue;

			//if(Bones[i].IDs[bi] >= BoneTransforms.size())
			//{
			//	Log("bone id "<<Bones[i].IDs[bi]<<" out of "<<BoneTransforms.size()<<std::endl;
			//	continue;
			//}

			if(Bones[i].Weights[bi] == 0.0f)
				continue;

			//influenced = true;

			//Transform += m_BoneInfo[ Bones[i].IDs[bi] ].FinalTransformation * Bones[i].Weights[bi];

			if(bi == 0)
				Transform = BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
			else
				Transform += BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
		}

		//if(!influenced)
		//	Transform.InitIdentity();
#endif

		Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
		Transformed.transform(Transform);
		Transformed.transform(rotmat);
		//Transformed = Transformed * scale;
		TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z) * scale;
		TransformedNorm[i] = origm->Normals[i];
		TransformedNorm[i].transform(Transform);
		TransformedNorm[i].transform(rotmat);
	}

	//If we're including inclines, adjust vertex heights
	if((g_mode == RENDERING || g_mode == PREREND_ADJFRAME) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0] * TILE_RISE;
		heights[1] = g_cornerinc[g_currincline][1] * TILE_RISE;
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2] * TILE_RISE;
		heights[3] = g_cornerinc[g_currincline][3] * TILE_RISE;

		Heightmap hm;
		hm.allocate(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();

#if 0
		if(g_currincline == 2)
			Log("g_currincline "<<g_currincline<<" inc4:"<<
			g_cornerinc[g_currincline][0]<<","<<
			g_cornerinc[g_currincline][1]<<","<<
			g_cornerinc[g_currincline][2]<<","<<
			g_cornerinc[g_currincline][3]<<std::endl;
#endif

		//TODO need to take into account rotation matrix also
		for(uint i=0; i<TransformedPos.size(); i++)
		{
			//TransformedPos[i].y += Bilerp(&hm,
			//	g_tilesize/2.0f + pos.x + TransformedPos[i].x,
			//	g_tilesize/2.0f + pos.z + TransformedPos[i].z);
			TransformedPos[i].y += hm.accheight(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z);
#if 0
			if(g_currincline == 2
				&& hm.accheight(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z) > 0.0f)
			{
				Log("adj #"<<i<<" xz"
					<<(g_tilesize/2.0f + pos.x + TransformedPos[i].x)<<","<<(g_tilesize/2.0f + pos.z + TransformedPos[i].z)
					<<" y+"<<
					hm.accheight(
					g_tilesize/2.0f + pos.x + TransformedPos[i].x,
					g_tilesize/2.0f + pos.z + TransformedPos[i].z)<<std::endl;
			}
#endif
		}

		//Regenerate normals:
		//Not possible, based on vertices alone, because we would also need to blend shared faces,
		//so leave this inaccuracy for now. TODO
	}

	for (uint i = 0 ; i < m_Entries.size() ; i++)
	{
		const uint MaterialIndex = m_Entries[i].MaterialIndex;

		//Log("mat "<<MaterialIndex<<" "<<m_Textures.size()<<std::endl;
		//

		if(MaterialIndex < m_Textures.size() && m_Textures.size())
		{

#if 0
			if (m_Textures[MaterialIndex]) {
				m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
			}
#else
			Material* mat = &m_Textures[MaterialIndex];

			//Log("mat->m_diffusem"<<mat->m_diffusem<<std::endl;
			//Log("g_texture[ mat->m_diffusem ].texname"<<g_texture[ mat->m_diffusem ].texname<<std::endl;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_diffusem ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_specularm ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_normalm ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_ownerm ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);
#endif

		}
		else
		{
			static unsigned int notex = 0;

			if(!notex)
				CreateTex(notex, "textures/notex.jpg", false, true);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);
		}

#if 0
		glDrawElementsBaseVertex(GL_TRIANGLES,
			m_Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(uint) * m_Entries[i].BaseIndex),
			m_Entries[i].BaseVertex);
#endif

		const unsigned int numindices = m_Entries[i].NumIndices;
		const unsigned int basevertex = m_Entries[i].BaseVertex;
		const unsigned int baseindex = m_Entries[i].BaseIndex;
		const unsigned int numunique = m_Entries[i].NumUniqueVerts;

		//glVertexPointer(3, GL_FLOAT, 0, &Positions[basevertex]);
		glVertexPointer(3, GL_FLOAT, 0, &TransformedPos[basevertex]);
		glTexCoordPointer(2, GL_FLOAT, 0, &TexCoords[basevertex]);
		//glNormalPointer(GL_FLOAT, 0, &Normals[basevertex]);
		glNormalPointer(GL_FLOAT, 0, &TransformedNorm[basevertex]);
		//glIndexPointer(GL_UNSIGNED_INT, 0, &Indices[baseindex]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//glDrawArrays(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		glDrawRangeElements(GL_TRIANGLES, 0, numunique, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		//glDrawRangeElements(GL_TRIANGLES, 0, 3, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);

#if 0
		static bool once = false;

		if(!once)
		{
			once = true;

			for(int v=0; v<numindices; v++)
			{
				Vec3f v3d = Positions[basevertex + Indices[baseindex + v]];
				Log("["<<v<<"] ("<<(v%3)<<"): "<<v3d.x<<","<<v3d.y<<","<<v3d.z<<std::endl;
			}
		}
#endif
		//Log("indices "<<numindices<<std::endl;
	}

	// Make sure the VAO is not changed from the outside
	//glBindVertexArray(0);
}


void Model2::RenderDepth(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat)
{
	//glBindVertexArray(m_VAO);
	//Log("entries "<<m_Entries.size()<<std::endl;

	Shader* s = &g_shader[g_curS];

	//Vec3f pos(0,0,0);

	Matrix modelmat;
	modelmat.setTranslation((const float*)&pos);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	//perform frame transformation on-the-fly
	std::vector<Matrix> BoneTransforms;

	//if(!m_pScene)
	//	ErrMess("No scene", "No scene");

#if 1
	if(m_pScene->mNumAnimations > 0)
	{
		float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float frames = TicksPerSecond * (float)m_pScene->mAnimations[0]->mDuration;
		float percentage = (float)frame / frames;
		float RunningTime = percentage * (float)m_pScene->mAnimations[0]->mDuration;

		BoneTransform(RunningTime, BoneTransforms);
	}
	else
	{
		BoneTransforms.resize( m_BoneInfo.size() );

		for(int i=0; i<m_BoneInfo.size(); i++)
		{
			BoneTransforms[i].InitIdentity();
		}
	}
#else
	//BoneTransform(0, BoneTransforms);
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;
	//Log("m_NumBones = "<<m_NumBones<<std::endl;
#endif

	//Log("m_NumBones = "<<m_NumBones<<std::endl;
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;

	std::vector<Vec3f> TransformedPos;
	std::vector<Vec3f> TransformedNorm;
	TransformedPos.resize(Positions.size());
	TransformedNorm.resize(Normals.size());

#if 0
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
#endif

	Model2* origm = &g_model2[origmodeli];

	for(uint i=0; i<Positions.size(); i++)
	{
#if 0
		Matrix BoneTransform = Bones[BoneIDs[0]] * Weights[0];
		BoneTransform     += Bones[BoneIDs[1]] * Weights[1];
		BoneTransform     += Bones[BoneIDs[2]] * Weights[2];
		BoneTransform     += Bones[BoneIDs[3]] * Weights[3];
#else
		Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
		Transform.InitIdentity();

		//bool influenced = false;

		for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
		{
			if(Bones[i].IDs[bi] < 0)
				continue;

			//if(Bones[i].IDs[bi] >= m_BoneInfo.size())
			//	continue;

			//if(Bones[i].IDs[bi] >= BoneTransforms.size())
			//{
			//	Log("bone id "<<Bones[i].IDs[bi]<<" out of "<<BoneTransforms.size()<<std::endl;
			//	continue;
			//}

			if(Bones[i].Weights[bi] == 0.0f)
				continue;

			//influenced = true;

			//Transform += m_BoneInfo[ Bones[i].IDs[bi] ].FinalTransformation * Bones[i].Weights[bi];

			if(bi == 0)
				Transform = BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
			else
				Transform += BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
		}

		//if(!influenced)
		//	Transform.InitIdentity();
#endif

		Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
		Transformed.transform(Transform);
		Transformed.transform(rotmat);
		//Transformed = Transformed * scale;
		TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z) * scale;
		TransformedNorm[i] = origm->Normals[i];
		TransformedNorm[i].transform(Transform);
		TransformedNorm[i].transform(rotmat);
	}

	//If we're including inclines, adjust vertex heights
	if((g_mode == RENDERING || g_mode == PREREND_ADJFRAME) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0] * TILE_RISE;
		heights[1] = g_cornerinc[g_currincline][1] * TILE_RISE;
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2] * TILE_RISE;
		heights[3] = g_cornerinc[g_currincline][3] * TILE_RISE;

		Heightmap hm;
		hm.allocate(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();

		for(uint i=0; i<TransformedPos.size(); i++)
		{
			//TransformedPos[i].y += Bilerp(&hm,
			//	g_tilesize/2.0f + pos.x + TransformedPos[i].x,
			//	g_tilesize/2.0f + pos.z + TransformedPos[i].z);
			TransformedPos[i].y += hm.accheight(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z);
		}

		//Regenerate normals:
		//Not possible, based on vertices alone, because we would also need to blend shared faces,
		//so leave this inaccuracy for now. TODO
	}

	for (uint i = 0 ; i < m_Entries.size() ; i++)
	{
		const uint MaterialIndex = m_Entries[i].MaterialIndex;

		if(MaterialIndex < m_Textures.size() && m_Textures.size())
		{

#if 0
			if (m_Textures[MaterialIndex]) {
				m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
			}
#else
			Material* mat = &m_Textures[MaterialIndex];

			//Log("mat->m_diffusem"<<mat->m_diffusem<<std::endl;
			//Log("g_texture[ mat->m_diffusem ].texname"<<g_texture[ mat->m_diffusem ].texname<<std::endl;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_diffusem ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
#if 0
			glActiveTextureARB(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_specularm ].texname);
			glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);

			glActiveTextureARB(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_normalm ].texname);
			glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);

			glActiveTextureARB(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_ownerm ].texname);
			glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);
#endif
#endif
		}
		else
		{
			static unsigned int notex = 0;

			if(!notex)
				CreateTex(notex, "textures/notex.jpg", false, true);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
		}

#if 0
		glDrawElementsBaseVertex(GL_TRIANGLES,
			m_Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(uint) * m_Entries[i].BaseIndex),
			m_Entries[i].BaseVertex);
#endif

		const unsigned int numindices = m_Entries[i].NumIndices;
		const unsigned int basevertex = m_Entries[i].BaseVertex;
		const unsigned int baseindex = m_Entries[i].BaseIndex;
		const unsigned int numunique = m_Entries[i].NumUniqueVerts;

		//glVertexPointer(3, GL_FLOAT, 0, &Positions[basevertex]);
		glVertexPointer(3, GL_FLOAT, 0, &TransformedPos[basevertex]);
		glTexCoordPointer(2, GL_FLOAT, 0, &TexCoords[basevertex]);
		//glNormalPointer(GL_FLOAT, 0, &Normals[basevertex]);
		glNormalPointer(GL_FLOAT, 0, &TransformedNorm[basevertex]);
		//glIndexPointer(GL_UNSIGNED_INT, 0, &Indices[baseindex]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//glDrawArrays(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		glDrawRangeElements(GL_TRIANGLES, 0, numunique, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		//glDrawRangeElements(GL_TRIANGLES, 0, 3, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);

#if 0
		static bool once = false;

		if(!once)
		{
			once = true;

			for(int v=0; v<numindices; v++)
			{
				Vec3f v3d = Positions[basevertex + Indices[baseindex + v]];
				Log("["<<v<<"] ("<<(v%3)<<"): "<<v3d.x<<","<<v3d.y<<","<<v3d.z<<std::endl;
			}
		}
#endif
		//Log("indices "<<numindices<<std::endl;
	}

	// Make sure the VAO is not changed from the outside
	//glBindVertexArray(0);
}

uint Model2::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


uint Model2::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	//assert(pNodeAnim->mNumRotationKeys > 0);

	for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


uint Model2::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	//assert(pNodeAnim->mNumScalingKeys > 0);

	for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


void Model2::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	uint NextPositionIndex = (PositionIndex + 1);
	//assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void Model2::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	uint NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}


void Model2::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	uint NextScalingIndex = (ScalingIndex + 1);
	//assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void Model2::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const Matrix& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[0];

	Matrix NodeTransformation(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		Matrix ScalingM;
		ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		Matrix RotationM = Matrix(RotationQ.GetMatrix());

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		Matrix TranslationM;
		TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
		//NodeTransformation = ScalingM * RotationM * TranslationM;
		//NodeTransformation = ScalingM;
		//NodeTransformation.postmult(RotationM);
		//NodeTransformation.postmult(TranslationM);

		//NodeTransformation = TranslationM;
		//NodeTransformation.postmult(RotationM);
		//NodeTransformation.postmult(ScalingM);
	}

	Matrix GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
		uint BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
		//m_BoneInfo[BoneIndex].FinalTransformation = m_BoneInfo[BoneIndex].BoneOffset * GlobalTransformation * m_GlobalInverseTransform;

		//m_BoneInfo[BoneIndex].FinalTransformation = m_BoneInfo[BoneIndex].BoneOffset;
		//m_BoneInfo[BoneIndex].FinalTransformation.postmult(GlobalTransformation);
		//m_BoneInfo[BoneIndex].FinalTransformation.postmult(m_GlobalInverseTransform);

		//m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform;
		//m_BoneInfo[BoneIndex].FinalTransformation.postmult(GlobalTransformation);
		//m_BoneInfo[BoneIndex].FinalTransformation.postmult(m_BoneInfo[BoneIndex].BoneOffset);
	}

	for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}


void Model2::BoneTransform(float TimeInSeconds, std::vector<Matrix>& Transforms)
{
	Matrix Identity;
	Identity.InitIdentity();

	float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)m_pScene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

	Transforms.resize(m_NumBones);

	for (uint i = 0 ; i < m_NumBones ; i++) {
		Transforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}


const aiNodeAnim* Model2::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void FreeModels2()
{
	for(int i=0; i<MODELS2; i++)
	{
		Model2* m = &g_model2[i];

		if(!m->m_on)
			continue;

		m->destroy();
	}
}
