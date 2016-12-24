



#include "debug.h"
#include "utils.h"
#include "platform.h"
#include "render/shader.h"

void LastNum(const char* l)
{
	//return;
#if 1
	std::ofstream last;
	char filepath[SPE_MAX_PATH+1];
	FullPath("last.txt", filepath);
	last.open(filepath, std::ios_base::out);
	last<<l;
	last.flush();
#else
	Log(l<<std::endl;
	
#endif
}

void LastNum(const char* f, const int line)
{
	return;
//	Log(f<<":"<<line<<std::endl;
	
}

#ifdef GLDEBUG
void CheckGLError(const char* file, int line)
{
	//char msg[2048];
	//sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	//ErrMess("Out of memory", msg);
	int error = glGetError();

	if(error == GL_NO_ERROR)
		return;

	Log("GL Error #"<<error<<" in "<<file<<" on line "<<line<<" using shader #"<<g_curS<<std::endl;
}
#endif

GLvoid APIENTRY GLMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
   //ErrMess("GL Error", message);
//   Log("GL Message: "<<message<<std::endl;
}
