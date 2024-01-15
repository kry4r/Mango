#version 400 core

out float ssaoOutput;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform int viewportWidth;
uniform int viewportHeight;
uniform int ssaoKernelSize;
uniform int ssaoNoiseSize;
uniform float ssaoRadius;
uniform float ssaoPower;
uniform vec3 samples[64];
uniform mat4 projection;

vec2 noiseScale = vec2(viewportWidth/ssaoNoiseSize, viewportHeight/ssaoNoiseSize);


void main()
{
    // Retrieve G-Buffer informations
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

    // TBN : Tangent Space --> View Space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;

    for(int i = 0; i < ssaoKernelSize; ++i)
    {
        // AO kernel sample position from TBN
        vec3 sampleAO = TBN * samples[i];
        sampleAO = fragPos + sampleAO * ssaoRadius;

        // Get the kernel sample position on screen
        vec4 offset = vec4(sampleAO, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // Get the kernel sample's depth information
        float sampleDepth = -texture(gPosition, offset.xy).w;

        // Check if the sample should be added or not
        float rangeCheck = smoothstep(0.0, 1.0, ssaoRadius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sampleAO.z ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / ssaoKernelSize);

    ssaoOutput = pow(occlusion, ssaoPower);
}