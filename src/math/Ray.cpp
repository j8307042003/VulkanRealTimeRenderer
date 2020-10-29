#include "Ray.h"
#include <iostream>

Ray::Ray(Vec3 origin, Vec3 dir) : origin(origin), dir(dir) {
	// std::cout << dir.x << "  " << dir.y << std::endl;
}
