#pragma once

#include "V3.h"

class PPC;
class FrameBuffer;
class CubeMap;

class TM {
public:
	V3* verts;
	int vertsN;
	int* in_shadow;
	V3* cols;
	V3* normals; 
	V3* og_cols;
	V3* tcs;
	int SM; // shading mode
	int tilm; // tiling mode
	unsigned int *tris;
	FrameBuffer* texture;
	int trisN;
	TM() : verts(0), vertsN(0), tris(0), trisN(0), cols(0), SM(0), tcs(0), texture(0) {};
	void SetAsAACube(V3 cc, float sideLength);
	void LoadBin(char *fname);
	void RenderAsPoints(int psize, PPC *ppc, FrameBuffer *fb);
	void RenderWireFrame(PPC *ppc, FrameBuffer *fb);
	void RenderFilledTrisSM3(PPC* ppc, FrameBuffer* fb, PPC* lightppc, float* shmap);
	void RenderFilledTrisSM1_2(PPC* ppc, FrameBuffer* fb);
	void RenderFilledTrisCM(PPC* ppc, FrameBuffer* fb, CubeMap* env);
	V3 GetCenter();
	void Translate(V3 tv);
	void SetCenter(V3 newCenter);
	void Rotate(V3 aO, V3 aD, float theta);
	void Scale(float factor);
	void Light(float ka, V3 ld);
	void Lightpoint(V3 matColor, float ka, float sa, V3 point, PPC* lightppc, float* shmap);
	void SetFloor();
	void SetTexturedRectangle(float rectw, float recth, float tx, float ty);
	void SetOneTriangle(V3 v0, V3 v1, V3 v2);
};