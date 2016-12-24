

#include "sound.h"
#include "utils.h"
#include "platform.h"

std::vector<CSound> g_concom;	//construction complete
std::vector<CSound> g_waifoo;	//waiting for orders
std::vector<CSound> g_acsnd;		//acknowledged
std::vector<CSound> g_undatk;	//under attack

CSound::CSound(const char* fp)
{
	strcpy(filepath, fp);
}

void CSound::play()
{
	//PlaySound(filepath, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
}

void LoadSounds()
{
	g_concom.push_back(CSound("sounds\\cc.wav"));
	g_concom.push_back(CSound("sounds\\cc2.wav"));
	g_concom.push_back(CSound("sounds\\cc3.wav"));
	g_waifoo.push_back(CSound("sounds\\wfo.wav"));
	g_waifoo.push_back(CSound("sounds\\wfo2.wav"));
	g_waifoo.push_back(CSound("sounds\\wfo3.wav"));
	g_acsnd.push_back(CSound("sounds\\a.wav"));
	g_acsnd.push_back(CSound("sounds\\a2.wav"));
	g_acsnd.push_back(CSound("sounds\\a3.wav"));
	g_undatk.push_back(CSound("sounds\\waua.wav"));
	g_undatk.push_back(CSound("sounds\\waua2.wav"));
	g_undatk.push_back(CSound("sounds\\waua3.wav"));
}

void AckSnd()
{
	if(g_acsnd.size() > 0)
		g_acsnd[ rand()%g_acsnd.size() ].play();
}

void ConCom()
{
	if(g_concom.size() > 0)
		g_concom[ rand()%g_concom.size() ].play();
}

void WaiFoO()
{
	if(g_waifoo.size() > 0)
		g_waifoo[ rand()%g_waifoo.size() ].play();
}
void WeAUA()
{
	if(g_undatk.size() > 0)
		g_undatk[ rand()%g_undatk.size() ].play();
}