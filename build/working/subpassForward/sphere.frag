#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common.h"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec3 worldNormal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;
layout(location = 6) in mat3 tangentToWorld;

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
layout(binding = 4) uniform sampler2D normalTex;
layout(binding = 5) uniform sampler2D GISkybox;

vec3 basicLit(vec3 normal, vec3 lightdir, vec3 lightcolor, vec3 viewdir, vec3 color)
{
	float NdotL = clamp(dot(normal, lightdir), 0 , 1);
	vec3 diffuse = clamp(NdotL + 0.1, 0, 1) * color;

	vec3 l = normalize(lightdir);
	vec3 v = normalize(viewdir);
	vec3 h = normalize(l + v);

	//Intensity of the specular light
	float NdotH = clamp(dot(normal, h), 0, 1);
	NdotH = pow(NdotH, cData.shininess);

	return diffuse + lightcolor * NdotH * cData.specularColor.rgb;
}

vec4 sampleGI(vec2 uv, float roughness)
{
	float lod = roughness * 13;
	float min = floor(lod);
	float max = ceil(lod);
	return mix(textureLod(GISkybox, uv, min), textureLod(GISkybox, uv, max), (max - min));
}



void main() {
	vec2 uvNor = uv;
	uvNor.y = 1 - uvNor.y;
	vec3 viewdir = normalize(ubo.cameraPos.xyz - worldPos);

	vec4 color = vec4(1, 1, 1, 1);
	color.rgb = vec3(0.955, 0.637, 0.538);

	vec3 reflectdir = reflect(-viewdir, normalize(worldNormal));
	//vec4 gicolor = texture(GISkybox, SampleSphericalMap(reflectdir));
	vec2 uv = SampleSphericalMap(reflectdir);
	uv.x = 1 - uv.x;
	vec4 gicolor = sampleGI(uv, 0.2);
	float metallic = 1.0;
	//color.rgb = mix(vec3(1), color.rgb, 1.0) * gicolor.rgb * 1.0;
	color.rgb = mix(vec3(1), color.rgb, metallic) * gicolor.rgb * abs(dot(viewdir, worldNormal));


	color.rgb = pow(color.rgb, vec3(1.0/2.2));
    outColor = vec4(color.rgb, 1.0);
}



