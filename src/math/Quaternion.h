#pragma once
#ifndef Quaternion_H
#define Quaternion_H

#include "Vec3.h"
#include "Matrix4.h"
// #include "glm\glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"




class Quaternion {
public:
	float x;
	float y;
	float z;
	float w;

	Quaternion() : x(0), y(0), z(0), w(1) {}

	Quaternion( Vec3 axis, float angle ) {
	// assume axis vector is normalized
		float s = sin(0.5f * angle);

		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
		w = 0.5f * angle;
	}

	Quaternion( float x, float y, float z, float w ) : x(x), y(y), z(z), w(w) {}

	Quaternion( double yaw, double pitch, double roll ) {
    	double cy = cos(yaw * 0.5);
    	double sy = sin(yaw * 0.5);
    	double cp = cos(pitch * 0.5);
    	double sp = sin(pitch * 0.5);
    	double cr = cos(roll * 0.5);
    	double sr = sin(roll * 0.5);

    	w = cy * cp * cr + sy * sp * sr;
    	x = cy * cp * sr - sy * sp * cr;
    	y = sy * cp * sr + cy * sp * cr;
    	z = sy * cp * cr - cy * sp * sr;
	}

	glm::mat4 GetMatrix();


	Quaternion operator*(const Quaternion& rhs) {
  		Quaternion q;
   
		const Vec3 vThis(x, y, z);
		const Vec3 rhsV3(rhs.x, rhs.y, rhs.z);
		q.w = w * rhs.w - Vec3::Dot(vThis, rhsV3);
		 
		const Vec3 newV = 
		    rhs.w * vThis + w * rhsV3 + Vec3::Cross(vThis, rhsV3);
		q.x = newV.x;
		q.y = newV.y;
		q.z = newV.z;
		
		q = q.Normalized();
		return q;
	}

	Quaternion Normalized()
	{
		Quaternion q1 = {0.0, 0.0, 0.0, 0.0};
		float len_inv = 1.0 / sqrt(w * w + x * x + y * y + z * z);
		q1.w = w * len_inv;
		q1.x = x * len_inv;
		q1.y = y * len_inv;
		q1.z = z * len_inv;
		return q1;
	}

};

#endif