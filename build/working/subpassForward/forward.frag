#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D mainTex;

void main() {
	vec2 uvNor = uv;
	uvNor.y = 1 - uvNor.y;
	vec4 color = texture(mainTex, uvNor);
	color.rgb = pow(color.rgb, vec3(1.0/2.2));
	//color.rgb = vec3(uv, 0.0);
    outColor = vec4(color.rgb, 1.0);
}