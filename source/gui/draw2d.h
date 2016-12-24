







#ifndef DRAW2D_H
#define DRAW2D_H

#define SPEDDEPTH	(28 + 256 * 200 + 256 * 256 * 7)

void DrawImage(unsigned int tex, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom, float *crop);
void DrawDeep(unsigned int difftex, unsigned int depthtex, int basedepth, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void DrawSquare(float r, float g, float b, float a, float left, float top, float right, float bottom, float *crop);
void DrawLine(float r, float g, float b, float a, float x1, float y1, float x2, float y2, float *crop);
void DrawDeepColor(float r, float g, float b, float a, float *v, int nv, GLenum mode);
void DrawDepth(unsigned int difftex, unsigned int depthtex,  unsigned int renderdepthtex, unsigned int renderfb, int basedepth, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);

void DrawSphericalBlend(unsigned int difftex, unsigned int depthtex, unsigned int renderdepthtex, unsigned int renderfb, float basedepth,
						float cx, float cy,
						float pixradius, float angle,
						float texleft, float textop, float texright, float texbottom);
#endif
