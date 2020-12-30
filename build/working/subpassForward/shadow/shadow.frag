#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
	mat4 vpMatrix;
	mat4 projectionMatrix;
	mat4 worldToCamMatrix;	
	vec4 cameraPos;
	vec4 directionalLightDir;
	vec4 directionalLightColor;
} ubo;


void main() {
	outColor = vec4(1.0, 0.0, 0.0, 1.0);
}



