#ifndef Matrix4_H
#define Matrix4_H

#include "Vec4.h"
#include <iostream>

class Matrix4 {
public:
	Vec4 data[4];
	//Vec4 x = {0, 0, 0, 0};
	//Vec4 y = {0, 0, 0, 0};
	//Vec4 z = {0, 0, 0, 0};
	//Vec4 w = {0, 0, 0, 0};

	Matrix4() : data{{0, 0, 0, 0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} {}
	Matrix4(const Vec4 & x, const Vec4 & y, const Vec4 & z, const Vec4 & w) {
		data[0] = {x.x, y.x, z.x, w.x};
		data[1] = {x.y, y.y, z.y, w.y};
		data[2] = {x.z, y.z, z.z, w.z};
		data[3] = {x.w, y.w, z.w, w.w};
	}

	Vec4&       operator[](int i) { return (data[i]); }
	const Vec4& operator[](int i) const { return (data[i]); }
};



inline Vec4 operator*(const Matrix4 & lhs, const Vec4 & v ) {
	// return {lhs.x.dot(v), lhs.y.dot(v), lhs.z.dot(v), lhs.w.dot(v)};
	return {
		Vec4(lhs.data[0].x, lhs.data[1].x, lhs.data[2].x, lhs.data[3].x).dot(v),
		Vec4(lhs.data[0].y, lhs.data[1].y, lhs.data[2].y, lhs.data[3].y).dot(v),
		Vec4(lhs.data[0].z, lhs.data[1].z, lhs.data[2].z, lhs.data[3].z).dot(v),
		Vec4(lhs.data[0].w, lhs.data[1].w, lhs.data[2].w, lhs.data[3].w).dot(v),
	};
}

inline Matrix4 operator*(const Matrix4& a, const Matrix4& b) {
  return {a * b.data[0], a * b.data[1], a * b.data[2], a * b.data[3]};
}

#endif