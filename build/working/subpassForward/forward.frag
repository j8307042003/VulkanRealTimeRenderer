#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
	vec4 cameraPos;
	vec4 directionalLightDir;
	vec4 directionalLightColor;
} ubo;

layout(binding = 2) uniform sampler2D mainTex;

vec3 basicLit(vec3 normal, vec3 lightdir, vec3 lightcolor, vec3 viewdir, vec3 color)
{
	float NdotL = clamp(dot(normal, lightdir), 0 , 1);
	vec3 diffuse = clamp(NdotL + 0.1, 0, 1) * color;

	vec3 l = normalize(lightdir);
	vec3 v = normalize(viewdir);
	vec3 h = normalize(l + v);

	//Intensity of the specular light
	float NdotH = clamp(dot(normal, h), 0, 1);
	NdotH = pow(NdotH, 15);

	return diffuse + lightcolor * NdotH;
}


void main() {
	vec2 uvNor = uv;
	uvNor.y = 1 - uvNor.y;
	vec4 color = texture(mainTex, uvNor);
	//color.rgb = color.rgb * ubo.directionalLightColor.rgb;
	//color.rgb = worldPos * 0.01;
	//color.rgb = normal;

	vec3 viewdir = normalize(ubo.cameraPos.xyz - worldPos);
	vec3 lit = basicLit(normal, -ubo.directionalLightDir.xyz, ubo.directionalLightColor.xyz, viewdir, color.rgb);
	color.rgb = lit;
	color.rgb = pow(color.rgb, vec3(1.0/2.2));
	//color.rgb = viewdir;
    outColor = vec4(color.rgb, 1.0);
}



