#include "AABB.h"
#include "math/Ray.h"




bool AABB::RayIntersect(const Ray & ray) {
	Vec3 invDir = 1.0f / ray.dir;

	float tmin;
	float tmax;

	if ( invDir.x > 0 ){
		tmin = (min.x - ray.origin.x) * invDir.x;
		tmax = (max.x - ray.origin.x) * invDir.x;
	}
	else {
		tmin = (max.x - ray.origin.x) * invDir.x;
		tmax = (min.x - ray.origin.x) * invDir.x;
	}

	float t0y;
	float t1y;

	if (invDir.y > 0 ){
		t0y = (min.y - ray.origin.y) * invDir.y;
		t1y = (max.y - ray.origin.y) * invDir.y;
	}
	else {
		t1y = (min.y - ray.origin.y) * invDir.y;
		t0y = (max.y - ray.origin.y) * invDir.y;
	}


	if (tmin > t1y || t0y > tmax) return false;

	tmin = tmin > t0y ? tmin : t0y;
	tmax = tmax < t1y ? tmax : t1y;

	float t0z;
	float t1z;


	if (invDir.z > 0 ){
		t0z = (min.z - ray.origin.z) * invDir.z;
		t1z = (max.z - ray.origin.z) * invDir.z;
	}
	else {
		t1z = (min.z - ray.origin.z) * invDir.z;
		t0z = (max.z - ray.origin.z) * invDir.z;
	}

	if (tmin > t1z || tmax < t0z) return false;


	if (t0z > tmin) tmin = t0z;
	if (t1z < tmax) tmax = t1z;


	return true;


}
