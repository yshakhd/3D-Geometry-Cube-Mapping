#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>
#include "M33.h"
#include "V3.h"

class PPC;
class CubeMap;

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int* pix; // pixel array
	float* zb; // z buffer to resolve visibility
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void Set(int u, int v, int col);
	void SetGuarded(int u, int v, int col);
	void SetChecker(unsigned int col0, unsigned int col1, int csize);
	void DrawAARectangle(V3 tlc, V3 brc, unsigned int col);
	void DrawDisk(V3 center, float r, unsigned int col);
	void DrawTriangleShadow(PPC* ppc, V3 p0, V3 p1, V3 p2, V3 col0, V3 col1, V3 col2, V3 z_abc, M33 col_abc, M33 norm_abc, PPC* lightppc, float* shmap, M33 tc_abc, FrameBuffer* texture);
	void DrawTriangleSM1_2(V3 p0, V3 p1, V3 p2, V3 col0, V3 col1, V3 col2, V3 z_abc, M33 col_abc, M33 tc_abc, FrameBuffer* texture);
	void DrawTriangleCM(PPC* ppc, V3 p0, V3 p1, V3 p2, V3 z_abc, M33 norm_abc, CubeMap* cm);
	void DrawSegment(V3 p0, V3 p1, unsigned int col);
	void DrawSegment(V3 p0, V3 p1, V3 c0, V3 c1);
	void Render3DSegment(PPC* ppc, V3 v0, V3 v1, V3 c0, V3 c1);
	void LoadTiff(char* fname);
	void SaveAsTiff(char* fname);
	void SetZB(float zf);
	int IsCloserThenSet(float currz, int u, int v);
	void RenderCamera(PPC* ppc, PPC* ppc3);
	void Render3DCircle(PPC* ppc, V3 v0, V3 col, float r);
	unsigned int LookUpBilinear(float s, float t);
	unsigned int LookUpNN(float s, float t);
	unsigned int Get(int u, int v);
	void RenderCM(PPC* ppc, CubeMap* cm);
};