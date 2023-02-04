#include "ppc.h"
#include "m33.h"
#include <FL/Fl.H>
#include <fstream>

using namespace std;

PPC::PPC(float hfov, int _w, int _h) : w(_w), h(_h) {

	C = V3(0.0f, 0.0f, 0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	float hfovd = hfov / 180.0f * 3.1415926f;
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, -(float)w / (2.0f * tan(hfovd / 2.0f)));

}

PPC::PPC(V3 _C, V3 _a, V3 _b, V3 _c) {
	C = _C;
	a = _a.unitvector();
	b = _b.unitvector();
	c = _c;
	w = w;
	h = h;
}

int PPC::Project(V3 P, V3 &pP) {

	M33 M;
	M.SetColumn(0, a);
	M.SetColumn(1, b);
	M.SetColumn(2, c);
	V3 q = M.Inverted()*(P - C);
	if (q[2] <= 0.0f)
		return 0;

	pP[0] = q[0] / q[2];
	pP[1] = q[1] / q[2];
	pP[2] = 1.0f / q[2];
	return 1;

}

V3 PPC::GetVD() {

	return (a^b).unitvector();

}

void PPC::Translate(char key, float offset) {
	V3 eye;
	switch (key) {
	case 'a':
		eye = a * -offset;
		break;
	case 'd':
		eye = a * offset;
		break;
	case 'w':
		eye = b * -offset;
		break;
	case 's':
		eye = b * offset;
		break;
	case 'q':
		eye = (b ^ a) * -offset;
		break;
	case 'e':
		eye = (b ^ a) * offset;
		break;
	default:
		break;
	}
	C += eye;
}

void PPC::Rotate(char key, float theta) {

	switch (key) {
	case 'u':
		c = c.rot_dir(a, theta);
		b = b.rot_dir(a, theta);
		break;
	case 'j':
		c = c.rot_dir(a, -theta);
		b = b.rot_dir(a, -theta);
		break;
	case 'h':
		c = c.rot_dir(b, -theta);
		a = a.rot_dir(b, -theta);
		break;
	case 'k':
		c = c.rot_dir(b, theta);
		a = a.rot_dir(b, theta);
		break;
	case 'n':
		a = a.rot_dir(a ^ b, theta);
		b = b.rot_dir(a ^ b, theta);
		c = c.rot_dir(a ^ b, theta);
		break;
	case 'm':
		a = a.rot_dir(b ^ a, theta);
		b = b.rot_dir(b ^ a, theta);
		c = c.rot_dir(b ^ a, theta);
		break;
	default:
		break;
	}
}

void PPC::Zoom(char key, float dist) {
	V3 move;
	switch (key) {
	case 'z':
		move = (a ^ b) * dist;
		break;
	case 'x':
		move = (b ^ a) * dist;
		break;
	default:
		break;
	}
	c += move;
}

PPC PPC::Interpolate(PPC v1, float frac) {
	PPC& v0 = *this;
	V3 Ci = v0.C + (v1.C - v0.C) * frac;
	V3 vd0 = GetVD();
	V3 vd1 = v1.GetVD();
	V3 vdi = vd0 + (vd1 - vd0) * frac;
	V3 ai = v0.a + (v1.a - v0.a) * frac;
	V3 L = Ci + vdi;
	V3 upg = (ai ^ vdi).unitvector();
	V3 bi = v0.b + (v1.b - v0.b) * frac;
	V3 ci = v0.c + (v1.c - v0.c) * frac;
	PPC vi = PPC(Ci, ai, bi, ci);
	vi.w = w;
	vi.h = h;
	vi.PositionAndOrient(Ci, L, upg);
	return vi;
}

float PPC::GetF() {
	return c*GetVD();
}

void PPC::PositionAndOrient(V3 Cn, V3 L, V3 upg) {

	V3 vdn = (L - Cn).unitvector();
	V3 an = (vdn ^ upg).unitvector();
	V3 bn = vdn ^ an;
	float f = GetF();
	V3 cn = vdn*f - an* (float)w / 2.0f - bn* (float)h / 2.0f;

	a = an;
	b = bn;
	c = cn;
	C = Cn;

}

void PPC::SavePPC(char* filename) {
	fstream fptr;
	fptr.open(filename, ios::out | ios::app);
	fptr << w << "\n" << h << "\n" << C << "\n" << a << "\n" << b << "\n" << c << "\n";
	fptr.close();
}

void PPC::LoadPPC(char* filename) {
	fstream fptr;
	fptr.open(filename, ios::in);
	fptr >> w >> h >> C[0] >> C[1] >> C[2] >> a[0] >> a[1] >> a[2] >> b[0] >> b[1] >> b[2] >> c[0] >> c[1] >> c[2];
	fptr.close();
}

void PPC::Unproject(V3 pP, V3& P) {
	P = C + (a * pP[0] + b * pP[1] + c) / pP[2];
}