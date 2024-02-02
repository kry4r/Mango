#version 400 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gRoughness;
layout (location = 4) out vec3 gMetalness;
layout (location = 5) out vec3 gAO;

in vec2 TexCoords;
in vec3 viewPos;
in vec3 normal;

uniform vec3 albedoColor;
uniform sampler2D texAlbedo;
uniform sampler2D texNormal;
uniform sampler2D texRoughness;
uniform sampler2D texMetalness;
uniform sampler2D texAO;

const float nearPlane = 0.1;
const float farPlane = 100.0f;

float LinearizeDepth(float depth);
vec3 computeTexNormal(vec3 viewNormal, vec3 texNormal);


void main()
{
    vec3 texNormal = normalize(texture(texNormal, TexCoords).rgb * 2.0 - 1.0);
    texNormal.g = -texNormal.g;   // In case the normal map was made with DX3D coordinates system in mind

    //gAlbedo.rgb = vec3(albedoColor);
    gAlbedo.rgb = vec3(texture(texAlbedo, TexCoords));
    gNormal.rgb = computeTexNormal(normal, texNormal);
    gRoughness.rgb = vec3(texture(texRoughness, TexCoords));
    gMetalness.rgb = vec3(texture(texMetalness, TexCoords));
    gAO.rgb = vec3(texture(texAO, TexCoords));
    gPosition.xyz = viewPos;
    gPosition.a = LinearizeDepth(gl_FragCoord.z);
}



float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

vec3 computeTexNormal(vec3 viewNormal, vec3 texNormal)
{
    vec3 dPosX  = dFdx(viewPos);
    vec3 dPosY  = dFdy(viewPos);
    vec2 dTexX = dFdx(TexCoords);
    vec2 dTexY = dFdy(TexCoords);

    vec3 normal = normalize(viewNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * texNormal);
}