#pragma once

#include "V3.h"

using namespace std;


class M33 {
public:
	V3 rows[3];
	M33() {};
	M33(V3 v0, V3 v1, V3 v2);
	M33(char i);
	V3& operator[](int i);
	V3 operator*(V3 v);
	V3 GetColumn(int i);
	void SetColumn(int i, V3 c);
	void SetRotX(float alpha);
	void SetRotY(float alpha);
	void SetRotZ(float alpha);
	M33 Inverted();
	M33 Transposed();
	M33 operator*(M33 m1);
	friend ostream& operator<<(ostream& ostr, M33& m);
	friend istream& operator>>(istream& istr, M33& m);
};