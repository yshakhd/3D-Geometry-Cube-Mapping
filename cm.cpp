#include "tm.h"
#include "ppc.h"
#include "framebuffer.h"
#include "M33.h"
#include "scene.h"
#include "cm.h"

CubeMap::CubeMap(V3 center, int res) {
	for (int i = 0; i < 6; i++) {
		faces[i] = new FrameBuffer(100, 100, res, res);
		ppcs[i] = new PPC(90.0f, res, res);
	}

	ppcs[0]->PositionAndOrient(center, V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0f, -1.0f)); // up
	ppcs[1]->PositionAndOrient(center, V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f)); // left
	ppcs[2]->PositionAndOrient(center, V3(0.0f, 0.0f, 1.0f), V3(0.0f, 1.0f, 0.0f)); // forward
	ppcs[3]->PositionAndOrient(center, V3(-1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f)); // right
	ppcs[4]->PositionAndOrient(center, V3(0.0f, -1.0f, 0.0f), V3(0.0f, 0.0f, 1.0f)); // down
	ppcs[5]->PositionAndOrient(center, V3(0.0f, 0.0f, -1.0f), V3(0.0f, 1.0f, 0.0f)); // back
}

unsigned int CubeMap::LookUpColor(V3 dir) {
	int cidx;
	float s, t;
	for (int i = 0; i < 6; i++) {
		cidx = (pidx + i) % 6;
		
		V3 proj;
		if ((ppcs[cidx]->Project(ppcs[cidx]->C + dir, proj))) {
			s = proj[0] / faces[cidx]->w;
			t = proj[1] / faces[cidx]->h;

			if ((s > 0.0f) && (t > 0.0f) && (s < 1.0f) && (t < 1.0f)) {
				pidx = cidx;
				return faces[cidx]->LookUpBilinear(s, t);
			}
		}
	}
}

void CubeMap::LoadFaces(char* filename1, char* filename2, char* filename3, char* filename4, char* filename5, char* filename6) {
	faces[0]->LoadTiff(filename1);
	faces[1]->LoadTiff(filename2);
	faces[2]->LoadTiff(filename3);
	faces[3]->LoadTiff(filename4);
	faces[4]->LoadTiff(filename5);
	faces[5]->LoadTiff(filename6);
}