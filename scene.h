#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"
#include "cm.h"

class Scene {
public:

	GUI* gui;
	FrameBuffer* fb, * fb3;
	PPC* ppc, * ppc3, * lightppc;
	TM* tms;
	int tmsN;
	float ka = 0.05f;
	float sa = 25;
	float shmap[512*512];
	CubeMap* envmap;
	Scene();
	void Render(PPC* renderPPC, FrameBuffer* renderFB);
	void Render(PPC* renderPPC, FrameBuffer* renderFB, V3 source);
	void Render();
	void DBG();
	void NewButton();
	void SM1();
	void SM2();
	void SM3();
	void AMB();
	void SPEC();
	void ClearShmap();
	void ComputeShmap();
	void TilingMode();
};

extern Scene* scene;