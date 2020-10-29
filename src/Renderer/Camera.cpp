#include "Camera.h"
#include "RenderView.h"
#include "math/Ray.h"
#include<algorithm>
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<stack>
#include<cmath>
#include "glm/gtc/quaternion.hpp"
#include "glm/glm.hpp"


#ifndef M_PI
#define M_PI 3.1415926
#endif

Vec3 normalize(Vec3 v) {
	return v.normalized();
}


const auto vec3Zero = Vec3{0, 0, 0};

int EncodeInt32(int r, int g, int b, int a)
{
	return std::min(255, r) + (std::min(g, 255) << 8) + (std::min(b, 255) << 16) + (std::min(a, 255) << 24);
}

Camera::Camera( int width, int height, Vec3 position) : 
	view(width, height),
	// transform(position, Quaternion(Vec3(0, 1, 0), 30))
	transform(position, glm::quat({0, 0, glm::radians(180.0f)}))
{
	//view = RenderView(width, height);
}

