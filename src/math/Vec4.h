#ifndef Vec4_H
#define Vec4_H
#include <string>
#include <iostream>     // std::cout, std::ios


class Vec4 {
public:
	float x;
	float y;
	float z;
	float w;

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  	Vec4(const Vec3& v, float w) : x{v.x}, y{v.y}, z{v.z}, w{w} {}
 	explicit Vec4(float v) : x{v}, y{v}, z{v}, w{v} {}

 	float&       operator[](int i) { return (&x)[i]; }
 	const float& operator[](int i) const { return (&x)[i]; }

	std::string tostring() const {
		std::ostringstream stringStream;
		stringStream << x << " " << y << " " << z << " " << w;
		return stringStream.str();		
	}

	float dot(Vec4 b) const {
		return x * b.x + y * b.y + z * b.z + w * b.w;
	}
};

inline Vec4 operator*(const Vec4 & a, const Vec4 & b) {
	return Vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline Vec4 operator+(const Vec4 & a, const Vec4 & b) {
	return Vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + a.w);
}

inline Vec4 operator*(const Vec4 & a, float b) {
	return Vec4(a.x * b, a.y * b, a.z * b, a.w * b);
}

inline Vec4 operator/(const Vec4 & a, float b) {
	return Vec4(a.x / b, a.y / b, a.z / b, a.w / b);
}

#endif