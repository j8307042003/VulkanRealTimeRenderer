#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec3 tangent;
layout (location = 5) in vec3 bitangent;

layout(location = 0) out vec3 fragColor;


layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
	mat4 vpMatrix;
	mat4 projectionMatrix;
	mat4 worldToCamMatrix;		
} ubo;


layout(push_constant) uniform PushConsts {
	mat4 modelMatrix;
} pushConsts;





void main() {
    vec4 outPos = ubo.mvpMatrix * vec4(pos, 1.0);
    outPos = ubo.vpMatrix * pushConsts.modelMatrix * vec4(pos, 1.0);
    //outPos.z *= -1;
    //outPos.z = 1 - outPos.z;
    gl_Position = outPos;
    fragColor = color.rgb;// * ubo.colorScale.x;
}