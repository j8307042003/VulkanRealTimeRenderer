#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
} ubo;

layout(binding = 1) uniform CustomUBO {
	vec3 custom;
} customUBO;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    // gl_Position = vec4(pos, 1.0);
    vec4 outPos = ubo.mvpMatrix * vec4(pos, 1.0);
    // outPos.z = 0;
    gl_Position = outPos;

    fragColor = colors[gl_VertexIndex];// * ubo.colorScale.x;
    fragColor = color.rgb;// * ubo.colorScale.x;
    fragUV = uv;
}