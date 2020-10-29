#pragma once

#include "math/Quaternion.h"
#include "math/Transform.h"
#include "math/Vec3.h"
#include "RenderView.h"
#include "glm/ext/matrix_float4x4.hpp"

class Camera {
public:
	Camera() : view(0, 0){}
	Camera( int width, int height, Vec3 position);

	Transform transform;

	int GetWidth() {return view.GetWidth();}
	int GetHeight() {return view.GetHeight();}
	void * GetIntegrator() {return view.GetIntegrator();}
private:
	RenderView view;
};