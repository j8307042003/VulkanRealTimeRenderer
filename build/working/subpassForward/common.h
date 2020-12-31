
#define PI 3.1415926

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1 - uv.y;
    return uv;
}


vec4 sampleGI_(sampler2D tex, vec2 uv, float roughness)
{
	float lod = min(roughness, 1.0) * 13;
	vec2 lodQuery = textureQueryLod(tex, uv);
	float min = max(lodQuery.x, floor(lod));
	float max = max(lodQuery.x, ceil(lod));
	return mix(textureLod(tex, uv, min), textureLod(tex, uv, max), (max - min));
}


// Converts a color from linear light gamma to sRGB gamma
vec4 fromLinear(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(vec4 sRGB)
{
    bvec4 cutoff = lessThan(sRGB, vec4(0.04045));
    vec4 higher = pow((sRGB + vec4(0.055))/vec4(1.055), vec4(2.4));
    vec4 lower = sRGB/vec4(12.92);

    return mix(higher, lower, cutoff);
}



vec3 PrefilterEnvMap(sampler2D GISource, float Roughness, vec3 R)
{
	vec4 gi = sampleGI_(GISource, SampleSphericalMap(R), Roughness) * 1.0;
	return gi.rgb;
}


// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
vec3 F_Schlick( vec3 SpecularColor, float VoH )
{
	float Fc = pow( 1 - VoH, 5 );				// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return clamp( 50.0 * SpecularColor.g, 0, 1 ) * Fc + (1 - Fc) * SpecularColor;
	
}

vec3 F_SchlickDisney(vec3 f0, float f90, float VoH) {
	return f0 + (vec3(f90) - f0) * pow(1.0 - VoH, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 Fr_DisneyDiffuse (vec3 f0, float NdotV , float NdotL , float LdotH ,
							float linearRoughness )
{
	float energyBias = mix (0 , 0.5 , linearRoughness );
	float energyFactor = mix (1.0 , 1.0 / 1.51 , linearRoughness );
	float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness ;
	vec3 lightScatter = F_SchlickDisney ( f0 , fd90 , NdotL );
	vec3 viewScatter = F_SchlickDisney (f0 , fd90 , NdotV );

	return lightScatter * viewScatter * energyFactor ;
}


vec3 fresnelSchlickRoughnessDisney(vec3 normal, vec3 lightdir, vec3 viewdir, vec3 F0, float roughness)
{
	vec3 l = normalize(lightdir);
	vec3 v = normalize(viewdir);
	vec3 h = normalize(l + v);

	float NdotV = abs(dot(normal, viewdir)) + 1e-5f;

	return Fr_DisneyDiffuse(F0, NdotV, max(0, dot(normal, l)), max(0, dot(l, h)), roughness );
}







vec3 ApproximateSpecularIBL(sampler2D GISource, sampler2D BRDFLUTTexture, vec3 SpecularColor , float Roughness, vec3 N, vec3 V )
{
	float NoV = clamp( dot( N, V ), 0, 1 );
	vec3 R = reflect(-V, N);
	vec3 PrefilteredColor = PrefilterEnvMap(GISource, Roughness, R );
	vec2 EnvBRDF = (texture(BRDFLUTTexture, vec2(NoV, 1.0 - Roughness))).rg;
	return PrefilteredColor * ( SpecularColor * EnvBRDF.x + EnvBRDF.y );
}

float V_Kelemen(float LoH) {
    return 0.25 / (LoH * LoH);
}

float D_GGX(float roughness, float NoH) {
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float Fd_Burley(float NoV, float NoL, float LoH, float roughness) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_SchlickDisney(vec3(1.0), f90, NoL).r;
    float viewScatter = F_SchlickDisney(vec3(1.0), f90, NoV).r;
    return lightScatter * viewScatter * (1.0 / PI);
}

struct BrdfInput
{
	vec3 albedo;
	vec3 specular;
	float metallic;
	float roughness;
	vec3 normal;
	vec3 viewdir;
	vec3 lightdir;
	vec3 lightColor;

	float clearCoat;
	float clearCoatRoughness;
	float shadowMask;
};


vec3 brdf_light(BrdfInput brdfInput)
{

	vec3 albedo = brdfInput.albedo;
	vec3 normal = brdfInput.normal;
	vec3 viewdir = brdfInput.viewdir;
	vec3 specularColor = brdfInput.specular;
	vec3 lightdir = brdfInput.lightdir;
	float metallic = brdfInput.metallic;
	float roughness = brdfInput.roughness;

	vec3 l = normalize(lightdir);
	vec3 v = normalize(viewdir);
	vec3 h = normalize(l + v);
	float LoH = max(0, dot(l, h));
	float NoH = max(0, dot(normal, h));	
	float NoV = max(0, dot(normal, v));
	float NoL = max(0, dot(normal, l));
	float VoH = max(0, dot(v, h));

	vec3 f0 = mix(vec3(0.04), specularColor, metallic);

	float D = D_GGX(roughness, NoH);
	vec3 F = F_Schlick(f0, VoH );
	F = F_SchlickDisney(f0, 1.0, LoH);
	float G = clamp(V_SmithGGXCorrelated(NoV, NoL, roughness), 0.0, 1.0);


	vec3 specular = min(vec3(1.0), max(D * F * G * 1.0, vec3(0.0)));
	//specular = G > 2.0 ? vec3(1.0) : vec3(0.0);
	vec3 diffuse = albedo * min(1.0, Fd_Burley(NoV, NoL, LoH, roughness));
	return (diffuse + specular) * brdfInput.lightColor * (1.0 - clamp(brdfInput.shadowMask, 0.0, 1.0));
}

vec3 brdf_IBL(BrdfInput brdfInput, sampler2D GISource, sampler2D BRDFLUTTexture)
{

	vec3 albedo = brdfInput.albedo;
	vec3 normal = brdfInput.normal;
	vec3 viewdir = brdfInput.viewdir;
	vec3 specularColor = brdfInput.specular;
	float metallic = brdfInput.metallic;
	float roughness = brdfInput.roughness;

	vec3 f0 = mix(vec3(0.04), specularColor, metallic);
	f0 = fresnelSchlickRoughness(max(0, dot(viewdir, normal)), f0, roughness);
	vec3 specular = ApproximateSpecularIBL(GISource, BRDFLUTTexture, f0, roughness, normal, viewdir);
	vec3 diffuse = PrefilterEnvMap(GISource, 0.9, normal) * albedo * (1 - metallic) * (1.0 - f0);	
	return diffuse + specular;
}


vec3 brdf(sampler2D GISource, sampler2D BRDFLUTTexture, BrdfInput brdfInput)
{
	vec3 result = vec3(0);

	vec3 albedo = brdfInput.albedo;
	vec3 normal = brdfInput.normal;
	vec3 viewdir = brdfInput.viewdir;
	vec3 specularColor = brdfInput.specular;
	vec3 lightdir = brdfInput.lightdir;
	float metallic = brdfInput.metallic;
	float roughness = brdfInput.roughness;
	float clearCoat = brdfInput.clearCoat;
	float clearCoatRoughness = brdfInput.clearCoatRoughness;


	float NoV = max(0, dot(normal, normalize(viewdir)));


	vec3 f0 = mix(vec3(0.04), specularColor, metallic);
	f0 = specularColor;
	f0 = fresnelSchlickRoughness(max(0, dot(viewdir, normal)), f0, roughness);

	const float envAtten = 0.7;
	float envLitScale = max(envAtten, 1.0 - brdfInput.shadowMask);

	vec3 F_IBL = brdf_IBL(brdfInput, GISource, BRDFLUTTexture) * envLitScale;
	vec3 F_light = brdf_light(brdfInput);
	vec3 F = F_IBL + F_light;


    // clear coat BRDF
    float  Fc = fresnelSchlickRoughness(NoV, vec3(0.04), clearCoatRoughness).r * clearCoat; // clear coat strength
	vec3 clearCoatspecular = ApproximateSpecularIBL(GISource, BRDFLUTTexture, vec3(0.04), clearCoatRoughness, normal, viewdir);



	//result = (diffuse * ( (1.0 - f) * (1.0 - metallic) ) + specular) * ( 1 - Fc) + Frc * clearCoatspecular;
	result = F * ( 1 - Fc) + Fc * clearCoatspecular * envLitScale;
	//result = (diffuse + specular);
	result *= 2.0;

	return result;
}



float GetShadow(sampler2D shadowMap, vec4 shadowClipSpace)
{
	float shadow = 0;

	const int sampleNum = 8;
	const float bias = 0.005;
	const float distanceScale = 1 / 10.0;

	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	vec2 sampleuv = shadowClipSpace.xy * 0.5 + 0.5;

	float count = 0;
	float maxWeight = length(vec2(sampleNum/2+1, sampleNum/2+1));
	for(int i = -sampleNum/2; i <= sampleNum/2; i++)
	{
		for (int j = -sampleNum/2; j <= sampleNum/2; ++j) 
		{

			float d = length(vec2(i, j));
			count = count + d;


			vec4 shadowmapSample = texture(shadowMap, sampleuv + vec2(i, j) * texelSize);
			float curShadow = shadowmapSample.r < abs(shadowClipSpace.z) - bias ? 1.0 : 0.0;

			if (abs(shadowClipSpace.x) > 1.0 || abs(shadowClipSpace.y) > 1.0) curShadow = 0.0;		
			//shadow += curShadow * (maxWeight - d);
			shadow += curShadow;
		}
	}
 
	//return shadow / (sampleNum * sampleNum);
	return shadow / ((sampleNum+1) * (sampleNum+1));
	//return shadow / ((sampleNum + 1));
	//return min(1.0, shadow / count);
	//return 0;

}
