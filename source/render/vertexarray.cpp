

#include "vertexarray.h"
#include "../utils.h"

VertexArray::VertexArray(const VertexArray& original)
{
	//Log("vertex array copy constructor");
	/*
	alloc(original.numverts);
	memcpy(vertices, original.vertices, sizeof(Vec3f)*numverts);
	memcpy(texcoords, original.texcoords, sizeof(Vec2f)*numverts);
	memcpy(normals, original.normals, sizeof(Vec3f)*numverts);*/

	numverts = 0;
	*this = original;
}


VertexArray& VertexArray::operator=(VertexArray const &original)
{
	//Log("vertex array assignment op");

	alloc(original.numverts);

	if(original.numverts > 0)
	{
		if(original.vertices) memcpy(vertices, original.vertices, sizeof(Vec3f)*numverts);
		if(original.texcoords) memcpy(texcoords, original.texcoords, sizeof(Vec2f)*numverts);
		if(original.normals) memcpy(normals, original.normals, sizeof(Vec3f)*numverts);
		//memcpy(tangents, original.tangents, sizeof(Vec3f)*numverts);
	}

	return *this;
}

void VertexArray::alloc(int numv)
{
	free();
	numverts = numv;
	vertices = new Vec3f[numv];
	texcoords = new Vec2f[numv];
	normals = new Vec3f[numv];
	//tangents = new Vec3f[numv];
	if(!vertices) OutOfMem(__FILE__, __LINE__);
	if(!texcoords) OutOfMem(__FILE__, __LINE__);
	if(!normals) OutOfMem(__FILE__, __LINE__);
}

void VertexArray::free()
{
	if(numverts <= 0)
		return;

	delete [] vertices;
	delete [] texcoords;
	delete [] normals;
	//delete [] tangents;
	numverts = 0;
}

void CopyVA(VertexArray* to, const VertexArray* from)
{
	to->alloc(from->numverts);

	for(int i=0; i<from->numverts; i++)
	{
		to->vertices[i] = from->vertices[i];
		to->texcoords[i] = from->texcoords[i];
		to->normals[i] = from->normals[i];
	}
}

void CopyVAs(VertexArray** toframes, int* tonframes, VertexArray* const* fromframes, int fromnframes)
{
	*tonframes = fromnframes;

	(*toframes) = new VertexArray[fromnframes];

	for(int i=0; i<fromnframes; i++)
	{
		CopyVA(&(*toframes)[i], &(*fromframes)[i]);
	}
}
