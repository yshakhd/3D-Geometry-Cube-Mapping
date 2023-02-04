#pragma once

#include "V3.h"

class PPC {
public:
	V3 a, b, c, C;
	int w, h;
	PPC(float hfov, int _w, int _h);
	PPC(V3 _C, V3 _a, V3 _b, V3 _c);
	int Project(V3 P, V3 &pP);
	V3 GetVD();
	void Rotate(char key, float theta);
	void PositionAndOrient(V3 Cn, V3 L, V3 upg);
	float GetF();
	void Translate(char key, float move);
	void Zoom(char key, float dist);
	PPC Interpolate(PPC v1, float frac);
	void SavePPC(char* filename);
	void LoadPPC(char* filename);
	void Unproject(V3 pP, V3& P);
};