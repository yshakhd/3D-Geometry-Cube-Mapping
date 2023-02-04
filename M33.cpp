#define _USE_MATH_DEFINES
#include "M33.h"
#include <cmath>

M33::M33(V3 v0, V3 v1, V3 v2) {
	rows[0] = v0;
	rows[1] = v1;
	rows[2] = v2;
}

M33::M33(char i) {
	if (i == 'i') {
		rows[0] = V3(1.0f, 0.0f, 0.0f);
		rows[1] = V3(0.0f, 1.0f, 0.0f);
		rows[2] = V3(0.0f, 0.0f, 1.0f);
	}
}

V3& M33::operator[](int i) {
	return rows[i];
}

V3 M33::operator*(V3 v) {
	M33& m = *this;
	V3 vecmul;
	vecmul[0] = m[0] * v;
	vecmul[1] = m[1] * v;
	vecmul[2] = m[2] * v;
	return vecmul;
}

V3 M33::GetColumn(int i) {
	return V3(rows[0][i], rows[1][i], rows[2][i]);
}

void M33::SetColumn(int i, V3 c) {
	rows[0][i] = c[0];
	rows[1][i] = c[1];
	rows[2][i] = c[2];
}

void M33::SetRotX(float alpha) {
	float alphaRad = alpha / 180.0f * M_PI;
	float cost = cosf(alphaRad);
	float sint = sinf(alphaRad);
	M33& m = *this;
	m[0] = V3(1.0f, 0.0f, 0.0f);
	m[1] = V3(0.0f, cost, -sint);
	m[2] = V3(0.0f, sint, cost);
}

void M33::SetRotY(float alpha) {
	float alphaRad = alpha / 180.0f * M_PI;
	float cost = cosf(alphaRad);
	float sint = sinf(alphaRad);
	M33& m = *this;
	m[0] = V3(cost, 0.0f, sint);
	m[1] = V3(0.0f, 1.0f, 0.0f);
	m[2] = V3(-sint, 0.0f, cost);
}

void M33::SetRotZ(float alpha) {
	float alphaRad = alpha / 180.0f * M_PI;
	float cost = cosf(alphaRad);
	float sint = sinf(alphaRad);
	M33& m = *this;
	m[0] = V3(cost, -sint, 0.0f);
	m[1] = V3(sint, cost, 0.0f);
	m[2] = V3(0.0f, 0.0f, 1.0f);
}

M33 M33::Transposed() {
	M33 transm;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	transm[0] = a;
	transm[1] = b;
	transm[2] = c;
	return transm;
}

M33 M33::Inverted() {
	M33 invmat;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	V3 _a = b ^ c; _a = _a / (a * _a);
	V3 _b = c ^ a; _b = _b / (b * _b);
	V3 _c = a ^ b; _c = _c / (c * _c);
	invmat[0] = _a;
	invmat[1] = _b;
	invmat[2] = _c;
	return invmat;
}

M33 M33::operator*(M33 m1) {
	M33& m0 = *this;
	M33 matmul;
	matmul[0] = m0 * m1.GetColumn(0);
	matmul[1] = m0 * m1.GetColumn(1);
	matmul[2] = m0 * m1.GetColumn(2);
	return matmul.Transposed();
}

ostream& operator<<(ostream& ostr, M33& m) {
	ostr << m[0] << "\n" << m[1] << "\n" << m[2];
	return ostr;
}

istream& operator>>(istream& istr, M33& m) {
	istr >> m[0] >> m[1] >> m[2];
	return istr;
}
