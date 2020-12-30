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
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;
// layout(location = 6) out mat3 tangentToWorld;
layout(location = 6) out vec4 shadowmapPos;

layout(binding = 0) uniform UBO {
	mat4 mvpMatrix;
	mat4 vpMatrix;
	mat4 projectionMatrix;
	mat4 worldToCamMatrix;		
    vec4 cameraPos;
    vec4 directionalLightDir;
    vec4 directionalLightColor;
    mat4 shadowmapVpMatrix;
} ubo;

layout(binding = 1) uniform CustomUBO {
	vec3 custom;
} customUBO;

layout(push_constant) uniform PushConsts {
	mat4 modelMatrix;
} pushConsts;



mat3 CreateTangentToWorldPerVertex(vec3 normal, vec3 tangent, float tangentSign)
{
    // For odd-negative scale transforms we need to flip the sign
    float sign = tangentSign * 1.0;
    vec3 binormal = cross(normal, tangent) * sign;
    return mat3(tangent, binormal, normal);
}

vec3 CreateBinormal(vec3 normal, vec3 tangent, float tangentSign)
{
    // For odd-negative scale transforms we need to flip the sign
    float sign = tangentSign * 1.0;
    vec3 binormal = cross(normal, tangent) * sign;
    return binormal;
}

const mat4 biasMat = mat4( 
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 );

void main() {
    vec4 outPos = ubo.mvpMatrix * vec4(pos, 1.0);
    outPos = ubo.projectionMatrix * ubo.worldToCamMatrix * pushConsts.modelMatrix * vec4(pos, 1.0);
    gl_Position = outPos;

    mat4 objectToWorld = inverse(pushConsts.modelMatrix);
    mat3 normalMatrix = transpose(inverse(mat3(pushConsts.modelMatrix)));

    fragColor = color.rgb;// * ubo.colorScale.x;
    fragUV = uv;
    worldPos = (pushConsts.modelMatrix * vec4(pos, 1.0)).xyz;
    fragNormal = normalize(mat3(normalMatrix)*normalize(normal));
    fragTangent = normalize(mat3(normalMatrix)*normalize(tangent));
    fragBitangent = normalize(mat3(normalMatrix)*normalize(bitangent));
    // fragBitangent = CreateBinormal(fragNormal, fragTangent, bitangent.x);

    //mat3 tbn = mat3(fragTangent, fragBitangent, fragNormal);
    mat3 tbn = mat3(fragTangent, fragBitangent, fragNormal);
    //tangentToWorld = (tbn);


    shadowmapPos = ubo.shadowmapVpMatrix * pushConsts.modelMatrix * vec4(pos, 1.0);
}