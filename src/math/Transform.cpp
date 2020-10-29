#include "Transform.h"
// #include "Matrix4.h"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/quaternion.hpp>


Vec3 Transform::TransformPoint(Vec3 point) {
	// Vec4 t = modelMatrix * Vec4(point, 1);
	// t = t / t.w;
	// return {t.x, t.y, t.z};
	return {};
}

Vec3 Transform::TransformDir(Vec3 dir) {
	// Matrix4 rotMat = rotation.GetMatrix();
	// Vec4 t = rotMat * Vec4(dir, 1);
	// std::cout << t.x << " " << t.y << " " << t.z << " " << t.w << std::endl;
	// t = t / t.w;
	// Vec3 v3dir = {t.x, t.y, t.z};
	// return v3dir.normalized();

	glm::vec4 result = modelMatrix * glm::vec4(dir.x, dir.y, dir.z, 0);
	return {result.x, result.y, result.z};
}



void Transform::UpdateMatrix() {
	// glm::mat4 rotMat = rotation.GetMatrix();
	glm::mat4 rotMat = glm::toMat4(rotation);
	//Matrix4 translateMat 
	//	= Matrix4( 
	//	{1, 0, 0, position.x},
	//	{0, 1, 0, position.y},
	//	{0, 0, 1, position.z},
	//	{0, 0, 0, 1}
	//	 );

	modelMatrix = glm::mat4(1.0);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, position.z));
	modelMatrix = modelMatrix * rotMat;
	// modelMatrix = translateMat;
	// modelMatrix = rotMat;
	//std::cout << modelMatrix.data[0].tostring() << std::endl
	//		  << modelMatrix.data[1].tostring() << std::endl
	//		  << modelMatrix.data[2].tostring() << std::endl
	//		  << modelMatrix.data[3].tostring() << std::endl;
}
