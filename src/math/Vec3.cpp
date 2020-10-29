#include "Vec3.h"
#include <math.h>



Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::Angle(Vec3 a, Vec3 b) {
	return acos(Vec3::Dot(a.normalized(), b.normalized()));
}

float Vec3::Dot(Vec3 a, Vec3 b){
  	return a.x * b.x + a.y * b.y + a.z * b.z;
}
	
Vec3 Vec3::Cross(Vec3 a, Vec3 b){
	return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vec3 Vec3::Reflect(Vec3 a, Vec3 b){
	return a - 2 * (Vec3::Dot(a, b)) * b;
}



//Vec3 Vec3::operator+(const Vec3 & rhs) const{
//	Vec3 tmp(x + rhs.x, y + rhs.y, z + rhs.z);
//	return tmp;
//}

Vec3 Vec3::operator+(const float& f) const {
	return Vec3(x + f, y + f, z + f);
}


Vec3 Vec3::operator-() const {
	return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator-(const float& f) const {
	return Vec3(x - f, y - f, z - f);
}

Vec3 Vec3::operator*(const Vec3 & rhs){
	Vec3 tmp(x * rhs.x, y * rhs.y, z * rhs.z);
	return tmp;
}

Vec3 Vec3::operator*(const float& f) {
	return Vec3(x * f, y * f, z * f);
}


Vec3 Vec3::operator/(const Vec3 & rhs){
	Vec3 tmp(x / rhs.x, y / rhs.y, z / rhs.z);
	return tmp;
}

Vec3 Vec3::operator/(const float& f) {
	return {x / f, y / f, z / f};
}


Vec3& Vec3::operator+=(const Vec3& rhs) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}	

Vec3& Vec3::operator-=(const Vec3& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}	