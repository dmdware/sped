#ifndef UTILS_H
#define UTILS_H

#include "platform.h"

#define CORRECT_SLASH '/'

#define ARRSZ(a)	(sizeof(a)/sizeof(a[0]))

extern FILE* g_applog;

const std::string DateTime();
const std::string TimeStr();
const std::string FileDateTime();
void FullPath(const char* filename, char* full);
void FullWritePath(const char* filename, char* full);
std::string MakeRelative(const char* full);
void ExePath(char* exepath);
std::string StripFile(std::string filepath);
void StripPathExt(const char* n, char* o);
void StripExt(char* filepath);
void StripPath(char* filepath);
bool SmartPath(char* o, std::string* mod, const char* dir);
bool TryRelative(const char* relfile);
int FindValidPath(aiString* p_szString, const char* g_szFileName);
void OpenLog(const char* filename, unsigned long long version);
void Log(const char* format, ...);
float StrToFloat(const char *s);
int HexToInt(const char* s);
int StrToInt(const char *s);
void CorrectSlashes(char* corrected);
void BackSlashes(char* corrected);
void ErrMess(const char* title, const char* message);
void InfoMess(const char* title, const char* message);
void WarnMess(const char* title, const char* message);
void OutOfMem(const char* file, int line);
unsigned long long Add64(unsigned long long a, unsigned long long b);

inline float fmax(const float a, const float b)
{
	return (((a)>(b))?(a):(b));
}

inline float fmin(const float a, const float b)
{
	return (((a)<(b))?(a):(b));
}

inline int imax(const int a, const int b)
{
	return (((a)>(b))?(a):(b));
}

inline int imin(const int a, const int b)
{
	return (((a)<(b))?(a):(b));
}

#define enmax(a,b) (((a)>(b))?(a):(b))
#define enmin(a,b) (((a)<(b))?(a):(b))

inline int iabs(int x)
{
	//return x & INT_MAX;
	//return x & 0x7FFFFFFF;
	return x < 0 ? -x : x;
}

//deterministic ceil
inline int iceil(const int num, const int denom)
{
	if(denom  == 0)
		return 0;

	int div = num / denom;
	const int mul = div * denom;
	const int rem = num - mul;

	if(rem > 0)
		div += 1;

	return div;
}

std::string iform(int n);
std::string ullform(unsigned long long n);
unsigned long long GetTicks();

std::string LoadTextFile(char* strFile);
void ListFiles(const char* fullpath, std::list<std::string>& files);
void ListDirs(const char* fullpath, std::list<std::string>& dirs);

#ifdef PLATFORM_WIN
void MiniDumpFunction( unsigned int nExceptionCode, EXCEPTION_POINTERS *pException );
#endif

#define OUTOFMEM()		OutOfMem(__FILE__,__LINE__)

#endif	//UTILS_H
