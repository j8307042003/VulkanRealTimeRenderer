#pragma once
#ifndef AABB_H
#define AABB_H

#include "math/Ray.h"
#include "math/Vec3.h"
#include <limits>

class AABB {
public:
	Vec3 min;
	float pad;
	Vec3 max;
	float pad1;

	AABB() {
        float minNum = std::numeric_limits<float>::lowest();
        float maxNum = std::numeric_limits<float>::max();
        min = Vec3(maxNum, maxNum, maxNum);
        max = Vec3(minNum, minNum, minNum);		
	}
	AABB(Vec3 min, Vec3 max) : min(min), max(max) {}

	Vec3 Diagonal() const {
		return max - min;
	}

	int MaxExtent() const {
		auto d = Diagonal();
		if(d.x > d.y && d.x > d.z) return 0;
		else if (d.y > d.z) return 1;
		else return 2;
	}

	bool RayIntersect(const Ray & ray);
	Vec3 Center() {return (max + min) / 2;}
	Vec3 Offset(Vec3 pos) const {
		Vec3 o = pos - min;
		if (max.x > min.x) o.x /= max.x - min.x;
		if (max.y > min.y) o.y /= max.y - min.y;
		if (max.z > min.z) o.z /= max.z - min.z;
		return o;
	}

	float SurfaceArea() const {
		auto size = Diagonal();
		if (size.x < 0 || size.y < 0 || size.z < 0) return 0;
		return 2 * ( size.x * size.y + size.x * size.z + size.y * size.z );
	}

};

static AABB aabbZero({0,0,0}, {0,0,0});


inline AABB merge_aabb(const AABB & a, const AABB & b){
	return AABB(min(a.min, b.min), max(a.max, b.max));	
}

inline Vec3 center(const AABB & a){
	return (a.min + a.max) / 2;
}

/*
inline AABB make_aabb(const Sphere & s) {
	return AABB(s.position - s.radius, s.position + s.radius);
}


inline AABB make_aabb(const Triangle & t) {
	return AABB(
		{
			std::min(std::min(t.Vertices[0].x, t.Vertices[1].x), t.Vertices[2].x),
			std::min(std::min(t.Vertices[0].y, t.Vertices[1].y), t.Vertices[2].y),
			std::min(std::min(t.Vertices[0].z, t.Vertices[1].z), t.Vertices[2].z)
		},

		{
			std::max(std::max(t.Vertices[0].x, t.Vertices[1].x), t.Vertices[2].x),
			std::max(std::max(t.Vertices[0].y, t.Vertices[1].y), t.Vertices[2].y),
			std::max(std::max(t.Vertices[0].z, t.Vertices[1].z), t.Vertices[2].z)
		}		

	);
}

inline AABB make_aabb(const Plane & p) {
	return AABB(
		{
			std::min(std::min(std::min(p.Vertices[0].x, p.Vertices[1].x), p.Vertices[2].x), p.Vertices[3].x),
			std::min(std::min(std::min(p.Vertices[0].y, p.Vertices[1].y), p.Vertices[2].y), p.Vertices[3].y),
			std::min(std::min(std::min(p.Vertices[0].z, p.Vertices[1].z), p.Vertices[2].z), p.Vertices[3].z)
		},

		{
			std::max(std::max(std::max(p.Vertices[0].x, p.Vertices[1].x), p.Vertices[2].x), p.Vertices[3].x),
			std::max(std::max(std::max(p.Vertices[0].y, p.Vertices[1].y), p.Vertices[2].y), p.Vertices[3].y),
			std::max(std::max(std::max(p.Vertices[0].z, p.Vertices[1].z), p.Vertices[2].z), p.Vertices[3].z)
		}		
	);
}
*/
#endif

