#version 400 core

in vec2 TexCoords;
in vec3 envMapCoords;
out vec4 colorOutput;


float PI  = 3.14159265359f;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gEffects;

uniform sampler2D ssao;
uniform sampler2D envMap;
uniform sampler2D envMapIrradiance;
//uniform sampler2D envMapPrefilter;
//uniform sampler2D brdfLUT;

uniform int gBufferView;
uniform int brdfMaxSamples;
uniform float materialRoughness;
uniform float materialMetallicity;
uniform float ambientIntensity;
uniform vec3 materialF0;
uniform mat4 view;

float Fd90(float NoL, float roughness);
float KDisneyTerm(float NoL, float NoV, float roughness);
vec3 FresnelSchlick(float NdotV, vec3 F0);
vec3 FresnelSchlick(float NdotV, vec3 F0, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness);
vec3 colorLinear(vec3 colorVector);
float saturate(float f);
vec2 getSphericalCoord(vec3 normalCoord);
float radicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N);
vec3 prefilterEnvMap(float roughness, vec3 R);
vec2 integrateBRDF(float roughness, float NoV);
vec3 computeApproximateRadiance(vec3 specularColor, float roughness, vec3 N, vec3 V);


void main()
{
    // Retrieve G-Buffer informations
    vec3 viewPos = texture(gPosition, TexCoords).rgb;
    vec3 albedo = colorLinear(texture(gAlbedo, TexCoords).rgb);
    vec3 normal = texture(gNormal, TexCoords).rgb;
    float roughness = texture(gAlbedo, TexCoords).a;
    float metalness = texture(gNormal, TexCoords).a;
    float ao = texture(gEffects, TexCoords).r;
    vec2 velocity = texture(gEffects, TexCoords).gb;
    float depth = texture(gPosition, TexCoords).a;

    float ssao = texture(ssao, TexCoords).r;
    vec3 envColor = texture(envMap, getSphericalCoord(normalize(envMapCoords))).rgb;

    vec3 color = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);

    if(depth == 1.0f)
    {
        color = envColor;
    }

    else
    {
        vec3 V = normalize(- viewPos);
        vec3 N = normalize(normal);

        float NdotV = saturate(dot(N, V));

        // Fresnel (Schlick) computation (F term)
        vec3 F0 = mix(materialF0, albedo, metalness);
        vec3 F = FresnelSchlick(NdotV, F0, roughness);

        // Energy conservation
        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - metalness;

        // Diffuse irradiance computation
        vec3 irradiance = texture(envMapIrradiance, getSphericalCoord(N * mat3(view))).rgb;
        diffuse = irradiance * (albedo / PI);

        specular = computeApproximateRadiance(F, roughness, N, V);

        color = (diffuse * kD) + specular;
    }


    // Switching between the different buffers
    // Final buffer
    if(gBufferView == 1)
        colorOutput = vec4(color, 1.0f);

    // Position buffer
    else if (gBufferView == 2)
        colorOutput = vec4(viewPos, 1.0f);

    // View Normal buffer
    else if (gBufferView == 3)
        colorOutput = vec4(normal, 1.0f);

    // Color buffer
    else if (gBufferView == 4)
        colorOutput = vec4(albedo, 1.0f);

    // Roughness buffer
    else if (gBufferView == 5)
        colorOutput = vec4(vec3(roughness), 1.0f);

    // Metalness buffer
    else if (gBufferView == 6)
        colorOutput = vec4(vec3(metalness), 1.0f);

    // Depth buffer
    else if (gBufferView == 7)
        colorOutput = vec4(vec3(depth/50.0f), 1.0f);

    // SSAO buffer
    else if (gBufferView == 8)
        colorOutput = vec4(vec3(ssao), 1.0f);

    // Velocity buffer
    else if (gBufferView == 9)
        colorOutput = vec4(velocity, 0.0f, 1.0f);
}



float Fd90(float NoL, float roughness)
{
  return (2.0f * NoL * roughness) + 0.4f;
}


float KDisneyTerm(float NoL, float NoV, float roughness)
{
  return (1.0f + Fd90(NoL, roughness) * pow(1.0f - NoL, 5.0f)) * (1.0f + Fd90(NoV, roughness) * pow(1.0f - NoV, 5.0f));
}


vec3 FresnelSchlick(float NdotV, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - NdotV, 5.0f);
}


vec3 FresnelSchlick(float NdotV, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - NdotV, 5.0f);
}


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0f) + 1.0f) * (NdotH2 * (alpha2 - 1.0f) + 1.0f));
}


float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness + 0.0001f;

    float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
    float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

    return ggxL * ggxV;
}


vec3 colorLinear(vec3 colorVector)
{
  vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));

  return linearColor;
}


float saturate(float f)
{
    return clamp(f, 0.0f, 1.0f);
}


vec2 getSphericalCoord(vec3 normalCoord)
{
    float phi = acos(-normalCoord.y);
    float theta = atan(1.0f * normalCoord.x, -normalCoord.z) + PI;

    return vec2(theta / (2.0f * PI), phi / PI);
}


float radicalInverse_VdC(uint bits) // In place of bitfieldreverse()
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10;
}


vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}


vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
    float alpha = roughness * roughness;

    float Phi = 2 * PI * Xi.x;
    float cosT = sqrt((1.0f - Xi.y) / (1.0f + (alpha * alpha - 1.0f) * Xi.y));
    float sinT = sqrt(1.0f - cosT * cosT);

    vec3 H;
    H.x = sinT * cos(Phi);
    H.y = sinT * sin(Phi);
    H.z = cosT;

    vec3 vecUp = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 tanX = normalize(cross(vecUp, N));
    vec3 tanY = cross(N, tanX);

    return normalize(tanX * H.x + tanY * H.y + N * H.z);
}


vec3 prefilterEnvMap(float roughness, vec3 R)
{
    vec3 N = R;
    vec3 V = R;
    vec3 prefilteredColor = vec3(0.0f);
    uint numSamples = brdfMaxSamples;
    float totalWeight = 0.0f;

    for(uint i = 0; i < numSamples; i++)
    {
        vec2 Xi = Hammersley(i, numSamples);
        vec3 H = ImportanceSampleGGX(Xi, roughness, N);
        vec3 L = 2 * dot(V, H) * H - V;

        float NdotL = saturate(dot(N, L));

        if(NdotL > 0.0f)
        {
            prefilteredColor += textureLod(envMap, getSphericalCoord(L * mat3(view)), 0).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    return prefilteredColor / totalWeight;
}


vec2 integrateBRDF(float roughness, float NdotV)
{
    vec3 V;
    V.x = sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;
    uint numSamples = brdfMaxSamples;

    vec3 N = vec3(0.0f, 0.0f, 1.0f);

    for(uint i = 0; i < numSamples; i++)
    {
        vec2 Xi = Hammersley(i, numSamples);
        vec3 H = ImportanceSampleGGX(Xi, roughness, N);
        vec3 L = normalize(2 * dot(V, H) * H - V);

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));

        if(NdotL > 0.0f)
        {
            float G = GeometryAttenuationGGXSmith(NdotL, NdotV, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return vec2(A, B) / numSamples;
}


vec3 computeApproximateRadiance(vec3 specularColor, float roughness, vec3 N, vec3 V)
{
    float NdotV = saturate(dot(N, V));
    vec3 R = 2.0f * dot(V, N) * N - V;

    vec3 prefilteredColor = prefilterEnvMap(roughness, R);
    vec2 envBRDF = integrateBRDF(roughness, NdotV);

    return prefilteredColor * (specularColor * envBRDF.x + envBRDF.y);
}