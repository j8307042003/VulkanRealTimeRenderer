#include "Quaternion.h"
#include "Matrix4.h"
// #include "glm\glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"


glm::mat4 Quaternion::GetMatrix() {
	glm::quat q = {x, y, z, w};
	glm::mat4 RotationMatrix = glm::toMat4(q);
	return RotationMatrix;	
}
