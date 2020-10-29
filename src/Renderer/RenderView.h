#pragma once

#include"math/Vec3.h"
#include<stdlib.h>
#include<stdio.h>
#include<iostream>


struct SampleIntegrator {
	unsigned int r, g, b, a;
};


class RenderView {
public:
	RenderView(int width, int height);

	int GetWidth() { return width; }
	int GetHeight() { return height; }
	Vec3 GetUV(int x, int y) {
		return Vec3( (float)x / width, (float)y / height, 0.0 );
	}

	void SetPixel(int x, int y, int colorInt32);
	void AddPixel(int x, int y, int colorInt32);
	int GetPixel(int x, int y);
	void AddPixelSample(int x, int y, int itNum, float r, float g, float b);

	unsigned int * GetBuffer() { return buffer; }
	void * GetIntegrator() { return sampleIntegrators; }
private:
	int width;
	int height;

	int EncodeInt32(int r, int g, int b, int a);

	void InitBuffer();

	unsigned int * buffer;
	SampleIntegrator * sampleIntegrators;

};