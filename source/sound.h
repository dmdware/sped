

#ifndef SOUND_H
#define SOUND_H

#include <vector>

using namespace std;

class CSound
{
public:
	CSound() {}
	CSound(const char* fp);

	char filepath[64];
	void play();
};

extern std::vector<CSound> g_concom;	//construction complete
extern std::vector<CSound> g_waifoo;	//waiting for orders
extern std::vector<CSound> g_acsnd;		//acknowledged

void LoadSounds();
void ConCom();
void WaiFoO();
void AckSnd();
void WeAUA();

#endif