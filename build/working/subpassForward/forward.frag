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
layout(binding = 6) uniform sampler2D BRDFLUTTex;

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


vec4 sampleGI(vec2 uv, float roughness)
{
	float lod = roughness * 13;
	float min = floor(lod);
	float max = ceil(lod);
	return mix(textureLod(GISkybox, uv, min), textureLod(GISkybox, uv, max), (max - min));
}

vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
	vec4 gi = sampleGI(SampleSphericalMap(R), Roughness);
	return gi.rgb;
}

vec2 IntegrateBRDF(float Roughness, float NoV)
{
	return textureLod(BRDFLUTTex, vec2(Roughness, NoV), 1.0).rg;
}


// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
vec3 F_Schlick( vec3 SpecularColor, float VoH )
{
	float Fc = pow( 1 - VoH, 5 );				// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return clamp( 50.0 * SpecularColor.g, 0, 1 ) * Fc + (1 - Fc) * SpecularColor;
	
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   


vec3 ApproximateSpecularIBL( vec3 SpecularColor , float Roughness, vec3 N, vec3 V )
{
	float NoV = clamp( dot( N, V ), 0, 1 );
	vec3 R = 2 * dot( V, N ) * N - V;
	vec3 PrefilteredColor = PrefilterEnvMap( Roughness, R );
	vec2 EnvBRDF = IntegrateBRDF( Roughness, NoV );
	return PrefilteredColor * ( SpecularColor * EnvBRDF.x + EnvBRDF.y );
}

vec3 PerturbNormal(  vec3 vertexNormal,
                     vec3 tangentNormal,
                     vec3 worldPosition,
                     vec2 uv)
{
    vec3 dxPos = dFdx(worldPosition);
    vec3 dyPos = dFdy(worldPosition);
    vec2 dxUV  = dFdx(uv);
    vec2 dyUV  = dFdy(uv);

    vec3 N = normalize(vertexNormal);
    vec3 T = normalize((dyUV.y * dxPos) - (dxUV.y * dyPos));
    vec3 B = normalize(cross(N, T));

    return normalize(tangentNormal * mat3(T, B, N));
}



void main() {
	vec2 uvNor = uv;
	uvNor.y = 1 - uvNor.y;
	vec4 color = texture(mainTex, uvNor);
	vec3 diffuse = color.rgb;
	//if (color.a < 0.01) discard;

	mat3 tbn = mat3(tangent, bitangent, worldNormal);

	vec3 normal = texture(normalTex, uvNor).rgb;
	normal = normalize(normal * 2 - 1);

	// normal = PerturbNormal(worldNormal, normal, worldPos, uv);

	// normal = vec3(normal.x, normal.z, normal.y);
	normal = mix(normal, vec3(0, 0, 1), 0.0);
	normal = normalize(tangentToWorld * normal);
	// normal = normalize(tbn * normal);


	vec4 pbrParam = texture(specTex, uvNor);
	float roughness = pbrParam.g;
	float metallic = pbrParam.b;

	vec3 viewdir = normalize(ubo.cameraPos.xyz - worldPos);
	//vec3 lit = basicLit(normal, -tLightDir, ubo.directionalLightColor.xyz, tViewdir, color.rgb);
	vec3 lit = basicLit(normal, -ubo.directionalLightDir.xyz, ubo.directionalLightColor.xyz, viewdir, color.rgb);
	color.rgb = lit;


	float fresnel = pow(1 - dot(worldNormal, viewdir), 5);
	float reflective = clamp(fresnel + metallic, 0, 1);

	//GI
	vec3 reflectdir = reflect(-viewdir, normal);
	//vec4 gicolor = texture(GISkybox, SampleSphericalMap(reflectdir));
	vec4 gicolor = sampleGI(SampleSphericalMap(reflectdir), roughness);
	vec3 specularColor = mix(vec3(0.04), color.rgb, metallic);


	vec3 l = normalize(-ubo.directionalLightDir.xyz);
	vec3 v = normalize(viewdir);
	vec3 h = normalize(l + v);

	float VoH = clamp(dot(viewdir, h), 0, 1);
	// specularColor = F_Schlick(specularColor, VoH);
	specularColor = fresnelSchlickRoughness(max(0, dot(viewdir, normal)), specularColor, roughness);
	gicolor.rgb = ApproximateSpecularIBL(specularColor, roughness, normal, viewdir);
	color.rgb += gicolor.rgb;
	// color.rgb += mix(vec3(1), color.rgb, metallic) * gicolor.rgb * reflective;
	//color.rgb += diffuse.rgb * gicolor.rgb * metallic + (1 - metallic) * gicolor.rgb;
	// color.rgb = texture(normalTex, uvNor).rgb;
	// color.rgb = vec3(roughness, metallic, 0.0);
	//color.rgb = tangent / 2 + 0.5;

	//color.rgb = normal;
	//color.rgb = vec3(0, normal.y, 0);
	color.rgb = tangent * 0.5 + 0.5;
	color.rgb = pow(color.rgb, vec3(1.0/2.1));
    outColor = vec4(color.rgb, 1.0);
}



