
#pragma once

#include <ostream>
#include <istream>

using namespace std;

class V3 {
public:
	float xyz[3];
	V3() {};
	V3(float x, float y, float z);
	V3 operator+(V3 v1);
	V3 operator-(V3 v1);
	float& operator[](int i);
	float operator*(V3 v1);
	V3 operator^(V3 v1);
	V3 operator+=(V3& v1);
	V3 operator*(float scalar);
	V3 operator/(float scalar);
	float length();
	V3 normalize();
	V3 rot_arb_axis(V3 v1, V3 v2, float alpha);
	V3 rot_dir(V3 v1, float alpha);
	V3 projection(V3 uvec);
	V3 unitvector();
	friend ostream& operator<<(ostream& ostr, V3& v);
	friend istream& operator>>(istream& istr, V3& v);
	void SetFromColor(unsigned int col);
	unsigned int GetColor();
	V3 Light(V3 matColor, float ka, float sa, V3 ld, V3 norm, V3 source);
};
