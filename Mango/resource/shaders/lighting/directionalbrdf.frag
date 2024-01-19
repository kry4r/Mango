#version 400 core

in vec2 TexCoords;
out vec4 colorOutput;


struct LightObject {
    vec3 direction;
    vec4 color;
};

float PI  = 3.14159265359f;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughness;
uniform sampler2D gMetalness;
uniform sampler2D gAO;
uniform sampler2D ssao;

// Light source(s) informations
uniform int lightDirectionalCounter = 1;
uniform LightObject lightDirectionalArray[1];

uniform int gBufferView;
uniform float materialRoughness;
uniform float materialMetallicity;
uniform float ambientIntensity;
uniform vec3 viewPos;
uniform vec3 materialF0;


float Fd90(float NoL, float roughness);
float KDisneyTerm(float NoL, float NoV, float roughness);
vec3 FresnelSchlick(float NdotV, vec3 F0);
vec3 FresnelSchlick(float NdotV, vec3 F0, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness);
vec3 colorLinear(vec3 colorVector);
vec3 colorSRGB(vec3 colorVector);
vec3 ReinhardTM(vec3 color);
float saturate(float f);
vec2 saturate(vec2 vec);
vec3 saturate(vec3 vec);

void main()
{
    // 获取G-Buffer信息
    vec3 worldPos = texture(gPosition, TexCoords).rgb; // 世界坐标
    vec3 normal = texture(gNormal, TexCoords).rgb; // 法线
    vec3 albedo = colorLinear(texture(gAlbedo, TexCoords).rgb);
    float roughness = texture(gRoughness, TexCoords).r;
    float metalness = texture(gMetalness, TexCoords).r;
    float ao = texture(gAO, TexCoords).r;
    float ssao = texture(ssao, TexCoords).r;
    float depth = texture(gPosition, TexCoords).a; // 深度

    vec3 V = normalize(- worldPos); // 视点方向
    vec3 N = normalize(normal); // 法线方向
    vec3 R = normalize(reflect(- V, N)); // 反射方向

    vec3 color = vec3(0.0f); // 最终颜色
    vec3 diffuse = vec3(0.0f); // 漫反射颜色
    vec3 specular = vec3(0.0f); // 镜面反射颜色
    

    vec3 ambient = ao * albedo * vec3(ambientIntensity);

    // Light source independent BRDF term(s)
    float NdotV = saturate(dot(N, V));

    // Fresnel (Schlick) computation (F term)
    vec3 F0 = mix(materialF0, albedo, metalness);
    vec3 F = FresnelSchlick(NdotV, F0, roughness);

    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0f) - kS;
    kD *= 1.0f - metalness;

    for (int i = 0; i < lightDirectionalCounter; i++)
    {
        vec3 L = normalize(- lightDirectionalArray[i].direction);
        vec3 H = normalize(L + V);

        vec3 lightColor = colorLinear(lightDirectionalArray[i].color.rgb);

        // BRDF terms
        float NdotL = saturate(dot(N, L));

        // Diffuse component computation
//        diffuse = albedo/PI - (albedo/PI) * metalness;    // The right way to compute diffuse but any surface that should reflect the environment would appear black at the moment...
        diffuse = (albedo/PI);

        // Disney diffuse term
        float kDisney = KDisneyTerm(NdotL, NdotV, roughness);


        // Distribution (GGX) computation (D term)
        float D = DistributionGGX(N, H, roughness);

        // Geometry attenuation (GGX-Smith) computation (G term)
        float G = GeometryAttenuationGGXSmith(NdotL, NdotV, roughness);

        // Specular component computation
        specular = (F * D * G) / (4 * NdotL * NdotV + 0.0001f);

        color += (diffuse * kDisney * kD + specular) * lightColor * NdotL * ssao;
    }
    color += ambient;
    // Reinhard Tonemapping
    color = ReinhardTM(color);

    // Switching between the different buffers
    if(gBufferView == 1)
    {
        color = colorSRGB(color);
        colorOutput = vec4(color, 1.0f);
    }
    // Position buffer
    else if (gBufferView == 2)
        colorOutput = vec4(worldPos, 1.0f);

    // World Normal buffer
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

    // AO buffer
    else if (gBufferView == 8)
        colorOutput = vec4(vec3(ssao), 1.0f);
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

    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0f) + 1.0f) * (NdotH2 * (alpha2 - 1.0f) + 1.0f));
}


float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness;

    float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
    float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

    return ggxL * ggxV;
}


vec3 colorLinear(vec3 colorVector)
{
  vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));

  return linearColor;
}


vec3 colorSRGB(vec3 colorVector)
{
  vec3 srgbColor = pow(colorVector.rgb, vec3(1.0f / 2.2f));

  return srgbColor;
}

vec3 ReinhardTM(vec3 color)
{
    return color / (color + vec3(1.0f));
}


float saturate(float f)
{
    return clamp(f, 0.0, 1.0);
}


vec2 saturate(vec2 vec)
{
    return clamp(vec, 0.0, 1.0);
}


vec3 saturate(vec3 vec)
{
    return clamp(vec, 0.0, 1.0);
}