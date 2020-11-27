#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec3 tangent;
layout (location = 5) in vec3 bitangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 eyeDirection;

layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
	mat4 vpMatrix;
	mat4 projectionMatrix;
	mat4 worldToCamMatrix;
} ubo;

layout(binding = 1) uniform CustomUBO {
	vec3 custom;
} customUBO;

layout(push_constant) uniform PushConsts {
	mat4 modelMatrix;
} pushConsts;

void main() {
    vec4 outPos = ubo.vpMatrix * vec4(pos / 10.0, 1.0);
    outPos = vec4(pos / 1.0, 1.0);
    gl_Position = vec4(pos, 1.0);


    mat4 inverseProjection = inverse(ubo.projectionMatrix);
    mat3 inverseModelview = transpose(mat3(ubo.worldToCamMatrix));
    vec3 unprojected = (inverseProjection * outPos).xyz;
    eyeDirection = normalize(inverseModelview * unprojected);


    fragColor = color.rgb;// * ubo.colorScale.x;
    fragUV = uv;
    worldPos = pos * 0.01;
    fragNormal = normalize(normal);
}