#include "utils.h"
#include "platform.h"
#include "window.h"
#include "app/appmain.h"

FILE* g_applog = NULL;

const std::string DateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

const std::string TimeStr()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%X", &tstruct);

	return buf;
}

const std::string FileDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	for(int i=0; i<strlen(buf); i++)
		if(buf[i] == ':')
			buf[i] = '-';

	return buf;
}


void OpenLog(const char* filename, unsigned long long version)
{
	char fullpath[SPE_MAX_PATH+1];
	FullWritePath(filename, fullpath);
	if(g_applog)
		fclose(g_applog);
	//g_applog.close();
	g_applog = fopen(fullpath, "wb");
	Log("%s\r\n", DateTime().c_str());
	char verstr[64];
	//VerStr(version, verstr);
	Log("Version %d\r\n", APPVERSION);
	
}

void Log(const char* format, ...)
{
	if(!g_applog)
		return;

	char dest[1024 * 16];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(dest, format, argptr);
	va_end(argptr);
	fwrite(dest, strlen(dest), 1, g_applog);
	fwrite("\r\n", strlen("\r\n"), 1, g_applog);
	fflush(g_applog);
#ifdef PLATFORM_IOS
	NSLog(@"%s", dest);
#endif
}

std::string MakeRelative(const char* full)
{
	char full2c[SPE_MAX_PATH+1];
	strcpy(full2c, full);
	CorrectSlashes(full2c);
	std::string full2(full2c);
	char exepath[SPE_MAX_PATH+1];
	ExePath(exepath);
	CorrectSlashes(exepath);

	//Log("exepath: "<<exepath);
	//Log("fulpath: "<<full);

	std::string::size_type pos = full2.find(exepath, 0);

	if(pos == std::string::npos)
	{
		return full2;
	}

	//Log("posposp: "<<pos);

	std::string sub = std::string( full2 ).substr(strlen(exepath), strlen(full)-strlen(exepath));

	//Log("subpath: "<<sub);

	return sub;
}

std::string StripFile(std::string filepath)
{
	int lastof = filepath.find_last_of("/\\");
	if(lastof < 0)
		lastof = strlen(filepath.c_str());
	else
		lastof += 1;

	std::string stripped = filepath.substr(0, lastof);
	return stripped;
}

void StripPath(char* filepath)
{
	std::string s0(filepath);
	size_t sep = s0.find_last_of("\\/");
	std::string s1;

	if (sep != std::string::npos)
		s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	strcpy(filepath, s1.c_str());
}

void StripExt(char* filepath)
{
	std::string s1(filepath);

	size_t dot = s1.find_last_of(".");
	std::string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(filepath, s2.c_str());
}

void StripPathExt(const char* n, char* o)
{
	std::string s0(n);
	size_t sep = s0.find_last_of("\\/");
	std::string s1;

	if (sep != std::string::npos)
		s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	size_t dot = s1.find_last_of(".");
	std::string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(o, s2.c_str());
}

std::string PopDirB(const char* i)
{
	std::string o = i;

	if(o[ o.length()-1 ] == '/' || 
		o[ o.length()-1 ] == '\\')
		o[ o.length()-1 ] = 0;

	return StripFile(o.c_str());
}

std::string PopDirF(const char* i)
{
	std::string o = i;
	
	int lastof = o.find_first_of("/\\");
	if(lastof < 0)
		lastof = strlen(o.c_str());
	else
		lastof += 1;

	std::string stripped = o.substr(lastof, o.size() - lastof);
	return stripped;
}

bool TryLongerPath(char* szTemp, aiString* p_szString)
{
    char szTempB[SPE_MAX_PATH];
    strcpy(szTempB,szTemp);

    // go to the beginning of the file name
    char* szFile = strrchr(szTempB,'\\');
    if (!szFile)szFile = strrchr(szTempB,'/');

    char* szFile2 = szTemp + (szFile - szTempB)+1;
    szFile++;
    char* szExt = strrchr(szFile,'.');
    if (!szExt)return false;
    szExt++;
    *szFile = 0;

    strcat(szTempB,"*.*");
    const unsigned int iSize = (const unsigned int) ( szExt - 1 - szFile );

   // HANDLE          h;
   // WIN32_FIND_DATA info;

    // build a list of files
	std::list<std::string> files;
	ListFiles(szTempB, files);

    //h = FindFirstFile(szTempB, &info);
    //if (h != INVALID_HANDLE_VALUE)
	for(std::list<std::string>::iterator fit=files.begin(); fit!=files.end(); ++fit)
    {
       // do
        {
           // if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
            if (!(strcmp(fit->c_str(), ".") == 0 || strcmp(fit->c_str(), "..") == 0))
            {
               // char* szExtFound = strrchr(info.cFileName, '.');
                const char* szExtFound = strrchr(fit->c_str(), '.');
                if (szExtFound)
                {
                    ++szExtFound;
                    if (0 == stricmp(szExtFound,szExt))
                    {
                        const unsigned int iSizeFound = (const unsigned int) (
                            //szExtFound - 1 - info.cFileName);
                            szExtFound - 1 - fit->c_str());

                        for (unsigned int i = 0; i < iSizeFound;++i)
                            //info.cFileName[i] = (CHAR)tolower(info.cFileName[i]);
                            (*fit)[i] = (char)tolower( (*fit)[i]);
						
                        //if (0 == memcmp(info.cFileName,szFile2, min(iSizeFound,iSize)))
                        if (0 == memcmp(fit->c_str(),szFile2, imin(iSizeFound,iSize)))
                        {
                            // we have it. Build the full path ...
                            char* sz = strrchr(szTempB,'*');
                            *(sz-2) = 0x0;
							
                            //strcat(szTempB,info.cFileName);
                            strcat(szTempB,fit->c_str());

                            // copy the result string back to the aiString
                            const size_t iLen = strlen(szTempB);
                            size_t iLen2 = iLen+1;
                            iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
                            memcpy(p_szString->data,szTempB,iLen2);
                            p_szString->length = iLen;
                            return true;
                        }
                    }
#if 0
                    // check whether the 8.3 DOS name is matching
                   // if (0 == stricmp(info.cAlternateFileName,p_szString->data))
                    if (0 == stricmp(info.cAlternateFileName,p_szString->data))
                    {
                        strcat(szTempB,info.cAlternateFileName);

                        // copy the result string back to the aiString
                        const size_t iLen = strlen(szTempB);
                        size_t iLen2 = iLen+1;
                        iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
                        memcpy(p_szString->data,szTempB,iLen2);
                        p_szString->length = iLen;
                        return true;
                    }
#endif
                }
            }
        }
       // while (FindNextFile(h, &info));

       // FindClose(h);
    }
    return false;
}

int FindValidPath(aiString* p_szString, const char* g_szFileName)
{
    //ai_assert(NULL != p_szString);
    aiString pcpy = *p_szString;
    if ('*' ==  p_szString->data[0])    {
        // '*' as first character indicates an embedded file
        return 5;
    }

    // first check whether we can directly load the file
    FILE* pFile = fopen(p_szString->data,"rb");
    if (pFile)fclose(pFile);
    else
    {
        // check whether we can use the directory of  the asset as relative base
        char szTemp[SPE_MAX_PATH*2], tmp2[SPE_MAX_PATH*2];
        strcpy(szTemp, g_szFileName);
        strcpy(tmp2,szTemp);

        char* szData = p_szString->data;
        if (*szData == '\\' || *szData == '/')++szData;

        char* szEnd = strrchr(szTemp,'\\');
        if (!szEnd)
        {
            szEnd = strrchr(szTemp,'/');
            if (!szEnd)szEnd = szTemp;
        }
        szEnd++;
        *szEnd = 0;
        strcat(szEnd,szData);


        pFile = fopen(szTemp,"rb");
        if (!pFile)
        {
            // convert the string to lower case
            for (unsigned int i = 0;;++i)
            {
                if ('\0' == szTemp[i])break;
                szTemp[i] = (char)tolower(szTemp[i]);
            }

            if(TryLongerPath(szTemp,p_szString))return 1;
            *szEnd = 0;

            // search common sub directories
            strcat(szEnd,"tex\\");
            strcat(szEnd,szData);

            pFile = fopen(szTemp,"rb");
            if (!pFile)
            {
                if(TryLongerPath(szTemp,p_szString))return 1;

                *szEnd = 0;

                strcat(szEnd,"textures\\");
                strcat(szEnd,szData);

                pFile = fopen(szTemp,"rb");
                if (!pFile)
                {
                    if(TryLongerPath(szTemp, p_szString))return 1;
                }

                // patch by mark sibly to look for textures files in the asset's base directory.
                const char *path=pcpy.data;
                const char *p=strrchr( path,'/' );
                if( !p ) p=strrchr( path,'\\' );
                if( p ){
                    char *q=strrchr( tmp2,'/' );
                    if( !q ) q=strrchr( tmp2,'\\' );
                    if( q ){
                        strcpy( q+1,p+1 );
                        if((pFile=fopen( tmp2,"r" ))){
                            fclose( pFile );
                            strcpy(p_szString->data,tmp2);
                            p_szString->length = strlen(tmp2);
                            return 1;
                        }
                    }
                }
                return 0;
            }
        }
        fclose(pFile);

        // copy the result string back to the aiString
        const size_t iLen = strlen(szTemp);
        size_t iLen2 = iLen+1;
        iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
        memcpy(p_szString->data,szTemp,iLen2);
        p_szString->length = iLen;

    }
    return 1;
}


bool TryRelative(const char* relfile)
{
	char full[SPE_MAX_PATH+1];
	FullPath(relfile, full);

	FILE* fp = fopen(full, "rb");

	if(!fp)
		return false;

	fclose(fp);

	return true;
}

bool SmartPath(char* o, std::string* mod, const char* dir)
{
	if(strstr(o, "..\\") >= 0 ||
		strstr(o, "../") >= 0)
	{
		Log("Adjusting");
		std::string d2 = dir;
		std::string o2 = o;

		while(o2[0] == '.' &&
			o2[1] == '.')
		{
			Log("Snip");
			d2 = PopDirB(d2.c_str());
			o2 = PopDirF(o2.c_str());
		}

		*mod = d2 + o2;

		Log("Adjusted: %s", (*mod).c_str());

		*mod = MakeRelative(mod->c_str());

		//*mod = o2;
		return true;
	}

	if(strstr(o, ":/") >= 0 ||
		strstr(o, ":\\") >= 0)
	{
		Log("Adjusting");
		char* i = new char[ strlen(o)+1 ];
		strcpy(i, o);
		StripPath(i);
		*mod = i;
		Log("Adjusted: %s", i);
		delete [] i;
		*mod = MakeRelative( (std::string(dir) + *mod).c_str() );
		return true;
	}

	*mod = std::string(dir) + std::string(o);
	*mod = MakeRelative(mod->c_str());

	return false;
}

void ExePath(char* exepath)
{
#ifndef PLATFORM_IOS
#if 0
#ifdef PLATFORM_WIN
	//char buffer[SPE_MAX_PATH+1];
	GetModuleFileName(NULL, exepath, SPE_MAX_PATH+1);
	//std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
	//std::string strexepath = std::string( buffer ).substr( 0, pos);
	//strcpy(exepath, strexepath.c_str());
#else
	char szTmp[32];
	//char buffer[SPE_MAX_PATH+1];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = std::min((int)readlink(szTmp, exepath, SPE_MAX_PATH+1), SPE_MAX_PATH);
	if(bytes >= 0)
		exepath[bytes] = '\0';
	//std::string strexepath = StripFile(std::string(buffer));
	//strcpy(exepath, strexepath.c_str());
#endif
#else
	char *base_path = SDL_GetBasePath();
#if 0
	if (base_path) {
        data_path = SDL_strdup(base_path);
        SDL_free(base_path);
    } else {
        data_path = SDL_strdup("./");
    }
#endif
	if(base_path)
	{
		strcpy(exepath, base_path);
		SDL_free(base_path);
	}
#endif
#endif
	
#ifdef PLATFORM_IOS
	char *base_path = SDL_GetBasePath();
	if(base_path)
	{
		strcpy(exepath, base_path);
		SDL_free(base_path);
	}
	strcat(exepath, "testfolder/");
#endif
}

void FullWritePath(const char* filename, char* full)
{
#ifdef PLATFORM_IOS
	/*
	 char exepath[SPE_MAX_PATH+1];
	 GetModuleFileName(NULL, exepath, SPE_MAX_PATH);
	 string path = StripFile(exepath);
	 
	 //char full[SPE_MAX_PATH+1];
	 sprintf(full, "%s", path.c_str());
	 
	 char c = full[ strlen(full)-1 ];
	 if(c != '\\' && c != '/')
	 strcat(full, "\\");
	 
	 strcat(full, filename);*/
	
	//NSString *path = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/"];
	//NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	//NSString *path = [paths objectAtIndex:0];
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *path = [paths objectAtIndex:0];
	sprintf(full, "%s/%s", [path cStringUsingEncoding:NSUTF8StringEncoding], filename);
	
	//NSLog(@"full write %s", full);
#else
	
	FullPath(filename, full);
#endif
}

void FullPath(const char* filename, char* full)
{
#if 1
	char exepath[SPE_MAX_PATH+1];
	ExePath(exepath);
	std::string path = StripFile(exepath);

	//char full[SPE_MAX_PATH+1];
	sprintf(full, "%s", path.c_str());

	char c = full[ strlen(full)-1 ];
	if(c != '\\' && c != '/')
		strcat(full, "\\");
	//strcat(full, "/");

	strcat(full, filename);
	CorrectSlashes(full);
#else
	strcpy(full, filename);
	CorrectSlashes(full);
#endif
}

float StrToFloat(const char *s)
{
	if(s[0] == '\0')
		return 1.0f;

	float x;
	std::istringstream iss(s);
	iss >> x;

	if(ISNAN(x))
		x = 1.0f;

	return x;
}

int HexToInt(const char* s)
{
	int x;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> x;
	return x;
}

int StrToInt(const char *s)
{
	int x;
	std::istringstream iss(s);
	iss >> x;
	return x;
}

void CorrectSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '\\')
			corrected[i] = '/';
}

void BackSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';
}

void ErrMess(const char* title, const char* message)
{
	//SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);
	//SDL_ShowCursor(false);
}

void InfoMess(const char* title, const char* message)
{
	//SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, NULL);
	//SDL_ShowCursor(false);
}

void WarnMess(const char* title, const char* message)
{
	//SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, message, NULL);
	//SDL_ShowCursor(false);
}

void OutOfMem(const char* file, int line)
{
	char msg[2048];
	sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	ErrMess("Out of memory", msg);
	//g_quit = true;
}

std::string LoadTextFile(char* strFile)
{
	char full[SPE_MAX_PATH+1];
	FullPath(strFile, full);
	
	std::ifstream fin(full);

	if(!fin)
	{
		Log("Failed to load file %s\r\n", strFile);
		return "";
	}

	std::string strLine = "";
	std::string strText = "";

	getline(fin, strText);	//corpd fix

	while(getline(fin, strLine))
		strText = strText + "\n" + strLine;

	fin.close();

	return strText;
}

void ListFiles(const char* fullpath, std::list<std::string>& files)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp  = opendir(fullpath)) == NULL)
	{
		Log( "Error opening %s", fullpath);
		return;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if(dirp->d_type != DT_REG)
			continue;

		files.push_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return;
}

void ListDirs(const char* fullpath, std::list<std::string>& dirs)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp  = opendir(fullpath)) == NULL)
	{
		Log("Error opening %s", fullpath);
		return;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if(dirp->d_type != DT_DIR)
			continue;
		
		if(strcmp(dirp->d_name, ".") == 0)
			continue;
		if(strcmp(dirp->d_name, "..") == 0)
			continue;

		dirs.push_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return;
}

//add thousands separators
std::string iform(int n)
{
	std::string s;

	char first[32];
	sprintf(first, "%d", n);

	for(int i=strlen(first)-3; i>=0; i-=3)
	{
		if(first[i] == '-' || i == 0 || first[i-1] == '-')
			break;

		int newl = strlen(first)+1;

		for(int j=newl; j>=i; j--)
		{
			first[j+1] = first[j];
		}

		first[i] = ',';
	}

	s = first;

	return s;
}

std::string ullform(unsigned long long n)
{
	std::string s;

	char first[32];
	sprintf(first, "%llu", n);

	for(int i=strlen(first)-3; i>=0; i-=3)
	{
		if(first[i] == '-' || i == 0 || first[i-1] == '-')
			break;

		int newl = strlen(first)+1;

		for(int j=newl; j>=i; j--)
		{
			first[j+1] = first[j];
		}

		first[i] = ',';
	}

	s = first;

	return s;
}

unsigned long long GetTicks()
{
	//return time(0);
#ifdef PLATFORM_WIN
	//return GetTicks();
	SYSTEMTIME st;
	GetSystemTime (&st);
	_FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	//LARGE_INTEGER lint;
	//lint.HighPart = ft.dwHighDateTime;
    //lint.LowPart = ft.dwLowDateTime;
	//convert from 100-nanosecond intervals to milliseconds
	return (*(unsigned long long*)&ft)/(10*1000);
#else
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return
    (unsigned long long)(tv.tv_sec) * 1000 +
    (unsigned long long)(tv.tv_usec) / 1000;
#endif
}

#if 0
unsigned long long Add64(unsigned long long a, unsigned long long b)
{
	_asm
	{

	}

	return 0;
}
#endif
