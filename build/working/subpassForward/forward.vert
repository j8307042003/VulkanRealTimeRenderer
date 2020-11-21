#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec3 fragNormal;

layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
} ubo;

layout(binding = 1) uniform CustomUBO {
	vec3 custom;
} customUBO;


void main() {
    vec4 outPos = ubo.mvpMatrix * vec4(pos, 1.0);
    gl_Position = outPos;

    fragColor = color.rgb;// * ubo.colorScale.x;
    fragUV = uv;
    worldPos = pos * 0.01;
    fragNormal = normalize(normal);
}