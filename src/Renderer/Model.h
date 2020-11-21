#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "VulkanLib/vkHelper.h"
#include <vector>


#ifndef MODEL_HEADER
#define MODEL_HEADER

class Model {
public:
	struct ModelVertexData {
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec4 color;
		glm::vec3 normal;
	};

	std::vector<ModelVertexData> data;
	std::vector<uint32_t> indices;
	void SetData(std::vector<ModelVertexData> data, std::vector<uint32_t> indices);
	void LoadModel(const char * filename);
};
#endif // !MODEL_HEADER
