#version 450
#extension GL_ARB_separate_shader_objects : enable


#include "common.h"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 eyeDirection;

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

layout(binding = 1) uniform customData {
	float ambient; //  1  0.588
	float diffuse; //  1  0.588
	float specular; //  1  0
	float shininess; //  1  0

	vec3 specularColor;
} cData;

layout(binding = 2) uniform sampler2D mainTex;
layout(binding = 3) uniform sampler2D specTex;

void main() {
	vec2 uvNor = uv;
	uvNor.y = 1 - uvNor.y;
	vec4 color = texture(mainTex, SampleSphericalMap(eyeDirection));
	color.rgb = pow(color.rgb, vec3(1.0/2.2));
    outColor = vec4(color.rgb, 1.0);
}



