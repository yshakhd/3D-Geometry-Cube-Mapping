#include "tm.h"
#include "ppc.h"
#include "framebuffer.h"
#include "M33.h"
#include "scene.h"
#include "cm.h"

using namespace std;

#include <fstream>
#include <iostream>

void TM::SetAsAACube(V3 cc, float sideLength) {

	vertsN = 8;
	verts = new V3[vertsN];
	float sl2 = sideLength / 2.0f;
	verts[0] = cc + V3(-sl2, +sl2, +sl2);
	verts[1] = cc + V3(-sl2, -sl2, +sl2);
	verts[2] = cc + V3(+sl2, -sl2, +sl2);
	verts[3] = cc + V3(+sl2, +sl2, +sl2);
	verts[4] = cc + V3(-sl2, +sl2, -sl2);
	verts[5] = cc + V3(-sl2, -sl2, -sl2);
	verts[6] = cc + V3(+sl2, -sl2, -sl2);
	verts[7] = cc + V3(+sl2, +sl2, -sl2);

	trisN = 12;
	tris = new unsigned int[trisN*3];
	int tri = 0;
	tris[3 * tri + 0] = 0; // tris[0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;
	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;
	tri++;

	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 6;
	tri++;
	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 3;
	tri++;

	tris[3 * tri + 0] = 7;
	tris[3 * tri + 1] = 6;
	tris[3 * tri + 2] = 5;
	tri++;
	tris[3 * tri + 0] = 5;
	tris[3 * tri + 1] = 4;
	tris[3 * tri + 2] = 7;
	tri++;

	tris[3 * tri + 0] = 4;
	tris[3 * tri + 1] = 5;
	tris[3 * tri + 2] = 1;
	tri++;
	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 0;
	tris[3 * tri + 2] = 4;
	tri++;


	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 5;
	tris[3 * tri + 2] = 6;
	tri++;
	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 1;
	tri++;

	tris[3 * tri + 0] = 4;
	tris[3 * tri + 1] = 0;
	tris[3 * tri + 2] = 3;
	tri++;
	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 4;
	tri++;

	cols = new V3[vertsN];
	cols[0] =
		cols[1] =
		cols[2] =
		cols[3] = V3(1.0f, 0.0f, 0.0f);
	cols[4] =
		cols[5] =
		cols[6] =
		cols[7] = V3(0.0f, 0.0f, 0.0f);
}

#if 0
    4      7

0     3
    5      6
1     2
#endif

void TM::LoadBin(char* fname) {

		ifstream ifs(fname, ios::binary);
		if (ifs.fail()) {
			cerr << "INFO: cannot open file: " << fname << endl;
			return;
		}

		ifs.read((char*)&vertsN, sizeof(int));
		char yn;
		ifs.read(&yn, 1); // always xyz
		if (yn != 'y') {
			cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
			return;
		}
		if (verts)
			delete verts;
		verts = new V3[vertsN];

		ifs.read(&yn, 1); // cols 3 floats
		normals = 0;
		if (cols)
			delete cols;
		cols = 0;
		if (yn == 'y') {
			cols = new V3[vertsN];
			og_cols = new V3[vertsN];
		}

		ifs.read(&yn, 1); // normals 3 floats
		if (normals)
			delete normals;
		normals = 0;
		if (yn == 'y') {
			normals = new V3[vertsN];
		}

		ifs.read(&yn, 1); // texture coordinates 2 floats
		float* tcs = 0; // don't have texture coordinates for now
		if (tcs)
			delete tcs;
		tcs = 0;
		if (yn == 'y') {
			tcs = new float[vertsN * 2];
		}


		ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

		if (cols) {
			ifs.read((char*)cols, vertsN * 3 * sizeof(float)); // load cols
			memcpy(og_cols, cols, vertsN * 3 * sizeof(float));
		}

		if (normals)
			ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

		if (tcs)
			ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

		ifs.read((char*)&trisN, sizeof(int));
		if (tris)
			delete tris;
		tris = new unsigned int[trisN * 3];
		ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

		in_shadow = new int[trisN];

		ifs.close();

		cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
		cerr << "      xyz " << ((cols) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	}


void TM::RenderAsPoints(int psize, PPC *ppc, FrameBuffer *fb) {

	for (int vi = 0; vi < vertsN; vi++) {
		V3 pp;
		if (!ppc->Project(verts[vi], pp))
			continue;
		fb->DrawDisk(pp, (float)psize, cols[vi].GetColor());
	}

}

void TM::RenderWireFrame(PPC *ppc, FrameBuffer *fb) {

	for (int tri = 0; tri < trisN; tri++) {
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = verts[tris[3 * tri + ei]];
			V3 v1 = verts[tris[3 * tri + (ei + 1) % 3]];
			V3 c0 = cols[tris[3 * tri + ei]];
			V3 c1 = cols[tris[3 * tri + (ei + 1) % 3]];
			V3 pv0, pv1;
			if (!ppc->Project(v0, pv0))
				continue;
			if (!ppc->Project(v1, pv1))
				continue;
			fb->DrawSegment(pv0, pv1, c0, c1);
		}
	}

}

void TM::RenderFilledTrisSM1_2(PPC* ppc, FrameBuffer* fb) {

	for (int tri = 0; tri < trisN; tri++) {
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = verts[tris[3 * tri + ei]];
			V3 v1 = verts[tris[3 * tri + (ei + 1) % 3]];
			V3 v2 = verts[tris[3 * tri + (ei + 2) % 3]];
			V3 c0 = cols[tris[3 * tri + ei]];
			V3 c1 = cols[tris[3 * tri + (ei + 1) % 3]];
			V3 c2 = cols[tris[3 * tri + (ei + 2) % 3]];
			
			V3 ttc0, ttc1, ttc2;
			if (tcs) {
				ttc0 = tcs[tris[3 * tri + ei]];
				ttc1 = tcs[tris[3 * tri + (ei + 1) % 3]];
				ttc2 = tcs[tris[3 * tri + (ei + 2) % 3]];
			}
			else {
				ttc0 = V3(0.0f, 0.0f, 0.0f);
				ttc1 = V3(0.0f, 0.0f, 0.0f);
				ttc2 = V3(0.0f, 0.0f, 0.0f);
			}

			V3 pv0, pv1, pv2;
			if (!ppc->Project(v0, pv0))
				continue;
			if (!ppc->Project(v1, pv1))
				continue;
			if (!ppc->Project(v2, pv2))
				continue;
			M33 mat = M33(pv0, pv1, pv2);
			mat[0][2] = 1;
			mat[1][2] = 1;
			mat[2][2] = 1;

			M33 invmat = mat.Inverted();
			V3 z_abc = invmat * V3(pv0[2], pv1[2], pv2[2]);
			M33 col_abc = invmat * M33(c0, c1, c2);
			M33 tc_abc = invmat * M33(ttc0, ttc1, ttc2);
			//M33 letcs = (ssim * tcm).Transposed();
			fb->DrawTriangleSM1_2(pv0, pv1, pv2, c0, c1, c2, z_abc, col_abc, tc_abc, this->texture);
		}
	}

}

float OddEvenTilingCheck(float c) {
	return 1 - c - float(int(c));
}

void TM::RenderFilledTrisSM3(PPC* ppc, FrameBuffer* fb, PPC* lightppc, float* shmap) {

	for (int tri = 0; tri < trisN; tri++) {
		V3 v0 = verts[tris[3 * tri]];
		V3 v1 = verts[tris[3 * tri + 1]];
		V3 v2 = verts[tris[3 * tri + 2]];
		V3 c0 = cols[tris[3 * tri]];
		V3 c1 = cols[tris[3 * tri + 1]];
		V3 c2 = cols[tris[3 * tri + 2]];
		V3 n0 = normals[tris[3 * tri]];
		V3 n1 = normals[tris[3 * tri + 1]];
		V3 n2 = normals[tris[3 * tri + 2]];

		V3 ttc0, ttc1, ttc2;
		if (tcs) {
			ttc0 = tcs[tris[3 * tri]];
			ttc1 = tcs[tris[3 * tri + 1]];
			ttc2 = tcs[tris[3 * tri + 2]];
		}
		else {
			ttc0 = V3(0.0f, 0.0f, 0.0f);
			ttc1 = V3(0.0f, 0.0f, 0.0f);
			ttc2 = V3(0.0f, 0.0f, 0.0f);
		}

		V3 pv0, pv1, pv2;
		if (!ppc->Project(v0, pv0))
			continue;
		if (!ppc->Project(v1, pv1))
			continue;
		if (!ppc->Project(v2, pv2))
			continue;
		M33 mat = M33(pv0, pv1, pv2);
		mat[0][2] = 1;
		mat[1][2] = 1;
		mat[2][2] = 1;

		M33 invmat = mat.Inverted();
		V3 z_abc = invmat * V3(pv0[2], pv1[2], pv2[2]);
		M33 col_abc = invmat * M33(c0, c1, c2);
		M33 norm_abc = invmat * M33(n0, n1, n2);
		M33 tc_abc = invmat * M33(ttc0, ttc1, ttc2);
		M33 model_abc;
		M33 ps;
		ps.SetColumn(0, v0 - ppc->C);
		ps.SetColumn(1, v1 - ppc->C);
		ps.SetColumn(2, v2 - ppc->C);

		model_abc.SetColumn(0, ppc->a);
		model_abc.SetColumn(1, ppc->b);
		model_abc.SetColumn(2, ppc->c);

		M33 qmat = ps.Inverted() * model_abc;

		if(!texture) fb->DrawTriangleShadow(ppc, pv0, pv1, pv2, c0, c1, c2, z_abc, col_abc, norm_abc, lightppc, shmap, tc_abc, this->texture);
		else {
			int umin = (int)(min(min(pv0[0], pv1[0]), pv2[0]) - 0.5);
			int umax = (int)(max(max(pv0[0], pv1[0]), pv2[0]) + 0.5);
			int vmin = (int)(min(min(pv0[1], pv1[1]), pv2[1]) - 0.5);
			int vmax = (int)(max(max(pv0[1], pv1[1]), pv2[1]) + 0.5);

			if ((umax < 0) || (vmax < 0) || (umin >= lightppc->w) || (vmin >= lightppc->h))
				return;
			if (umin < 0)
				umin = 0;
			if (vmin < 0)
				vmin = 0;
			if (umax >= lightppc->w)
				umax = lightppc->w - 1;
			if (vmax >= lightppc->h)
				vmax = lightppc->h - 1;

			V3 edge1(pv1[1] - pv0[1], pv0[0] - pv1[0], -pv0[0] * pv1[1] + pv0[1] * pv1[0]);
			float sidedness = edge1[0] * pv2[0] + edge1[1] * pv2[1] + edge1[2];
			if (sidedness < 0) {
				edge1[0] = -edge1[0];
				edge1[1] = -edge1[1];
				edge1[2] = -edge1[2];
			}

			V3 edge2(pv2[1] - pv1[1], pv1[0] - pv2[0], -pv1[0] * pv2[1] + pv1[1] * pv2[0]);
			sidedness = edge2[0] * pv0[0] + edge2[1] * pv0[1] + edge2[2];
			if (sidedness < 0) {
				edge2[0] = -edge2[0];
				edge2[1] = -edge2[1];
				edge2[2] = -edge2[2];
			}

			V3 edge3(pv2[1] - pv0[1], pv0[0] - pv2[0], -pv0[0] * pv2[1] + pv0[1] * pv2[0]);
			sidedness = edge3[0] * pv1[0] + edge3[1] * pv1[1] + edge3[2];
			if (sidedness < 0) {
				edge3[0] = -edge3[0];
				edge3[1] = -edge3[1];
				edge3[2] = -edge3[2];
			}

			float edge1_p2 = edge1[0] * pv2[0] + edge1[1] * pv2[1] + edge1[2];
			float edge2_p0 = edge2[0] * pv0[0] + edge2[1] * pv0[1] + edge2[2];
			float edge3_p1 = edge3[0] * pv1[0] + edge3[1] * pv1[1] + edge3[2];

			for (int v = vmin; v <= vmax; v++) {
				for (int u = umin; u <= umax; u++) {
					float edge1_cp = edge1[0] * u + edge1[1] * v + edge1[2];
					float edge2_cp = edge2[0] * u + edge2[1] * v + edge2[2];
					float edge3_cp = edge3[0] * u + edge3[1] * v + edge3[2];

					if ((edge1_p2 * edge1_cp > 0) && (edge2_p0 * edge2_cp > 0) && (edge3_p1 * edge3_cp > 0)) {
						float z_depth = V3(u, v, 1) * z_abc;
						if (fb->IsCloserThenSet(z_depth, u, v)) {
							V3 norm = norm_abc.Transposed() * V3(u, v, 1);
							V3 point;
							ppc->Unproject(V3(u, v, z_depth), point);

							V3 col = col_abc.Transposed() * V3(u, v, 1);
							
							float s = (qmat.Transposed() * V3(ttc0[0], ttc1[0], ttc2[0]) * V3(u, v, 1.f)) / (qmat.Transposed() * V3(1.f, 1.f, 1.f) * V3(u, v, 1.f));
							float t = (qmat.Transposed() * V3(ttc0[1], ttc1[1], ttc2[1]) * V3(u, v, 1.f)) / (qmat.Transposed() * V3(1.f, 1.f, 1.f) * V3(u, v, 1.f));

							if (tilm == 1) { // tiling
								if(s > 1)
									s -= (int)s;
								if(t > 1)
									t -= (int)t;
							}
							else {
								if (s > 1) {
									if ((int)s % 2 == 0)
										s -= (int)s;
									else
										s = 1 - s + (float)((int)s);
								}
								if (t > 1) {
									if ((int)t % 2 == 0)
										t -= (int)t;
									else
										t = 1 - t + (int)t;
								}
							}

							// fb->Set(u, v, texture->LookUpNN(currtcs[0], currtcs[1]));
							fb->SetGuarded(u, v, texture->LookUpBilinear(s, t));
						}

					}
				}
			}
		}
	}
}

void TM::RenderFilledTrisCM(PPC* ppc, FrameBuffer* fb, CubeMap* env) {

	for (int tri = 0; tri < trisN; tri++) {
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = verts[tris[3 * tri + ei]];
			V3 v1 = verts[tris[3 * tri + (ei + 1) % 3]];
			V3 v2 = verts[tris[3 * tri + (ei + 2) % 3]];
			V3 n0 = normals[tris[3 * tri + ei]];
			V3 n1 = normals[tris[3 * tri + (ei + 1) % 3]];
			V3 n2 = normals[tris[3 * tri + (ei + 2) % 3]];

			V3 pv0, pv1, pv2;
			if (!ppc->Project(v0, pv0))
				continue;
			if (!ppc->Project(v1, pv1))
				continue;
			if (!ppc->Project(v2, pv2))
				continue;
			M33 mat = M33(pv0, pv1, pv2);
			mat[0][2] = 1;
			mat[1][2] = 1;
			mat[2][2] = 1;

			M33 invmat = mat.Inverted();
			V3 z_abc = invmat * V3(pv0[2], pv1[2], pv2[2]);
			M33 norm_abc = invmat * M33(n0, n1, n2);
			fb->DrawTriangleCM(ppc, pv0, pv1, pv2, z_abc, norm_abc, env);
		}
	}

}

V3 TM::GetCenter() {

	V3 ret(0.0f, 0.0f, 0.0f);
	for (int vi = 0; vi < vertsN; vi++)
		ret = ret + verts[vi];
	ret = ret / (float)vertsN;
	return ret;
}

void TM::Rotate(V3 v1, V3 v2, float alpha) {

	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi].rot_arb_axis(v1, v2, alpha);
	}
}


void TM::Translate(V3 tv) {

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] += tv;

}

void TM::SetCenter(V3 newCenter) {

	V3 center = GetCenter();
	V3 tv = newCenter - center;
	Translate(tv);

}

void TM::Scale(float factor) {
	V3 c = GetCenter();
	SetCenter(V3(0.0f, 0.0f, 0.0f));
	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = verts[vi] * factor;
	Translate(c);
}

void TM::Light(float ka, V3 ld) {

	if (!normals) {
		cerr << "INFO: need normals" << endl;
		return;
	}
	for (int vi = 0; vi < vertsN; vi++) {
		cols[vi] = verts[vi].Light(og_cols[vi], ka, 15, ld, normals[vi], V3(0.0f, 0.0f, 0.0f));
	}

}

void TM::Lightpoint(V3 matColor, float ka, float sa, V3 point, PPC* lightppc, float* shmap) {

	if (!normals) {
		cerr << "INFO: need normals" << endl;
		return;
	}

	float epsilon = 0.05;

	for (int vi = 0; vi < vertsN; vi++) {
		int shadowFlag = 0;

		V3 lightcontact;
		if (lightppc->Project(verts[vi], lightcontact)) {
			if ((0 < lightcontact[0]) && (lightcontact[0] < lightppc->w) && (0 < lightcontact[1]) && (lightcontact[1] < lightppc->h)) {
				if (shmap[(lightppc->h - (int)lightcontact[1] - 1) * lightppc->w + (int)lightcontact[0]] - epsilon >= lightcontact[2])
					shadowFlag = 1;
			}
		}
		if (shadowFlag) cols[vi] = og_cols[vi] * ka;
		else {
			V3 lightdir = (verts[vi] - point).unitvector();
			cols[vi] = verts[vi].Light(og_cols[vi], ka, sa, lightdir, normals[vi], point);
		}
		
	}
}

void TM::SetFloor() {
	vertsN = 4;
	verts = new V3[vertsN];
	verts[0] = V3(40.f, -25.f, -100.f);
	verts[1] = V3(175.f, -25.f, -500.f);
	verts[2] = V3(-40.f, -25.f, -100.f);
	verts[3] = V3(-175.f, -25.f, -500.f);

	trisN = 2;
	tris = new unsigned int[trisN * 3];
	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;
	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 3;

	cols = new V3[vertsN];
	cols[0] =
		cols[1] =
		cols[2] =
		cols[3] = V3(0.0f, 0.5f, 0.5f);	
		
	og_cols = new V3[vertsN];
	og_cols[0] =
		og_cols[1] =
		og_cols[2] =
		og_cols[3] = V3(0.0f, 0.5f, 0.5f);

	normals = new V3[vertsN];
	normals[0] =
		normals[1] =
		normals[2] =
		normals[3] = V3(0.0f, 1.0f, 0.0f);
}

void TM::SetOneTriangle(V3 v0, V3 v1, V3 v2) {

	vertsN = 3;
	trisN = 1;
	verts = new V3[vertsN];
	cols = new V3[vertsN];
	normals = new V3[vertsN];

	verts[0] = v0;
	verts[1] = v1;
	verts[2] = v2;
	cols[0] = V3(0.0f, 0.0f, 0.0f);
	cols[1] = cols[0];
	cols[2] = cols[0];
	V3 n = ((v1 - v0) ^ (v2 - v0)).unitvector();
	normals[0] =
		normals[1] =
		normals[2] = n;

	tris = new unsigned int[3 * trisN];
	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;
}


// vertical rectangle in x0y plane, with O as center
void TM::SetTexturedRectangle(float rectw, float recth, float tx, float ty) {

	trisN = 2;
	vertsN = 4;
	verts = new V3[vertsN];
	cols = new V3[vertsN];
	normals = new V3[vertsN];
	tcs = new V3[vertsN];
	tris = new unsigned int [3 * trisN];

	//0(0, 0)       3 (1, 0)
	//	   \
	//		   \
	//1(0, 1)       2 (1, 1)

	verts[0] = V3(rectw / 2.0f, -recth / 2.0f, 0.0f);
	verts[1] = V3(rectw / 2.0f, recth / 2.0f, 0.0f);
	verts[2] = V3(-rectw / 2.0f, -recth / 2.0f, 0.0f);
	verts[3] = V3(-rectw / 2.0f, recth / 2.0f, 0.0f);
	
	int tri = 0;
	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 3;
	tri++;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2; 

	normals[0] =
		normals[1] =
		normals[2] =
		normals[3] = V3(0.f, 0.f, 1.f);

	cols[0] = cols[1] = cols[2] = cols[3] = V3(1.0f, 0.0f, 0.0f);

	// float tc = 0.5f;
	tcs[0] = V3(0.0f, 0.0f, 0.0f);
	tcs[1] = V3(0.0f, ty, 0.0f);
	tcs[2] = V3(tx, 0.0f, 0.0f);
	tcs[3] = V3(tx, ty, 0.0f);

}