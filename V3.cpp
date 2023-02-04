#define _USE_MATH_DEFINES
#include "V3.h"
#include "M33.h"
#include <cmath>

V3::V3(float x, float y, float z) {
	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
}

V3 V3::operator+(V3 v1) {
	V3 sum;
	V3& v0 = *this;
	sum[0] = v0[0] + v1[0];
	sum[1] = v0[1] + v1[1];
	sum[2] = v0[2] + v1[2];
	return sum;
}

V3 V3::operator+=(V3& v1) {
	V3 sum;
	V3& v0 = *this;
	v0[0] += v1[0];
	v0[1] += v1[1];
	v0[2] += v1[2];
	return sum;
}

V3 V3::operator-(V3 v1) {
	V3 diff;
	V3& v0 = *this;
	diff[0] = v0[0] - v1[0];
	diff[1] = v0[1] - v1[1];
	diff[2] = v0[2] - v1[2];
	return diff;
}

float& V3::operator[](int i) {
	return xyz[i];
}

float V3::operator*(V3 v1) {
	V3& v0 = *this;
	return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

ostream& operator<<(ostream& ostr, V3& v) {
	ostr << v[0] << " " << v[1] << " " << v[2];
	return ostr;
}

istream& operator>>(istream& istr, V3& v) {
	istr >> v[0] >> v[1] >> v[2];
	return istr;
}

V3 V3::operator^(V3 v1) {
	V3 cross;
	V3& v0 = *this;
	cross[0] = v0[1] * v1[2] - (v0[2] * v1[1]);
	cross[1] = -1 * (v0[0] * v1[2] - (v0[2] * v1[0]));
	cross[2] = v0[0] * v1[1] - (v0[1] * v1[0]);
	return cross;
}

V3 V3::operator*(float scalar) {
	V3 prod;
	V3& v0 = *this;
	prod[0] = v0[0] * scalar;
	prod[1] = v0[1] * scalar;
	prod[2] = v0[2] * scalar;
	return prod;
}

V3 V3::operator/(float scalar) {
	V3 prod;
	if (!scalar) return prod;
	V3& v0 = *this;
	prod = v0 * (1 / scalar);
	return prod;
}

float V3::length() {
	V3& v = *this;
	float dot = v * v;
	return sqrt(dot);
}

V3 V3::normalize() {
	V3& v = *this;
	return v / v.length();
}

V3 V3::unitvector() {
	V3 ret = *this;
	return ret / ret.length();
}

V3 V3::projection(V3 uvec) {
	V3& v0 = *this;
	return uvec * ((v0 * uvec) / v0.length());
}

V3 V3::rot_arb_axis(V3 v1, V3 v2, float alpha) {
	M33 laxes;
	laxes[1] = v2;
	V3 aux(1.0f, 0.0f, 0.0f);
	if (fabsf(v2[0]) > fabsf(v2[1]))
		aux = V3(0.0f, 1.0f, 0.0f);
	laxes[2] = aux ^ v2;
	laxes[0] = laxes[1] ^ laxes[2];

	V3& P = *this;
	V3 Pl = laxes * (P - v1);
	M33 roty; roty.SetRotY(alpha);
	V3 Pr = roty * Pl;
	return laxes.Inverted() * Pr + v1;
}

V3 V3::rot_dir(V3 v1, float alpha) {
	return rot_arb_axis(V3(0.0f, 0.0f, 0.0f), v1, alpha);
}

void V3::SetFromColor(unsigned int col) {

	V3& v = *this;
	v[0] = (float)(((unsigned char*)(&col))[0]) / 255.0f;
	v[1] = (float)(((unsigned char*)(&col))[1]) / 255.0f;
	v[2] = (float)(((unsigned char*)(&col))[2]) / 255.0f;

}

unsigned int V3::GetColor() {

	V3 v = *this;
	v[0] = (v[0] < 0.0f) ? 0.0f : v[0];
	v[1] = (v[1] < 0.0f) ? 0.0f : v[1];
	v[2] = (v[2] < 0.0f) ? 0.0f : v[2];

	v[0] = (v[0] > 1.0f) ? 1.0f : v[0];
	v[1] = (v[1] > 1.0f) ? 1.0f : v[1];
	v[2] = (v[2] > 1.0f) ? 1.0f : v[2];

	unsigned int ret;
	int rgb[3]{};
	rgb[0] = (int)(v[0] * 255.0f);
	rgb[1] = (int)(v[1] * 255.0f);
	rgb[2] = (int)(v[2] * 255.0f);
	ret = 0xFF000000 + 256 * 256 * rgb[2] + 256 * rgb[1] + rgb[0];
	return ret;
}

V3 V3::Light(V3 matColor, float ka, float sa, V3 ld, V3 norm, V3 source) {
	V3 ref = ld - (norm * 2 * (ld * norm));
	V3 ref_lvec = ref.normalize();
	V3 eye_vec = (source - *this).normalize();

	V3 ret;
	float kd = (ld * -1.0f) * norm;
	kd = (kd < 0.0f) ? 0.0f : kd;
	float ks = pow(ref_lvec * eye_vec, sa);
	ks = (ks < 0.0f) ? 0.0f : ks;
	ret = matColor * (ka + (1.0f - ka) * kd + ks);
	return ret;
}