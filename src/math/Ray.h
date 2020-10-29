#pragma once
#ifndef Ray_H
#define Ray_H


#include "Vec3.h"

class Ray {
public:
	Ray(){}
	Ray(Vec3 origin, Vec3 dir);

	Vec3 origin;
	Vec3 dir;

	
};

#endif