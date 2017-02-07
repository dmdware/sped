

#include "appmain.h"
#include "../gui/gui.h"
#include "../render/shader.h"
#include "res.h"
#include "../gui/font.h"
#include "../texture.h"
#include "../render/model2.h"
#include "../math/frustum.h"
#include "../render/billboard.h"
#include "segui.h"
#include "../render/particle.h"
#include "../render/shadow.h"
#include "../render/particle.h"
#include "../platform.h"
#include "../utils.h"
#include "../window.h"
#include "../sim/sim.h"
#include "../math/matrix.h"
#include "../math/vec3f.h"
#include "../math/math3d.h"
#include "../math/camera.h"
#include "sesim.h"
#include "../sim/entity.h"
#include "../save/compilemap.h"
#include "../save/modelholder.h"
#include "../tool/rendersprite.h"
#include "../tool/rendertopo.h"
#include "../debug.h"
#include "segui.h"
#include "../sim/tile.h"
#include "undo.h"
#include "../render/ordraw.h"

APPMODE g_mode = LOADING;
bool g_gameover = false;

void Draw()
{
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
    CHECKGLERROR();

    if(g_mode == LOADING)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else if(g_mode == EDITOR)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    else if(g_mode == RENDERING)
        //glClearColor(g_transpkey[0], g_transpkey[1], g_transpkey[2], 1.0f);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CHECKGLERROR();
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

    g_gui.frameupd();

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CHECKGLERROR();
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

    g_gui.draw();
    //DrawEdMap(&g_edmap);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CHECKGLERROR();
#endif

    if(g_mode == EDITOR)
    {
#ifdef DEBUG
        LastNum(__FILE__, __LINE__);
#endif
        Ortho(g_width, g_height, 1, 1, 1, 1);
        char dbgstr[128];
        sprintf(dbgstr, "b's:%d", (int)g_edmap.m_brush.size());
        RichText rdbgstr(dbgstr);
        DrawShadowedText(MAINFONT8, 0, g_height-16, &rdbgstr);

		if(g_rolllock)
		{
			char rlstr[123] = " Roll-lock enabled: orientability map \n object only has yaw and pitch \n rotations";
			RichText rlstrr = RichText(rlstr);
			float rlc[4] = {1,0,0,1};
			float w, h;
			float x, y;

			x = g_toprightviewport->m_pos[0];
			y = g_toprightviewport->m_pos[1]+16;
			w = g_toprightviewport->m_pos[2]-x;
			h = g_toprightviewport->m_pos[3]-y;

			DrawBoxShadText(MAINFONT8, x, y, w, h, &rlstrr, rlc, 0, -1);
			//DrawShadowedText(MAINFONT8, x, y, &rlstrr, rlc);

			//Vec3f objdir = Normalize( g_cam.m_view - g_cam.m_pos );
			//Vec3f objdir = Vec3f(0,0,0) - Normalize( g_cam.m_view - g_cam.m_pos );
			Vec3f objdir = Normalize( Vec3f(0,0,-1) );
			float orlon = GetLon(objdir.x, objdir.z);
			float orlat = GetLat(objdir.y);

			char datastr[123];
			sprintf(datastr, "objdir:%f,%f,%f   orlon=%f    orlat=%f",
				objdir.x, objdir.y, objdir.z, orlon, orlat);
			RichText rd = RichText(datastr);

			y = g_toprightviewport->m_pos[3]-16;
			
			DrawBoxShadText(MAINFONT8, x, y, w, h, &rd, rlc, 0, -1);
		}

        EndS();
    }

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

    SDL_GL_SwapWindow(g_window);
}

void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 0
    if(g_mode == RENDERING)
    {
        //SwapBuffers(g_hDC);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
        Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        CHECKGLERROR();
        glViewport(0, 0, g_width, g_height);
        CHECKGLERROR();
        glClearColor(1.0, 1.0, 1.0, 0.0);
        CHECKGLERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECKGLERROR();
        Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        Log(__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
    }
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif
	
#if 00
	if(g_rendtopo)
	{
		if(g_projtype == PROJ_ORTHO)
			UseShadow(SHADER_MODEL, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
		else
			UseShadow(SHADER_MODELPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, g_depth);
		glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
		
		DrawClip();
		//RenderToShadowMap(projection, viewmat, modelmat, g_cam.m_view, DrawClipDepth);
#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif
		//RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawClip);
		EndS();
		return;
	}
#endif

#if 1
	if(g_projtype == PROJ_ORTHO)
		UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	else
		UseShadow(SHADER_MAPPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTextureARB(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1iARB(g_shader[SHADER_OWNED].m_slot[SSLOT_TEXTURE1], 1);
    glActiveTextureARB(GL_TEXTURE0);*/
    DrawModelHolders();
#ifdef DEBUG
    CHECKGLERROR();
#endif
	//if(g_model2[0].m_on)
	//	g_model2[0].Render();
    EndS();
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#if 1
	if(g_projtype == PROJ_ORTHO)
		UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	else
		UseShadow(SHADER_MAPPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
#else
	UseShadow(SHADER_LIGHTTEST, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glUniformMatrix4fv(g_shader[SHADER_LIGHTTEST].m_slot[SSLOT_MVP], 1, GL_FALSE, g_lightmat.m_matrix);
#endif
    CHECKGLERROR();
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
	DrawTile();
	Matrix offmat;
	Vec3f offvec(-g_tilesize, 0, -g_tilesize);
	offmat.setTranslation((float*)&offvec);
	glUniformMatrix4fv(g_shader[g_curS].m_slot[SSLOT_MODELMAT], 1, GL_FALSE, offmat.m_matrix);
	//DrawTile();
	offvec = Vec3f(-g_tilesize, 0, 0);
	offmat.setTranslation((float*)&offvec);
	glUniformMatrix4fv(g_shader[g_curS].m_slot[SSLOT_MODELMAT], 1, GL_FALSE, offmat.m_matrix);
	//DrawTile();
    CHECKGLERROR();
    DrawEdMap(&g_edmap, g_showsky);
    CHECKGLERROR();
    EndS();

	if(g_orlist.on)
	//if(0)
	{
		glDisable(GL_CULL_FACE);
		if(g_projtype == PROJ_ORTHO)
			UseShadow(SHADER_OR, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
		else
		//g_projtype = PROJ_PERSP;
			UseShadow(SHADER_ORPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
		DrawOr(&g_orlist, g_renderframe, Vec3f(0,0,0), 
			M_PI*2.0f*(float)g_rendpitch/(float)g_nrendsides, 
			M_PI*2.0f*(float)g_rendyaw/(float)g_nrendsides);
		EndS();
	}
		

#if 0
	glDisable(GL_DEPTH_TEST);
    UseShadow(SHADER_COLOR3D, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	Matrix mvp;
	mvp.set(projection.m_matrix);
	mvp.postmult(viewmat);
	glUniformMatrix4fv(g_shader[g_curS].m_slot[SSLOT_MVP], 1, GL_FALSE, mvp.m_matrix);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 0, 0, 1, 0.2f);
	VertexArray* va = &g_tileva[g_currincline];
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	//glDrawArrays(GL_LINE_LOOP, 0, va->numverts);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 0, 0, 0.2f);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	glDrawArrays(GL_POINTS, 0, va->numverts);
    EndS();
	glEnable(GL_DEPTH_TEST);
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#if 0
    if(g_mode == RENDERING)
    {
        CHECKGLERROR();
        glFlush();
        CHECKGLERROR();
        //glFinish();
        CHECKGLERROR();
        SaveRender();
        CHECKGLERROR();
    }
#endif
}

void DrawSceneTeam(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{

#if 1
    UseShadow(SHADER_TEAM, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTextureARB(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1iARB(g_shader[SHADER_OWNED].m_slot[SSLOT_TEXTURE1], 1);
    glActiveTextureARB(GL_TEXTURE0);*/
    DrawModelHolders();
#ifdef DEBUG
    CHECKGLERROR();
#endif
    EndS();
#endif

    UseShadow(SHADER_TEAM, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    CHECKGLERROR();
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
	DrawTile();
    CHECKGLERROR();
    DrawEdMap(&g_edmap, g_showsky);
    CHECKGLERROR();
    EndS();
}

void DrawSceneDepth()
{
    //g_model[themodel].draw(0, Vec3f(0,0,0), 0);
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    //DrawModelHoldersDepth();
    DrawModelHolders();

#ifdef DEBUG
    CHECKGLERROR();
#endif
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
    //if(g_model[0].m_on)
    //	g_model[0].draw(0, Vec3f(0,0,0), 0);

    //DrawEdMapDepth(&g_edmap, false);
    DrawEdMap(&g_edmap, false);/*
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/

	DrawTile();
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
#ifdef DEBUG
    CHECKGLERROR();
#endif
}

void UpdateLoading()
{
    static int stage = 0;

    switch(stage)
    {
        //case 0: SetStatus("Loading textures...", true); stage++; break;
        //case 1: LoadTiles(); LoadMap(); LoadTerrainTextures(); LoadHoverTex(); LoadSkyBox("defsky"); SetStatus("Loading particles...", true); stage++; break;
        //case 2: LoadParticles(); SetStatus("Loading projectiles...", true); stage++; break;
        //case 3: LoadProjectiles(); SetStatus("Loading unit sprites...", true); stage++; break;
        //case 4: LoadUnitSprites(); SetStatus("Loading sounds...", true); stage++; break;
        //case 5: LoadSounds(); SetStatus("Loading Map sprites...", true); stage++; break;
        //case 6: BSprites(); SetStatus("Loading models...", true); stage++; break;
    //case 0:
   //     if(!Load1Model2()) stage++;
    //    break;
    //case 1:
    case 0:
        if(!Load1Texture()) stage++;
        break;
    //case 2:
    case 1:
        //SetStatus("logo");
        g_mode = EDITOR;
        g_gui.hideall();
        g_gui.show("editor");
        //g_mode = LOGO;
        //OpenSoleView("logo");
        //g_gui.show("choose file");
        break;
    }
}

int g_reStage = 0;
void UpdateReloading()
{
    switch(g_reStage)
    {
    case 0:
        if(Load1Texture())
        {
            g_mode = EDITOR;
            g_gui.hideall();
            g_gui.show("editor");
        }
        break;
    }
}

void UpdateGameState()
{
    //CalculateFrameRate();
    //Scroll();
    //LastNum("pre upd u");
    //UpdateUnits();
    //LastNum("pre upd b");
    //UpdateMaps();
    //LastNum("post upd b");
    //UpdateParticles();
    //LastNum("up pl");
    //UpdatePlayers();
    //LastNum("up ai");
    //UpdateAI();
    //ResourceTicker();
    //UpdateTimes();
    //UpdateFPS();
}

void UpdateEditor()
{
    if(g_keys[SDL_SCANCODE_W])
    {
        g_cam.accelerate(50.0f/g_zoom);
    }
    if(g_keys[SDL_SCANCODE_S])
    {
        g_cam.accelerate(-50.0f/g_zoom);
    }

    if(g_keys[SDL_SCANCODE_A])
    {
        g_cam.accelstrafe(-50.0f/g_zoom);
    }
    if(g_keys[SDL_SCANCODE_D])
    {
        g_cam.accelstrafe(50.0f/g_zoom);
    }

    if(g_keys[SDL_SCANCODE_R])
    {
        g_cam.accelrise(25.0f/g_zoom);
    }
    if(g_keys[SDL_SCANCODE_F])
    {
        g_cam.accelrise(-25.0f/g_zoom);
    }

    g_cam.frameupd();
    g_cam.friction2();
}

void Update()
{
    if(g_mode == LOADING)
        UpdateLoading();
    else if(g_mode == LOGO)
        UpdateLogo();
    //else if(g_mode == INTRO)
    //	UpdateIntro();
    //else if(g_mode == PLAY)
    //	UpdateGameState();
    else if(g_mode == EDITOR)
        UpdateEditor();
    else if(g_mode == RENDERING)
        UpdateRender();
	else if(g_mode == ORVIEW)
		UpdateOrient();
}

void LoadConfig()
{
    char cfgfull[SPE_MAX_PATH+1];
    FullPath(CONFIGFILE, cfgfull);

#if 0
    ifstream config(cfgfull);

    int fulls;
    config>>fulls;

    if(fulls)
        g_fullscreen = true;
    else
        g_fullscreen = false;

    config>>g_selres.width>>g_selres.height;
    config>>g_bpp;

    g_width = g_selres.width;
    g_height = g_selres.height;
#endif

    ifstream f(cfgfull);
    std::string line;
    char keystr[128];
    char actstr[128];

    while(!f.eof())
    {

#if 0
        key = -1;
        down = NULL;
        up = NULL;
#endif
        strcpy(keystr, "");
        strcpy(actstr, "");

        getline(f, line);
        sscanf(line.c_str(), "%s %s", keystr, actstr);

        float valuef = StrToFloat(actstr);
        int valuei = StrToInt(actstr);
        bool valueb = (bool)valuei;

        if(stricmp(keystr, "fullscreen") == 0)					g_fullscreen = valueb;
        else if(stricmp(keystr, "work_width") == 0)				g_width = g_selres.width = valuei;
        else if(stricmp(keystr, "work_height") == 0)			g_height = g_selres.height = valuei;
        else if(stricmp(keystr, "work_bpp") == 0)				g_bpp = valuei;
        else if(stricmp(keystr, "render_pitch") == 0)			g_defrenderpitch = valuef;
        else if(stricmp(keystr, "render_yaw") == 0)				g_defrenderyaw = valuef;
        else if(stricmp(keystr, "1_tile_pixel_width") == 0)		g_1tilewidth = valuei;
        else if(stricmp(keystr, "sun_x") == 0)					g_lightOff.x = valuef;
        else if(stricmp(keystr, "sun_y") == 0)					g_lightOff.y = valuef;
        else if(stricmp(keystr, "sun_z") == 0)					g_lightOff.z = valuef;
        else if(stricmp(keystr, "shadow_pass") == 0)			g_shadowpass = valueb;
        else if(stricmp(keystr, "antialias") == 0)				g_antialias = valueb;
        else if(stricmp(keystr, "tile_size_cm") == 0)			g_tilesize = valuei;
        else if(stricmp(keystr, "max_undo_steps") == 0)			g_maxundo = valuei;
        else if(stricmp(keystr, "render_sides") == 0)			g_nrendsides = valuei;
        else if(stricmp(keystr, "fit_to_2_power") == 0)			g_fit2pow = valueb;
		else if(stricmp(keystr, "export_depth") == 0)			g_exportdepth = valueb;
		else if(stricmp(keystr, "export_team") == 0)			g_exportteam = valueb;
		else if(stricmp(keystr, "hide_tex_err") == 0)			g_hidetexerr = valueb;
		else if(stricmp(keystr, "tile_rise_cm") == 0)			g_tilerisecm = valuef;
		else if(stricmp(keystr, "orientability_jump_plane_width_pixels") == 0)
		{			
			g_orwpx = valuei;
			//g_bigtex = imax( g_orwpx, g_orhpx ) * imax( g_orlons, g_orlats );
		}

		else if(stricmp(keystr, "orientability_jump_plane_height_pixels") == 0)
		{			
			g_orhpx = valuei;
			//g_bigtex = imax( g_orwpx, g_orhpx ) * imax( g_orlons, g_orlats );
		}

		else if(stricmp(keystr, "orientability_jump_longitudes") == 0)
		{			
			g_orlons = valuei;
			//g_bigtex = imax( g_orwpx, g_orhpx ) * imax( g_orlons, g_orlats );
		}

		else if(stricmp(keystr, "orientability_jump_latitudes") == 0)
		{			
			g_orlats = valuei;
			//g_bigtex = imax( g_orwpx, g_orhpx ) * imax( g_orlons, g_orlats );
		}
		else if(stricmp(keystr, "orientability_maps_size") == 0)
		{			
			g_bigtex = valuei;
			//g_bigtex = imax( g_orwpx, g_orhpx ) * imax( g_orlons, g_orlats );
		}
    }
}

#if 0
void WriteConfig()
{
    std::ofstream config;
    config.show(CONFIGFILE, std::ios_base::out);

    int fulls;
    if(g_fullscreen)
        fulls = 1;
    else
        fulls = 0;

    config<<fulls<<std::endl;
    config<<g_selres.width<<" "<<g_selres.height<<std::endl;
    config<<g_bpp;
}
#endif

/*
void EnumerateMaps()
{
	WIN32_FIND_DATA ffd;
	std::string bldgPath = ExePath() + "\\bldgs\\*";
	HANDLE hFind = FindFirstFile(bldgPath.c_str(), &ffd);

	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if(!strstr(ffd.cFileName, ".bsp"))
				continue;

			int pos = std::string( ffd.cFileName ).find_last_of( ".bsp" );
			std::string name = std::string( ffd.cFileName ).substr(0, pos-3);

			g_bldgs.push_back(name);
		} while(FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	else
	{
		// Failure
	}
}*/

bool DrawNextFrame(int desiredFrameRate)
{
	static long long lastTime = GetTicks();
	static long long elapsedTime = 0;

	long long currentTime = GetTicks(); // Get the time (milliseconds = seconds * .001)
	long long deltaTime = currentTime - lastTime; // Get the slice of time
	int desiredFPS = 1000 / (float)desiredFrameRate; // Store 1 / desiredFrameRate

	elapsedTime += deltaTime; // Add to the elapsed time
	lastTime = currentTime; // Update lastTime

	// Check if the time since we last checked is greater than our desiredFPS
	if( elapsedTime > desiredFPS )
	{
		elapsedTime -= desiredFPS; // Adjust the elapsed time

		// Return true, to animate the next frame of animation
		return true;
	}

	// We don't animate right now.
	return false;
	/*
	long long currentTime = GetTickCount();
	float desiredFPMS = 1000.0f/(float)desiredFrameRate;
	int deltaTime = currentTime - g_lasttime;

	if(deltaTime >= desiredFPMS)
	{
		g_lasttime = currentTime;
		return true;
	}

	return false;*/
}

void EventLoop()
{
#if 0
    key->keysym.scancode
    SDLMod  e.key.keysym.mod
    key->keysym.unicode

    if( mod & KMOD_NUM ) printf( "NUMLOCK " );
    if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
    if( mod & KMOD_LCTRL ) printf( "LCTRL " );
    if( mod & KMOD_RCTRL ) printf( "RCTRL " );
    if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
    if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
    if( mod & KMOD_RALT ) printf( "RALT " );
    if( mod & KMOD_LALT ) printf( "LALT " );
    if( mod & KMOD_CTRL ) printf( "CTRL " );
    if( mod & KMOD_SHIFT ) printf( "SHIFT " );
    if( mod & KMOD_ALT ) printf( "ALT " );
#endif

    //SDL_EnableUNICODE(SDL_ENABLE);

    GUI* gui = &g_gui;

    while (!g_quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
#ifdef DEBUG
			Log("ev "<<e.type<<std::endl;
#endif

            InEv ev;
            ev.intercepted = false;

            switch(e.type)
            {
            case SDL_WINDOWEVENT:
                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:

					//InfoMess("info", "resz");

                    if(g_mode == PREREND_ADJFRAME)
                    {
#ifdef DEBUG
                        Log("to r");
                        Log("rf"<<g_renderframe<<" rsz "<<g_deswidth<<","<<g_desheight<<std::endl;
#endif
                        Resize(g_deswidth, g_desheight);
                        g_mode = RENDERING;
                    }
                    else
                    {
#ifdef DEBUG
                        Log("rf"<<g_renderframe<<" rsz "<<e.window.data1<<","<<e.window.data2<<std::endl;
#endif
                        Resize(e.window.data1, e.window.data2);
                    }
                    break;
                default:
                    break;
                }
                break;
            case SDL_QUIT:
                g_quit = true;
                break;
            case SDL_KEYDOWN:
                ev.type = INEV_KEYDOWN;
                ev.key = e.key.keysym.sym;
                ev.scancode = e.key.keysym.scancode;

				//Handle copy
				if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
				{
					//SDL_SetClipboardText( inputText.c_str() );
					ev.type = INEV_COPY;
				}
				//Handle paste
				if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
				{
					//inputText = SDL_GetClipboardText();
					//renderText = true;
					ev.type = INEV_PASTE;
				}
				//Select all
				if( e.key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL )
				{
					//inputText = SDL_GetClipboardText();
					//renderText = true;
					ev.type = INEV_SELALL;
				}

                gui->inev(&ev);

                if(!ev.intercepted)
                    g_keys[e.key.keysym.scancode] = true;

                g_keyintercepted = ev.intercepted;
                break;
            case SDL_KEYUP:
                ev.type = INEV_KEYUP;
                ev.key = e.key.keysym.sym;
                ev.scancode = e.key.keysym.scancode;

                gui->inev(&ev);

                if(!ev.intercepted)
                    g_keys[e.key.keysym.scancode] = false;

                g_keyintercepted = ev.intercepted;
                break;
            case SDL_TEXTINPUT:
                //g_gui.charin(e.text.text);	//UTF8
                ev.type = INEV_TEXTIN;
                ev.text = e.text.text;

                //Log("SDL_TEXTINPUT:";
                //for(int i=0; i<strlen(e.text.text); i++)
                //{
                 //   Log("[#"<<(unsigned int)(unsigned char)e.text.text[i]<<"]";
                //}
                //Log(std::endl;
                //

                gui->inev(&ev);
                break;
            case SDL_TEXTEDITING:
                //g_gui.charin(e.text.text);	//UTF8
                ev.type = INEV_TEXTED;
                ev.text = e.text.text;
                ev.cursor = e.edit.start;
                ev.sellen = e.edit.length;

                //Log("SDL_TEXTEDITING:";
                //for(int i=0; i<strlen(e.text.text); i++)
                //{
                //    Log("[#"<<(unsigned int)(unsigned char)e.text.text[i]<<"]";
                //}
                //Log(std::endl;
                //

                //Log("texted: cursor:"<<ev.cursor<<" sellen:"<<ev.sellen<<std::endl;
                //

                gui->inev(&ev);
#if 0
                ev.intercepted = false;
                ev.type = INEV_TEXTIN;
                ev.text = e.text.text;

                gui->inev(&ev);
#endif
                break;
#if 0
            case SDL_TEXTINPUT:
                /* Add new text onto the end of our text */
                strcat(text, event.text.text);
#if 0
                ev.type = INEV_CHARIN;
                ev.key = wParam;
                ev.scancode = 0;

                gui->inev(&ev);
#endif
                break;
            case SDL_TEXTEDITING:
                /*
                   Update the composition text.
                   Update the cursor position.
                   Update the selection length (if any).
                 */
                composition = event.edit.text;
                cursor = event.edit.start;
                selection_len = event.edit.length;
                break;
#endif
                //else if(e.type == SDL_BUTTONDOWN)
                //{
                //}
            case SDL_MOUSEWHEEL:
                ev.type = INEV_MOUSEWHEEL;
                ev.amount = e.wheel.y;

                gui->inev(&ev);
            case SDL_MOUSEBUTTONDOWN:
                switch (e.button.button)
                {
                case SDL_BUTTON_LEFT:
                    g_mousekeys[MOUSE_LEFT] = true;
                    g_moved = false;

                    ev.type = INEV_MOUSEDOWN;
                    ev.key = MOUSE_LEFT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);

                    g_keyintercepted = ev.intercepted;
                    break;
                case SDL_BUTTON_RIGHT:
                    g_mousekeys[MOUSE_RIGHT] = true;

                    ev.type = INEV_MOUSEDOWN;
                    ev.key = MOUSE_RIGHT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                case SDL_BUTTON_MIDDLE:
                    g_mousekeys[MOUSE_MIDDLE] = true;

                    ev.type = INEV_MOUSEDOWN;
                    ev.key = MOUSE_MIDDLE;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (e.button.button)
                {
                case SDL_BUTTON_LEFT:
                    g_mousekeys[MOUSE_LEFT] = false;

                    ev.type = INEV_MOUSEUP;
                    ev.key = MOUSE_LEFT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                case SDL_BUTTON_RIGHT:
                    g_mousekeys[MOUSE_RIGHT] = false;

                    ev.type = INEV_MOUSEUP;
                    ev.key = MOUSE_RIGHT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                case SDL_BUTTON_MIDDLE:
                    g_mousekeys[MOUSE_MIDDLE] = false;

                    ev.type = INEV_MOUSEUP;
                    ev.key = MOUSE_MIDDLE;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                //g_mouse.x = e.motion.x;
                //g_mouse.y = e.motion.y;

                if(g_mouseout)
                {
                    //TrackMouse();
                    g_mouseout = false;
                }
                if(MousePosition())
                {
                    g_moved = true;

                    ev.type = INEV_MOUSEMOVE;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                }
                break;
            }
        }

		//continue;

        if(g_mode == LOADING || g_mode == RENDERING || DrawNextFrame(DRAW_FRAME_RATE))
        {
#ifdef DEBUG
			Log("calc draw rate");
			
            CHECKGLERROR();
#endif
            CalcDrawRate();
#ifdef DEBUG
			Log("score fps");
			
            CHECKGLERROR();
#endif
#ifdef DEBUG
			Log("update");
			
            LastNum(__FILE__, __LINE__);
            CHECKGLERROR();
#endif
            Update();
#ifdef DEBUG
			Log("draw");
			
            LastNum(__FILE__, __LINE__);
            CHECKGLERROR();
#endif
            Draw();
#ifdef DEBUG
            LastNum(__FILE__, __LINE__);
#endif
        }
    }
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void SignalCallback(int signum)
{
    //printf("Caught signal %d\n",signum);
    // Cleanup and hide up stuff here

    // Terminate program
    g_quit = true;
}

void Init()
{
#ifdef PLATFORM_LINUX
    signal(SIGINT, SignalCallback);
#endif

    SDL_Init(SDL_INIT_VIDEO);
    OpenLog("log.txt", APPVERSION);
    srand(GetTicks());
    g_lastsave[0] = '\0';
    LoadConfig();

		Log("asdasd");
		

	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	// Create a logger instance for Console Output
	Assimp::DefaultLogger::create("",severity, aiDefaultLogStream_STDOUT);
	// Create a logger instance for File Output (found in project folder or near .exe)
	Assimp::DefaultLogger::create("assimp_log.txt", severity, aiDefaultLogStream_FILE);
	// Now I am ready for logging my stuff
	Assimp::DefaultLogger::get()->info("this is my info-call");

    //EnumerateMaps();
    //EnumerateDisplay();
    //MapKeys();

    InitOnce();
}

void Deinit()
{
    DestroyEntities();
    BreakWin(TITLE);
    // Clean up
    SDL_Quit();
}

#ifdef PLATFORM_WIN
void MiniDumpFunction( uint32_t nExceptionCode, EXCEPTION_POINTERS *pException )
{
#ifdef USESTEAM
	// You can build and set an arbitrary comment to embed in the minidump here,
	// maybe you want to put what level the user was playing, how many players on the server,
	// how much memory is free, etc...
	SteamAPI_SetMiniDumpComment( "Minidump comment: SteamworksExample.exe\n" );

	// The 0 here is a build ID, we don't set it
	SteamAPI_WriteMiniDump( nExceptionCode, pException, 0 );
#endif
}
#endif

#ifdef USESTEAM
//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int32_t nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
#ifdef PLATFORM_WIN
	::OutputDebugString( pchDebugText );
#endif

	if(!g_applog)
		OpenLog("log.txt", APPVERSION);

	Log(pchDebugText);
	

	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int32_t x = 3;
		x = x;
	}
}
#endif

void Main(int argc, char* argv[])
{
	
#ifdef USESTEAM

	if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steam_appid.txt from the game depot.

		return;
	}

	// Init Steam CEG
	if ( !Steamworks_InitCEGLibrary() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "Steamworks_InitCEGLibrary() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (InitDrmLibrary() failed).\n" );
		return;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	if ( !SteamAPI_Init() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "SteamAPI_Init() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n" );
		return;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

#endif

	Init();

#ifdef PLATFORM_LINUX
	gtk_init(&argc, &argv);	//after sdl init
#endif

	//Log("MakeWindow: " << std::endl;
	

	MakeWin(TITLE);

	//exit(0);
	//Log("Queue: "      << std::endl;
	

	//SDL_ShowCursor(false);
	Queue();

//	Log("FillGUI: "    << std::endl;
	

	FillGUI();

//	Log("EventLoop: "  << std::endl;
	

	EventLoop();

//	Log("Deinit: "     << std::endl;
	

	Deinit();
	//SDL_ShowCursor(true);
}

#ifdef PLATFORM_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR argv, int argc)
#else
int main(int argc, char* argv[])
#endif
{
	//MessageBox(NULL, "asdasd", "asdasd", NULL);
	//Log("Log start"    << std::endl; /* TODO, include date */
	//Log("Init: "       << std::endl;
	//

#ifdef PLATFORM_WIN
	if ( IsDebuggerPresent() )
	{
		// We don't want to mask exceptions (or report them to Steam!) when debugging.
		// If you would like to step through the exception handler, attach a debugger
		// after running the game outside of the debugger.	

		Main(argc, &argv);
		return 0;
	}
#endif
	
#ifdef PLATFORM_WIN
#ifdef USESTEAM
	_set_se_translator( MiniDumpFunction );

	try  // this try block allows the SE translator to work
	{
#endif
#endif
		Main(argc, &argv);
#ifdef PLATFORM_WIN
#ifdef USESTEAM
	}
	catch( ... )
	{
		return -1;
	}
#endif
#endif

	return 0;
}



