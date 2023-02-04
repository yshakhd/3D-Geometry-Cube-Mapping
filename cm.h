#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"

class CubeMap {
public:
	FrameBuffer* faces[6];
	PPC* ppcs[6];
	int pidx; // previous face index
	CubeMap(V3 center, int res);
	unsigned int LookUpColor(V3 refldir);
	void LoadFaces(char* filename1, char* filename2, char* filename3, char* filename4, char* filename5, char* filename6);
};