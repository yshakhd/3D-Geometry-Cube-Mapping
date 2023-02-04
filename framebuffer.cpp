#include "framebuffer.h"
#include "math.h"
#include <iostream>
#include "scene.h"
#include "ppc.h"
#include "tm.h"
#include <tiffio.h>
#include "cm.h"

using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) :
	Fl_Gl_Window(u0, v0, _w, _h, 0) {

	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
	zb = new float[w * h];

}


void FrameBuffer::draw() {

	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);

}

int FrameBuffer::handle(int event) {

	switch (event)
	{
	case FL_KEYBOARD: {
		KeyboardHandle();
		return 0;
	}
	case FL_MOVE: {
		int u = Fl::event_x();
		int v = Fl::event_y();
		cerr << u << " " << v << "      \r";
		return 0;
	}
	default:
		return 0;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle() {
	int key = Fl::event_key();
	switch (key) {
	case 'w':
	case 'a':
	case 's':
	case 'd':
	case 'q':
	case 'e':
		cerr << "INFO: pressed " << (char) key << endl;
		(scene->ppc)->Translate(key, 1.0f);
		break;
	case 'u':
	case 'h':
	case 'j':
	case 'k':
	case 'n':
	case 'm':
		cerr << "INFO: pressed " << (char)key << endl;
		(scene->ppc)->Rotate(key, 0.75f);
		break;
	case 'z':
	case 'x':
		cerr << "INFO: pressed " << (char)key << endl;
		(scene->ppc)->Zoom(key, 3.0f);
		break;
	case 'i':
		cerr << "INFO: pressed " << (char)key << endl;
		*(scene->ppc) = (scene->ppc)->Interpolate(PPC(20.0f, 300, 100), 0.2f);
		break;
	default:
		cerr << "INFO: do not understand keypress" << endl;
		return;
	}
	//scene->Render();
}

void FrameBuffer::SetBGR(unsigned int bgr) {

	for (int uv = 0; uv < w * h; uv++)
		pix[uv] = bgr;

}

void FrameBuffer::SetZB(float zf) {

	for (int uv = 0; uv < w * h; uv++)
		zb[uv] = zf;

}


void FrameBuffer::SetChecker(unsigned int col0, unsigned int col1, int csize) {

	for (int v = 0; v < h; v++) {
		for (int u = 0; u < w; u++) {
			int cu = u / csize;
			int cv = v / csize;
			if ((cu + cv) % 2)
				Set(u, v, col0);
			else
				Set(u, v, col1);
		}
	}
}

void FrameBuffer::DrawAARectangle(V3 tlc, V3 brc, unsigned int col) {

	// entire rectangle off screen
	if (tlc[0] > (float)w)
		return;
	if (brc[0] < 0.0f)
		return;
	if (tlc[1] > (float)h)
		return;
	if (brc[1] < 0.0f)
		return;

	// rectangle partially off screen
	if (tlc[0] < 0.0f)
		tlc[0] = 0.0f;
	if (brc[0] > (float)w)
		brc[0] = (float)w;
	if (tlc[1] < 0.0f)
		tlc[1] = 0.0f;
	if (brc[1] > (float)h)
		brc[1] = (float)h;

	int umin = (int)(tlc[0] + 0.5f);
	int umax = (int)(brc[0] - 0.5f);
	int vmin = (int)(tlc[1] + 0.5f);
	int vmax = (int)(brc[1] - 0.5f);
	for (int v = vmin; v <= vmax; v++) {
		for (int u = umin; u <= umax; u++) {
			Set(u, v, col);
		}
	}

}

void FrameBuffer::DrawDisk(V3 center, float r, unsigned int col) {

	int umin = (int)(center[0] - r);
	int umax = (int)(center[0] + r);
	int vmin = (int)(center[1] - r);
	int vmax = (int)(center[1] + r);
	center[2] = 0.0f;
	for (int v = vmin; v <= vmax; v++) {
		for (int u = umin; u <= umax; u++) {
			V3 pixCenter(.5f + (float)u, .5f + (float)v, 0.0f);
			V3 distVector = pixCenter - center;
			if (r * r < distVector * distVector)
				continue;
			SetGuarded(u, v, col);
		}
	}
}


void FrameBuffer::Set(int u, int v, int col) {

	pix[(h - 1 - v) * w + u] = col;

}


void FrameBuffer::SetGuarded(int u, int v, int col) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return;
	Set(u, v, col);

}

void FrameBuffer::DrawSegment(V3 p0, V3 p1, unsigned int col) {

	V3 cv;
	cv.SetFromColor(col);
	DrawSegment(p0, p1, cv, cv);

}

void FrameBuffer::DrawSegment(V3 p0, V3 p1, V3 c0, V3 c1) {

	float maxSpan = (fabsf(p0[0] - p1[0]) < fabsf(p0[1] - p1[1])) ?
		fabsf(p0[1] - p1[1]) : fabsf(p0[0] - p1[0]);
	int segsN = (int)maxSpan + 1;
	V3 currPoint = p0;
	V3 currColor = c0;
	V3 stepv = (p1 - p0) / (float)segsN;
	V3 stepcv = (c1 - c0) / (float)segsN;
	int si;
	for (si = 0;
		si <= segsN;
		si++, currPoint = currPoint + stepv, currColor = currColor + stepcv) {
		int u = (int)currPoint[0];
		int v = (int)currPoint[1];
		if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
			continue;
		if (IsCloserThenSet(currPoint[2], u, v))
			SetGuarded(u, v, currColor.GetColor());
	}
}


// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname) {
	TIFF* in = TIFFOpen(fname, "r");
	if (in == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w * h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cerr << "failed to load " << fname << endl;
	}

	TIFFClose(in);
}

// save as tiff image
void FrameBuffer::SaveAsTiff(char* fname) {

	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}

int FrameBuffer::IsCloserThenSet(float currz, int u, int v) {

	float zbz = zb[(h - 1 - v) * w + u];
	if (zbz > currz)
		return 0;
	zb[(h - 1 - v) * w + u] = currz;
	return 1;

}

void FrameBuffer::DrawTriangleSM1_2(V3 p0, V3 p1, V3 p2, V3 col0, V3 col1, V3 col2, V3 z_abc, M33 col_abc, M33 tc_abc, FrameBuffer* texture) {
	
	int umin = (int)(min(min(p0[0], p1[0]), p2[0]) - 0.5);
	int umax = (int)(max(max(p0[0], p1[0]), p2[0]) + 0.5);
	int vmin = (int)(min(min(p0[1], p1[1]), p2[1]) - 0.5);
	int vmax = (int)(max(max(p0[1], p1[1]), p2[1]) + 0.5);

	if ((umax < 0) || (vmax < 0) || (umin > scene->lightppc->w) || (vmin > scene->lightppc->h))
		return;
	if (umin < 0)
		umin = 0;
	if (vmin < 0)
		vmin = 0;
	if (umax >= scene->lightppc->w)
		umax = scene->lightppc->w - 1;
	if (vmax >= scene->lightppc->h)
		vmax = scene->lightppc->h - 1;

	V3 edge1(p1[1] - p0[1], p0[0] - p1[0], -p0[0] * p1[1] + p0[1] * p1[0]);
	float sidedness = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
	if (sidedness < 0) {
		edge1[0] = -edge1[0];
		edge1[1] = -edge1[1];
		edge1[2] = -edge1[2];
	}

	V3 edge2(p2[1] - p1[1], p1[0] - p2[0], -p1[0] * p2[1] + p1[1] * p2[0]);
	sidedness = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
	if (sidedness < 0) {
		edge2[0] = -edge2[0];
		edge2[1] = -edge2[1];
		edge2[2] = -edge2[2];
	}

	V3 edge3(p2[1] - p0[1], p0[0] - p2[0], -p0[0] * p2[1] + p0[1] * p2[0]);
	sidedness = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];
	if (sidedness < 0) {
		edge3[0] = -edge3[0];
		edge3[1] = -edge3[1];
		edge3[2] = -edge3[2];
	}

	float edge1_p2 = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
	float edge2_p0 = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
	float edge3_p1 = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];

	for (int v = vmin; v <= vmax; v++) {
		for (int u = umin; u <= umax; u++) {
			float edge1_cp = edge1[0] * u + edge1[1] * v + edge1[2];
			float edge2_cp = edge2[0] * u + edge2[1] * v + edge2[2];
			float edge3_cp = edge3[0] * u + edge3[1] * v + edge3[2];

			if ((edge1_p2 * edge1_cp > 0) && (edge2_p0 * edge2_cp > 0) && (edge3_p1 * edge3_cp > 0)) {	
				float z_depth = V3((float) u + .5f, (float) v + .5f, 1) * z_abc;
				if (IsCloserThenSet(z_depth, u, v)) {
					V3 col = col_abc.Transposed() * V3(u, v, 1);
					SetGuarded(u, v, col.GetColor());
					if (texture) {
						V3 txt = tc_abc.Transposed() * V3(u, v, 1);
						// fb->Set(u, v, texture->LookUpNN(currtcs[0], currtcs[1]));
						SetGuarded(u, v, texture->LookUpBilinear(txt[0], txt[1]));
					}
				}
					
			}
		}
	}
}

void FrameBuffer::DrawTriangleShadow(PPC* ppc, V3 p0, V3 p1, V3 p2, V3 col0, V3 col1, V3 col2, V3 z_abc, M33 col_abc, M33 norm_abc, PPC* lightppc, float* shmap, M33 tc_abc, FrameBuffer* texture) {

	int umin = (int)(min(min(p0[0], p1[0]), p2[0]) - 0.5);
	int umax = (int)(max(max(p0[0], p1[0]), p2[0]) + 0.5);
	int vmin = (int)(min(min(p0[1], p1[1]), p2[1]) - 0.5);
	int vmax = (int)(max(max(p0[1], p1[1]), p2[1]) + 0.5);
	
	if ((umax < 0) || (vmax < 0) || (umin >= scene->lightppc->w) || (vmin >= scene->lightppc->h))
		return;
	if (umin < 0)
		umin = 0;
	if (vmin < 0)
		vmin = 0;
	if (umax >= scene->lightppc->w)
		umax = scene->lightppc->w - 1;
	if (vmax >= scene->lightppc->h)
		vmax = scene->lightppc->h - 1;

	V3 edge1(p1[1] - p0[1], p0[0] - p1[0], -p0[0] * p1[1] + p0[1] * p1[0]);
	float sidedness = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
	if (sidedness < 0) {
		edge1[0] = -edge1[0];
		edge1[1] = -edge1[1];
		edge1[2] = -edge1[2];
	}

	V3 edge2(p2[1] - p1[1], p1[0] - p2[0], -p1[0] * p2[1] + p1[1] * p2[0]);
	sidedness = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
	if (sidedness < 0) {
		edge2[0] = -edge2[0];
		edge2[1] = -edge2[1];
		edge2[2] = -edge2[2];
	}

	V3 edge3(p2[1] - p0[1], p0[0] - p2[0], -p0[0] * p2[1] + p0[1] * p2[0]);
	sidedness = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];
	if (sidedness < 0) {
		edge3[0] = -edge3[0];
		edge3[1] = -edge3[1];
		edge3[2] = -edge3[2];
	}

	float edge1_p2 = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
	float edge2_p0 = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
	float edge3_p1 = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];

	for (int v = vmin; v <= vmax; v++) {
		for (int u = umin; u <= umax; u++) {
			float edge1_cp = edge1[0] * u + edge1[1] * v + edge1[2];
			float edge2_cp = edge2[0] * u + edge2[1] * v + edge2[2];
			float edge3_cp = edge3[0] * u + edge3[1] * v + edge3[2];

			if ((edge1_p2 * edge1_cp > 0) && (edge2_p0 * edge2_cp > 0) && (edge3_p1 * edge3_cp > 0)) {
				float z_depth = V3(u, v, 1) * z_abc;
				if (IsCloserThenSet(z_depth, u, v)) {
					V3 norm = norm_abc.Transposed() * V3(u, v, 1);
					V3 point;
					ppc->Unproject(V3(u, v, z_depth), point);

					V3 col = col_abc.Transposed() * V3(u, v, 1);

					// determine whether pixel is in shadow
					int shadowFlag = 0;
					float epsilon = 0.05;
					V3 lightcontact;
					if (lightppc->Project(point, lightcontact)) {
						if ((0 < lightcontact[0]) && (lightcontact[0] < lightppc->w) && (0 < lightcontact[1]) && (lightcontact[1] < lightppc->h)) {
							if (shmap[(lightppc->h - (int)lightcontact[1] - 1) * lightppc->w + (int)lightcontact[0]] - epsilon >= lightcontact[2])
								shadowFlag = 1;
						}
					}

					if (shadowFlag) SetGuarded(u, v, (col * scene->ka).GetColor());
					else {
						V3 new_col = point.Light(col, scene->ka, floor(scene->sa), (point - lightppc->C).unitvector(), norm, ppc->C);
						SetGuarded(u, v, new_col.GetColor());
					}

					if (texture) {
						V3 txt = tc_abc.Transposed() * V3(u, v, 1);
						// fb->Set(u, v, texture->LookUpNN(currtcs[0], currtcs[1]));
						SetGuarded(u, v, texture->LookUpBilinear(txt[0], txt[1]));
					}
				}

			}
		}
	}
}

void FrameBuffer::DrawTriangleCM(PPC* ppc, V3 p0, V3 p1, V3 p2, V3 z_abc, M33 norm_abc, CubeMap* cm) {

	int umin = (int)(min(min(p0[0], p1[0]), p2[0]) - 0.5);
	int umax = (int)(max(max(p0[0], p1[0]), p2[0]) + 0.5);
	int vmin = (int)(min(min(p0[1], p1[1]), p2[1]) - 0.5);
	int vmax = (int)(max(max(p0[1], p1[1]), p2[1]) + 0.5);

	if ((umax < 0) || (vmax < 0) || (umin > ppc->w) || (vmin > ppc->h))
		return;
	if (umin < 0)
		umin = 0;
	if (vmin < 0)
		vmin = 0;
	if (umax >= ppc->w)
		umax = ppc->w - 1;
	if (vmax >= ppc->h)
		vmax = ppc->h - 1;

	V3 edge1(p1[1] - p0[1], p0[0] - p1[0], -p0[0] * p1[1] + p0[1] * p1[0]);
	float sidedness = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
	if (sidedness < 0) {
		edge1[0] = -edge1[0];
		edge1[1] = -edge1[1];
		edge1[2] = -edge1[2];
	}

	V3 edge2(p2[1] - p1[1], p1[0] - p2[0], -p1[0] * p2[1] + p1[1] * p2[0]);
	sidedness = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
	if (sidedness < 0) {
		edge2[0] = -edge2[0];
		edge2[1] = -edge2[1];
		edge2[2] = -edge2[2];
	}

	V3 edge3(p2[1] - p0[1], p0[0] - p2[0], -p0[0] * p2[1] + p0[1] * p2[0]);
	sidedness = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];
	if (sidedness < 0) {
		edge3[0] = -edge3[0];
		edge3[1] = -edge3[1];
		edge3[2] = -edge3[2];
	}

	float edge1_p2 = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
	float edge2_p0 = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
	float edge3_p1 = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];

	for (int v = vmin; v <= vmax; v++) {
		for (int u = umin; u <= umax; u++) {
			float edge1_cp = edge1[0] * u + edge1[1] * v + edge1[2];
			float edge2_cp = edge2[0] * u + edge2[1] * v + edge2[2];
			float edge3_cp = edge3[0] * u + edge3[1] * v + edge3[2];

			if ((edge1_p2 * edge1_cp > 0) && (edge2_p0 * edge2_cp > 0) && (edge3_p1 * edge3_cp > 0)) {
				float z_depth = V3((float)u + .5f, (float)v + .5f, 1) * z_abc;
				if (IsCloserThenSet(z_depth, u, v)) {
					V3 norm = norm_abc.Transposed() * V3(u, v, 1);
					V3 point;
					ppc->Unproject(V3(u, v, z_depth), point);

					V3 eye = point - ppc->C;
					V3 dir = eye - (norm * (eye * norm)) * 2;
					SetGuarded(u, v, cm->LookUpColor(dir));
					
				}

			}
		}
	}
}

void FrameBuffer::Render3DCircle(PPC* ppc, V3 v0, V3 col, float r) {
	V3 pv0;
	if (!ppc->Project(v0, pv0)) return;
	DrawDisk(pv0, r, col.GetColor());
}

void FrameBuffer::Render3DSegment(PPC* ppc, V3 v0, V3 v1, V3 c0, V3 c1) {
	V3 pv0, pv1;
	if (!ppc->Project(v0, pv0))
		return;
	if (!ppc->Project(v1, pv1))
		return;
	DrawSegment(pv0, pv1, c0, c1);
}

void FrameBuffer::RenderCamera(PPC* ppc, PPC* ppc3) {
	V3 tlc = ppc->C + ppc->c;
	V3 trc = tlc + (ppc->a * ppc->w);
	V3 brc = trc + (ppc->b * ppc->h);
	V3 blc = brc - (ppc->a * ppc->w);
	
	V3 blue = V3(0.0f, 0.0f, 1.0f);
	V3 green = V3(0.0f, 1.0f, 0.0f);
	V3 red = V3(1.0f, 0.0f, 0.0f);

	Render3DSegment(ppc3, tlc, trc, green, red);
	Render3DSegment(ppc3, tlc, blc, red, green);
	Render3DSegment(ppc3, brc, trc, green, red);
	Render3DSegment(ppc3, brc, blc, red, green);
	Render3DSegment(ppc3, ppc->C, trc, blue, green);
	Render3DSegment(ppc3, ppc->C, blc, blue, green);
	Render3DSegment(ppc3, ppc->C, tlc, blue, green);
	Render3DSegment(ppc3, ppc->C, brc, blue, green);

	Render3DCircle(ppc3, ppc->C, blue, 5.f);
}

void FrameBuffer::RenderCM(PPC* ppc, CubeMap* cm) {
	for (int u = 0; u < w; u++)
		for (int v = 0; v < h; v++)
			if (!(zb[(h - 1 - v) * w + u]))
				SetGuarded(u, v, cm->LookUpColor(ppc->c + ppc->a * u + ppc->b * v));
}

unsigned int FrameBuffer::LookUpNN(float s, float t) {

	if (s < 0.0f || t < 0.0f)
		return 0xFFFFFF00;

	float sf = s - (float)((int)s);
	float tf = t - (float)((int)t);

	int u = (int)(sf * (float)w);
	int v = (int)(tf * (float)h);
	return Get(u, v);

}


unsigned int FrameBuffer::LookUpBilinear(float s, float t) {

	if (s <= 0.0f || t <= 0.0f)
		return 0xFFFFFF00;

	float sf = s - (float)((int)s);
	float tf = t - (float)((int)t);

	float uf = sf * (float)w;
	float vf = tf * (float)h;
	if (uf < 0.5f || vf < 0.5f || uf > -.5f + (float)w || vf > -.5f + (float)h)
		return LookUpNN(s, t);

	int uA = (int)(uf - 0.5f);
	int vA = (int)(vf - 0.5f);
	float x = uf - (.5f + (float)uA);
	float y = vf - (.5f + (float)vA);

	V3 A; A.SetFromColor(Get(uA, vA));
	V3 B; B.SetFromColor(Get(uA, vA + 1));
	V3 C; C.SetFromColor(Get(uA + 1, vA + 1));
	V3 D; D.SetFromColor(Get(uA + 1, vA));
	V3 bic = A * (1 - x) * (1 - y) + B * (1 - x) * y + C * x * y + D * x * (1 - y);

	return bic.GetColor();
}

unsigned int FrameBuffer::Get(int u, int v) {

	return pix[(h - 1 - v) * w + u];

}