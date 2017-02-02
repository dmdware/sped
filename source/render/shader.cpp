






#include "../utils.h"
#include "../render/shader.h"
#include "../platform.h"
#include "../debug.h"
#include "../window.h"

Shader g_shader[SHADERS];
int g_curS = 0;

GLint Shader::GetUniform(const char* strVariable)
{
	if(!m_hProgramObject)
		return -1;

	return glGetUniformLocation(m_hProgramObject, strVariable);
}

GLint Shader::GetAttrib(const char* strVariable)
{
	if(!m_hProgramObject)
		return -1;

	return glGetAttribLocation(m_hProgramObject, strVariable);
}

void Shader::MapUniform(int slot, const char* variable)
{
	m_slot[slot] = GetUniform(variable);
	//Log("\tmap uniform "<<variable<<" = "<<(int)m_slot[m_slot]<<std::endl;
}

void Shader::MapAttrib(int slot, const char* variable)
{
	m_slot[slot] = GetAttrib(variable);
	//Log("\tmap attrib "<<variable<<" = "<<(int)m_slot[m_slot]<<std::endl;
}

void GetGLVersion(int* major, int* minor)
{
	// for all versions
	char* ver = (char*)glGetString(GL_VERSION); // ver = "3.2.0"

#ifdef DEBUG
	CHECKGLERROR();
	LastNum(__FILE__, __LINE__);
#endif

	char vermaj[4];

	for(int i=0; i<4; i++)
	{
		if(ver[i] != '.')
			vermaj[i] = ver[i];
		else
			vermaj[i] = '\0';
	}

	//*major = ver[0] - '0';
	*major = StrToInt(vermaj);
	if( *major >= 3)
	{
		// for GL 3.x
		glGetIntegerv(GL_MAJOR_VERSION, major); // major = 3
		glGetIntegerv(GL_MINOR_VERSION, minor); // minor = 2
	}
	else
	{
		*minor = ver[2] - '0';
	}

	// GLSL
	ver = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION); // ver = "1.50 NVIDIA via Cg compiler"
}

void InitGLSL()
{
#ifndef PLATFORM_MAC
	//strstr("abab", "ba");
	////glewExperimental = GL_TRUE;		//xp fix?
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		ErrMess("Error initializing GLEW!", (const char*)glewGetErrorString( glewError ));
		return;
	}
#endif

	Log("Renderer: %s", (char*)glGetString(GL_RENDERER));
	Log("GL_VERSION = %s", (char*)glGetString(GL_VERSION));

	
#ifdef __glew_h__
#if 0
	//Make sure OpenGL 2.1 is supported
	if( !GLEW_VERSION_2_1 )
	{
		ErrMess("Error", "OpenGL 2.1 not supported!\n" );
		return;
	}
#elif 0
	if( !GLEW_VERSION_3_2 )
	{
		ErrMess("Error", "OpenGL 3.2 not supported!\n" );
		return;
	}
#else
	if( !GLEW_VERSION_1_4 )
	{
		ErrMess("Error", "OpenGL 1.4 not supported!\n" );
		g_quit = true;
		return;
	}
#endif
#endif
	
#if 1
	char* szGLExtensions = (char*)glGetString(GL_EXTENSIONS);

	Log(szGLExtensions);
	

#if !defined( PLATFORM_MAC ) && !defined( PLATFORM_IOS )
	if(!strstr(szGLExtensions, "GL_ARB_debug_output"))
	{
		//ErrMess("Error", "GL_ARB_debug_output extension not supported!");
		//g_quit = true;
		//return;
		Log("GL_ARB_debug_output extension not supported");
	}
	else
	{
		Log("Reging debug handler");
		
		//glDebugMessageCallback(&GLMessageHandler, 0);	//xp broke no fun
		CHECKGLERROR();
	}
#endif

	if(!strstr(szGLExtensions, "GL_ARB_framebuffer_object"))
	{
		ErrMess("Error", "GL_ARB_framebuffer_object extension not supported!");
		g_quit = true;
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shader_objects"))
	{
		ErrMess("Error", "GL_ARB_shader_objects extension not supported!");
		g_quit = true;
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shading_language_100"))
	{
		ErrMess("Error", "GL_ARB_shading_language_100 extension not supported!");
		g_quit = true;
		return;
	}
#endif

	int major, minor;
	GetGLVersion(&major, &minor);

	if(major < 1 || ( major == 1 && minor < 4 ))
	{
		ErrMess("Error", "OpenGL 1.4 is not supported!");
		g_quit = true;
	}

	CHECKGLERROR();
	
	LoadShader(SHADER_ORTHO, "shaders/ortho.vert", "shaders/ortho.frag", true, true, false);
	LoadShader(SHADER_LIGHTTEST, "shaders/lighttest.vert", "shaders/lighttest.frag", true, true, false);
	LoadShader(SHADER_COLOR2D, "shaders/color2d.vert", "shaders/color2d.frag", true, false, false);
	LoadShader(SHADER_MODEL, "shaders/model.vert", "shaders/model.frag", true, true, true);
	LoadShader(SHADER_MODELPERSP, "shaders/modelpersp.vert", "shaders/model.frag", true, true, true);
	LoadShader(SHADER_COLOR3D, "shaders/color3d.vert", "shaders/color3d.frag", true, false, false);
	LoadShader(SHADER_COLOR3DPERSP, "shaders/color3dpersp.vert", "shaders/color3d.frag", true, false, false);
	LoadShader(SHADER_BILLBOARD, "shaders/billboard.vert", "shaders/billboard.frag", true, true, false);
	LoadShader(SHADER_DEPTH, "shaders/depth.vert", "shaders/depth.frag", true, true, false);
	LoadShader(SHADER_DEPTHRGBA, "shaders/depth.vert", "shaders/depthrgba.frag", true, true, false);
	//LoadShader(SHADER_SHADOW, "shaders/shadow.vert", "shaders/shadow.frag");
	LoadShader(SHADER_OWNED, "shaders/owned.vert", "shaders/owned.frag", true, true, true);
	LoadShader(SHADER_MAP, "shaders/map.vert", "shaders/map.frag", true, true, true);
	LoadShader(SHADER_MAPPERSP, "shaders/mappersp.vert", "shaders/map.frag", true, true, true);
	LoadShader(SHADER_WATER, "shaders/water.vert", "shaders/water.frag", true, true, true);
	LoadShader(SHADER_BILLBOARDPERSP, "shaders/billboardpersp.vert", "shaders/billboard.frag", true, true, false);
	LoadShader(SHADER_TEAM, "shaders/team.vert", "shaders/team.frag", true, true, true);
	
	LoadShader(SHADER_OR, "shaders/or.vert", "shaders/or.frag", true, true, false);
	LoadShader(SHADER_ORPERSP, "shaders/orpersp.vert", "shaders/orpersp.frag", true, true, false);

	CHECKGLERROR();
}

void LoadShader(int shader, char* strVertex, char* strFragment, bool hasverts, bool hastexcoords, bool hasnormals)
{
	Shader* s = &g_shader[shader];
	
	g_shader[shader].m_hasverts = hasverts;
	g_shader[shader].m_hastexcoords = hastexcoords;
	g_shader[shader].m_hasnormals = hasnormals;

	std::string strVShader, strFShader;

	if(s->m_hVertexShader || s->m_hFragmentShader || s->m_hProgramObject)
		s->release();

	//s->m_hVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	//s->m_hFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	s->m_hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	s->m_hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	strVShader = LoadTextFile(strVertex);
	strFShader = LoadTextFile(strFragment);

	const char *szVShader = strVShader.c_str();
	const char *szFShader = strFShader.c_str();

	//glShaderSourceARB(s->m_hVertexShader, 1, &szVShader, NULL);
	//glShaderSourceARB(s->m_hFragmentShader, 1, &szFShader, NULL);
	glShaderSource(s->m_hVertexShader, 1, &szVShader, NULL);
	glShaderSource(s->m_hFragmentShader, 1, &szFShader, NULL);

	//glCompileShaderARB(s->m_hVertexShader);
	glCompileShader(s->m_hVertexShader);
	GLint logLength;
	glGetShaderiv(s->m_hVertexShader, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
		if(!log) OutOfMem(__FILE__, __LINE__);
        glGetShaderInfoLog(s->m_hVertexShader, logLength, &logLength, log);
        Log("Shader %s compile log: %s", strVertex, log);
        free(log);
    }

	CHECKGLERROR();

	//glCompileShaderARB(s->m_hFragmentShader);
	glCompileShader(s->m_hFragmentShader);
	glGetShaderiv(s->m_hFragmentShader, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
		if(!log) OutOfMem(__FILE__, __LINE__);
        glGetShaderInfoLog(s->m_hFragmentShader, logLength, &logLength, log);
        Log("Shader %s compile log: %s", strFragment, log);
        free(log);
    }

	CHECKGLERROR();

	//s->m_hProgramObject = glCreateProgramObjectARB();
	s->m_hProgramObject = glCreateProgram();
	//glAttachObjectARB(s->m_hProgramObject, s->m_hVertexShader);
	//glAttachObjectARB(s->m_hProgramObject, s->m_hFragmentShader);
	glAttachShader(s->m_hProgramObject, s->m_hVertexShader);
	glAttachShader(s->m_hProgramObject, s->m_hFragmentShader);
	//glLinkProgramARB(s->m_hProgramObject);
	glLinkProgram(s->m_hProgramObject);

	//glUseProgramObjectARB(s->m_hProgramObject);

	//Log("shader "<<strVertex<<","<<strFragment<<std::endl;

	CHECKGLERROR();

	Log("Program %s / %s", strVertex, strFragment);

	glGetProgramiv(s->m_hProgramObject, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(s->m_hProgramObject, logLength, &logLength, log);
		Log("Program link log: %s", log);
		free(log);
	}

	GLint status;
	glGetProgramiv(s->m_hProgramObject, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		Log("link status 0");
	}
	else
	{
		Log("link status ok");
	}

	Log("\r\n\r\n");

	CHECKGLERROR();

    s->MapAttrib(SSLOT_POSITION, "position");
	CHECKGLERROR();
    s->MapAttrib(SSLOT_NORMAL, "normalIn");
	CHECKGLERROR();
    s->MapAttrib(SSLOT_TEXCOORD0, "texCoordIn0");
	CHECKGLERROR();
    s->MapAttrib(SSLOT_TEXCOORD1, "texCoordIn1");
	CHECKGLERROR();
    //s->MapAttrib(SSLOT_TANGENT, "tangent");
	s->MapUniform(SSLOT_SHADOWMAP, "shadowmap");
	CHECKGLERROR();
	s->MapUniform(SSLOT_LIGHTMATRIX, "lightMatrix");
	CHECKGLERROR();
	s->MapUniform(SSLOT_LIGHTPOS, "lightPos");
	CHECKGLERROR();
	//s->MapUniform(SSLOT_LIGHTDIR, "lightDir");
	s->MapUniform(SSLOT_TEXTURE0, "texture0");
	CHECKGLERROR();
	s->MapUniform(SSLOT_TEXTURE1, "texture1");
	CHECKGLERROR();
	s->MapUniform(SSLOT_TEXTURE2, "texture2");
	CHECKGLERROR();
	s->MapUniform(SSLOT_TEXTURE3, "texture3");
	CHECKGLERROR();
	s->MapUniform(SSLOT_NORMALMAP, "normalmap");
	CHECKGLERROR();
	s->MapUniform(SSLOT_SPECULARMAP, "specularmap");
	CHECKGLERROR();
    s->MapUniform(SSLOT_PROJECTION, "projection");
	CHECKGLERROR();
    s->MapUniform(SSLOT_MODELMAT, "model");
	CHECKGLERROR();
    s->MapUniform(SSLOT_VIEWMAT, "view");
	CHECKGLERROR();
    s->MapUniform(SSLOT_MVP, "mvp");
	CHECKGLERROR();
    s->MapUniform(SSLOT_MODELVIEW, "modelview");
	s->MapUniform(SSLOT_NORMALMAT, "normalMat");
	//s->MapUniform(SSLOT_INVMODLVIEWMAT, "invModelView");
    s->MapUniform(SSLOT_COLOR, "color");
	CHECKGLERROR();
    s->MapUniform(SSLOT_OWNCOLOR, "owncolor");
	CHECKGLERROR();
    s->MapUniform(SSLOT_WIDTH, "width");
	CHECKGLERROR();
    s->MapUniform(SSLOT_HEIGHT, "height");
	CHECKGLERROR();
    s->MapUniform(SSLOT_MIND, "mind");
	CHECKGLERROR();
    s->MapUniform(SSLOT_MAXD, "maxd");
	CHECKGLERROR();
    s->MapUniform(SSLOT_CAMERAPOS, "cameraPos");
	CHECKGLERROR();
    s->MapUniform(SSLOT_SCALE, "scale");
	CHECKGLERROR();
	s->MapUniform(SSLOT_MAXELEV, "maxelev");
	CHECKGLERROR();
	s->MapUniform(SSLOT_SANDONLYMAXY, "sandonlymaxy");
	CHECKGLERROR();
	s->MapUniform(SSLOT_SANDGRASSMAXY, "sandgrassmaxy");
	CHECKGLERROR();
	s->MapUniform(SSLOT_GRASSONLYMAXY, "grassonlymaxy");
	CHECKGLERROR();
	s->MapUniform(SSLOT_GRASSDIRTMAXY, "grassdirtmaxy");
	CHECKGLERROR();
	s->MapUniform(SSLOT_DIRTONLYMAXY, "dirtonlymaxy");
	CHECKGLERROR();
	s->MapUniform(SSLOT_DIRTROCKMAXY, "dirtrockmaxy");
	CHECKGLERROR();
	s->MapUniform(SSLOT_SANDGRAD, "sandgrad");
	CHECKGLERROR();
	s->MapUniform(SSLOT_SANDDET, "sanddet");
	CHECKGLERROR();
	s->MapUniform(SSLOT_GRASSGRAD, "grassgrad");
	CHECKGLERROR();
	s->MapUniform(SSLOT_GRASSDET, "grassdet");
	CHECKGLERROR();
	s->MapUniform(SSLOT_DIRTGRAD, "dirtgrad");
	CHECKGLERROR();
	s->MapUniform(SSLOT_DIRTDET, "dirtdet");
	CHECKGLERROR();
	s->MapUniform(SSLOT_ROCKGRAD, "rockgrad");
	CHECKGLERROR();
	s->MapUniform(SSLOT_ROCKGRAD, "rockdet");
	CHECKGLERROR();
	s->MapUniform(SSLOT_SUNDIRECTION, "sundirection");
	CHECKGLERROR();
	s->MapUniform(SSLOT_GRADIENTTEX, "gradienttex");
	CHECKGLERROR();
	s->MapUniform(SSLOT_DETAILTEX, "detailtex");
	CHECKGLERROR();
	s->MapUniform(SSLOT_OWNERMAP, "ownermap");

	CHECKGLERROR();
	s->MapUniform(SSLOT_JUMPTEX, "jumptex");
	CHECKGLERROR();
	s->MapUniform(SSLOT_POSXTEX, "posxtex");
	CHECKGLERROR();
	s->MapUniform(SSLOT_POSYTEX, "posytex");
	CHECKGLERROR();
	s->MapUniform(SSLOT_POSZTEX, "posztex");
	
	CHECKGLERROR();
	s->MapUniform(SSLOT_CORNERA, "cornera");
	CHECKGLERROR();
	s->MapUniform(SSLOT_CORNERB, "cornerb");
	CHECKGLERROR();
	s->MapUniform(SSLOT_CORNERC, "cornerc");
	CHECKGLERROR();
	s->MapUniform(SSLOT_CORNERD, "cornerd");
	
	CHECKGLERROR();
	s->MapUniform(SSLOT_ORJPLWPX, "orjplwpx");
	CHECKGLERROR();
	s->MapUniform(SSLOT_ORJPLHPX, "orjplhpx");
	CHECKGLERROR();
	s->MapUniform(SSLOT_ORJLONS, "orjlons");
	CHECKGLERROR();
	s->MapUniform(SSLOT_ORJLATS, "orjlats");
	
	CHECKGLERROR();
	s->MapUniform(SSLOT_ORJLON, "orjlon");
	CHECKGLERROR();
	s->MapUniform(SSLOT_ORJLAT, "orjlat");

	CHECKGLERROR();
	s->MapUniform(SSLOT_ORMAPSZ, "ormapsz");

	CHECKGLERROR();
}

void UseS(int shader)
{
#ifdef DEBUG
	CHECKGLERROR();
#endif
	g_curS = shader;
	//glUseProgramObjectARB(g_shader[shader].m_hProgramObject);
	glUseProgram(g_shader[shader].m_hProgramObject);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Shader* s = &g_shader[g_curS];
#ifdef DEBUG
	CHECKGLERROR();
#endif

#ifdef DEBUG
	CHECKGLERROR();
#endif
	//if(s->m_slot[SSLOT_POSITION] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	//if(s->m_slot[SSLOT_TEXCOORD0] != -1) glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	//if(s->m_slot[SSLOT_NORMAL] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CHECKGLERROR();
#endif

	if(s->m_hasverts)	glEnableClientState(GL_VERTEX_ARRAY);
	if(s->m_hastexcoords)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(s->m_hasnormals)	glEnableClientState(GL_NORMAL_ARRAY);
}

void EndS()
{
#ifdef DEBUG
	CHECKGLERROR();
#endif

	if(g_curS < 0)
		return;

	Shader* s = &g_shader[g_curS];

#ifdef DEBUG
	CHECKGLERROR();
#endif
	//if(s->m_slot[SSLOT_POSITION] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	//if(s->m_slot[SSLOT_TEXCOORD0] != -1) glDisableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	//if(s->m_slot[SSLOT_NORMAL] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CHECKGLERROR();
#endif

	glUseProgram(0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	g_curS = -1;
}

void Shader::release()
{
	if(m_hVertexShader)
	{
		glDetachShader(m_hProgramObject, m_hVertexShader);
		glDeleteShader(m_hVertexShader);
		m_hVertexShader = NULL;
	}

	if(m_hFragmentShader)
	{
		glDetachShader(m_hProgramObject, m_hFragmentShader);
		glDeleteShader(m_hFragmentShader);
		m_hFragmentShader = NULL;
	}

	if(m_hProgramObject)
	{
		glDeleteProgram(m_hProgramObject);
		m_hProgramObject = NULL;
	}
}

void TurnOffShader()
{
	glUseProgram(0);
}

void ReleaseShaders()
{
	for(int i=0; i<SHADERS; i++)
		g_shader[i].release();
}

