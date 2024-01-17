#version 400 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gRoughness;
layout (location = 4) out vec3 gMetalness;

in vec2 TexCoords;
in vec3 worldPos;
in vec3 normal;

uniform vec3 albedoColor;
uniform sampler2D texAlbedo;
uniform sampler2D texNormal;
uniform sampler2D texRoughness;
uniform sampler2D texMetalness;

const float nearPlane = 0.1;
const float farPlane = 100.0f;

float LinearizeDepth(float depth);


void main()
{
    gPosition.xyz = worldPos;
    gPosition.a = LinearizeDepth(gl_FragCoord.z);

    gNormal = normalize(normal);

    //gAlbedo.rgb = vec3(albedoColor);
    gAlbedo.rgb = vec3(texture(texAlbedo, TexCoords));
    gRoughness.rgb = vec3(texture(texRoughness, TexCoords));
    gMetalness.rgb = vec3(texture(texMetalness, TexCoords));
}



float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}