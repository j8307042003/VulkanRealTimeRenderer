#pragma once

#include <thread>
#include "Camera.h"

class Renderer {
public:
	Renderer(){};

	void SetRenderData(Camera * cam);
	virtual void StartRender() = 0;
	virtual void UpdateFrame() = 0;
	virtual void ClearImage() = 0;
	virtual void Stall() {running = false;}
	virtual void Resume() {running = true;}
protected:
	Camera * cam;

	bool running = true;
};