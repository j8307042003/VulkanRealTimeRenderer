#pragma once
#ifndef Vec3_H
#define Vec3_H
#include <string>
#include <math.h>
#include <iostream>     // std::cout, std::ios
#include <sstream>
#include <algorithm>

class Vec3 {

public:
	float x;
	float y;
	float z;

	Vec3(){}
	Vec3(float x, float y, float z);


	float length() const {
		return sqrt(x * x + y * y + z * z);
	}


	void normalize(){
		float len = length();
		x /= len;
		y /= len;
		z /= len;
	}

	Vec3 normalized() const {
		float len = length();
		return Vec3(x/len, y/len, z/len);
	}	

	std::string tostring() const {
		std::ostringstream stringStream;
		stringStream << x << " " << y << " " << z;
		return stringStream.str();		
	}

	static float Angle(Vec3 a, Vec3 b);

	static float Dot(Vec3 a, Vec3 b);
	
	static Vec3 Cross(Vec3 a, Vec3 b);

	static Vec3 Reflect(Vec3 a, Vec3 b);


	// Vec3 operator+(const Vec3& rhs) const ;
	friend Vec3 operator+(const Vec3 & lhs,const Vec3& rhs)
  	{
  		return Vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
  	}
	Vec3 operator+(const float& f) const;
	inline Vec3 operator-(const Vec3& rhs);
  	friend Vec3 operator-(const Vec3& lhs,const Vec3& rhs)
  	{
  		return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
  	}

	Vec3 operator-(const float& f)const;
	Vec3 operator-()const;
	Vec3 operator*(const Vec3& rhs);
	Vec3 operator*(const float& f);
  	friend Vec3 operator*(float lhs,const Vec3& rhs)
  	{
  		return Vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
  	}


	Vec3 operator/(const Vec3& rhs);
	Vec3 operator/(const float& f);
  	friend Vec3 operator/(float lhs,const Vec3& rhs)
  	{
  		return Vec3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
  	}	

	Vec3& operator+=(const Vec3& rhs);
	Vec3& operator-=(const Vec3& rhs);

	float operator[] (int index) const
	{
		switch(index) {
			case 0: return x;
			case 1: return y;
			case 2: return z;
		}

		return 0;
	}
};

inline Vec3 Vec3::operator-(const Vec3 & rhs){
	return {x - rhs.x, y - rhs.y, z - rhs.z};
}

inline Vec3 min(const Vec3 & a, const Vec3 & b){
	return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

inline Vec3 max(const Vec3 & a, const Vec3 & b){
	return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}


#endif

