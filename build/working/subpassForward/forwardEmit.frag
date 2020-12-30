#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec3 worldNormal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;
//layout(location = 6) in mat3 tangentToWorld;
layout(location = 6) in vec4 shadowmapPos;

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
layout(binding = 6) uniform sampler2D BRDFLUTTexture;
layout(binding = 7) uniform sampler2D ShadowmapTexture;
layout(binding = 8) uniform sampler2D emitTex;

#include "common.h"




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
	// NdotH = pow(NdotH, 5);

	return diffuse + lightcolor * NdotH * cData.specularColor.rgb;
	// return diffuse + lightcolor * NdotH;
}



void main() {
	vec2 uvNor = uv;
	uvNor.y = 1 - uvNor.y;
	vec4 albedo = texture(mainTex, uvNor);
	vec3 diffuse = albedo.rgb;
	vec4 color = vec4(0);
	if (albedo.a < 0.01) discard;

	vec3 nData = fromLinear(texture(normalTex, uvNor)).rgb;
	nData = (nData * 2.0 - 1.0);
	//nData = vec3(0.0, 0.0, 1.0);
	vec3 normal = normalize(tangent * nData.x + bitangent * nData.y + worldNormal * nData.z);
	normal = mix(worldNormal, normal, 0.5) ;

	vec4 pbrParam = fromLinear(texture(specTex, uvNor));
	float roughness = pbrParam.g;
	float metallic = pbrParam.b;
	vec3 viewdir = normalize(ubo.cameraPos.xyz - worldPos);

	vec3 lit = basicLit(normal, -ubo.directionalLightDir.xyz, ubo.directionalLightColor.xyz, viewdir, diffuse.rgb);

	//color.rgb += lit;
	// color.rgb += max(0, dot(normal, -ubo.directionalLightDir.xyz));

	//shadow
	vec4 shadowDepth = shadowmapPos / shadowmapPos.w;
	float shadow = GetShadow(ShadowmapTexture, shadowDepth);
	
	BrdfInput brdfData;
	brdfData.albedo = diffuse;
	brdfData.specular = mix(vec3(0.04), diffuse.rgb, metallic);;
	brdfData.metallic = metallic;
	brdfData.roughness = roughness;
	brdfData.normal = normal;
	brdfData.viewdir = viewdir;
	brdfData.lightdir = -ubo.directionalLightDir.xyz;
	brdfData.clearCoat = 0;
	brdfData.clearCoatRoughness = 0;
	brdfData.lightColor = ubo.directionalLightColor.rgb;
	brdfData.shadowMask = shadow;
	color.rgb += brdf(GISkybox, BRDFLUTTexture, brdfData);


	color.rgb *= (1 - shadow * 0.8);
	color.rgb += texture(emitTex, uvNor).rgb;

	color.rgb = pow(color.rgb, vec3(1.0/2.1));
    outColor = vec4(color.rgb, 1.0);
}



