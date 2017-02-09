



#ifndef SHADER_H
#define SHADER_H

#include "../platform.h"

#define SSLOT_SHADOWMAP			0
#define SSLOT_LIGHTMATRIX		1
#define SSLOT_LIGHTPOS			2
//#define SSLOT_LIGHTDIR			3
#define SSLOT_TEXTURE0			4
#define SSLOT_TEXTURE1			5
#define SSLOT_TEXTURE2			6
#define SSLOT_TEXTURE3			7
#define SSLOT_POSITION			8
#define SSLOT_NORMAL			9
#define SSLOT_TEXCOORD0			10
#define SSLOT_TEXCOORD1			11
#define SSLOT_PROJECTION		12
#define SSLOT_MODELMAT			13
#define SSLOT_VIEWMAT			14
#define SSLOT_NORMALMAT			15
#define SSLOT_INVMODLVIEWMAT	16
#define SSLOT_MVP			17
#define SSLOT_COLOR				18
#define SSLOT_OWNCOLOR			19
#define SSLOT_WIDTH				20
#define SSLOT_HEIGHT			21
#define SSLOT_CAMERAPOS			22
#define SSLOT_SCALE				23
#define SSLOT_MIND				24
#define SSLOT_MAXD				25
#define SSLOT_NORMALMAP			26
#define SSLOT_SPECULARMAP		27
//#define SSLOT_TANGENT			28
#define SSLOT_MAXELEV			28
#define SSLOT_SANDONLYMAXY		29
#define SSLOT_SANDGRASSMAXY		30
#define SSLOT_GRASSONLYMAXY		31
#define SSLOT_GRASSDIRTMAXY		32
#define SSLOT_DIRTONLYMAXY		33
#define SSLOT_DIRTROCKMAXY		34
#define SSLOT_SANDGRAD			35
#define SSLOT_SANDDET			36
#define SSLOT_GRASSGRAD			37
#define SSLOT_GRASSDET			38
#define SSLOT_DIRTGRAD			39
#define SSLOT_DIRTDET			40
#define SSLOT_ROCKGRAD			41
#define SSLOT_ROCKDET			42
#define SSLOT_SUNDIRECTION		43
#define SSLOT_GRADIENTTEX		44
#define SSLOT_DETAILTEX			45
#define SSLOT_OWNERMAP			46
#define SSLOT_MODELVIEW			47
#define SSLOT_JUMPTEX			48
#define SSLOT_POSXTEX			49
#define SSLOT_POSYTEX			50
#define SSLOT_POSZTEX			51
#define SSLOT_CORNERA			52
#define SSLOT_CORNERB			53
#define SSLOT_CORNERC			54
#define SSLOT_CORNERD			55
#define SSLOT_ORJPLWPX			56
#define SSLOT_ORJPLHPX			57
#define SSLOT_ORJLONS			58
#define SSLOT_ORJLATS			59
#define SSLOT_ORJLON			60
#define SSLOT_ORJLAT			61
#define SSLOT_VIEWVEC			62
#define SSLOT_ORMAPSZ			63
#define SSLOT_IMVP				64
#define SSLOT_ORJROLL			65
#define SSLOT_ORJROLLS			66
#define SSLOT_UPDIR				67
#define SSLOT_SIDEDIR			68
#define SSLOT_VIEWDIR			69
#define SSLOT_UPLEN				70
#define SSLOT_SIDELEN			71
#define SSLOT_CAMLAT			72
#define SSLOT_CAMLON			73
#define SSLOT_CAMROLL			74
#define SSLOT_INCIDLAT			75
#define SSLOT_INCIDLON			76
#define SSLOT_INCIDROLL			78
#define SSLOT_CAMCEN			79
#define SSLOTS					80


#ifdef PLATFORM_MAC
#define SHADERTYPE GLuint
#else
#define SHADERTYPE GLhandleARB
#endif

class Shader
{
public:
	Shader()	{			 }
	~Shader()	{ release(); }

	GLint GetUniform(const char* strVariable);
	GLint GetAttrib(const char* strVariable);
	
	SHADERTYPE GetProgram()	{	return m_hProgramObject; }
	SHADERTYPE GetVertexS()	{	return m_hVertexShader; }
	SHADERTYPE GetFragmentS()	{	return m_hFragmentShader; }
	void MapUniform(int slot, const char* variable);
	void MapAttrib(int slot, const char* variable);

	void release();

	bool m_hasverts;
	bool m_hastexcoords;
	bool m_hasnormals;
	GLint m_slot[SSLOTS];
	
	SHADERTYPE m_hVertexShader;
	SHADERTYPE m_hFragmentShader;
	SHADERTYPE m_hProgramObject;
};

#define SHADER_DEPTH			0
#define SHADER_SHADOW			1
#define SHADER_OWNED			2
#define SHADER_ORTHO			3
#define SHADER_COLOR2D			4
#define SHADER_MODEL			5
#define SHADER_MAP				6
#define SHADER_COLOR3D			7
#define SHADER_BILLBOARD		8
#define SHADER_WATER			9
#define SHADER_COLOR3DPERSP		10
#define SHADER_BILLBOARDPERSP	11
#define SHADER_TEAM				12
#define SHADER_MODELPERSP		13
#define SHADER_MAPPERSP			14
#define SHADER_LIGHTTEST		15
#define SHADER_DEPTHRGBA		16
#define SHADER_OR				17
#define SHADER_ORPERSP			18
#define SHADERS					19

extern Shader g_shader[SHADERS];
extern int g_curS;

void UseS(int shader);
void EndS();
void InitGLSL();
void TurnOffShader();
void ReleaseShaders();
std::string LoadTextFile(char* strFile);
void LoadShader(int shader, char* strVertex, char* strFragment, bool hasverts, bool hastexcoords, bool hasnormals);
void GetGLVersion(int* major, int* minor);

#endif


