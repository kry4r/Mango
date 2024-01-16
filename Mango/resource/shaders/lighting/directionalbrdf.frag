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
uniform sampler2D gColor;
uniform sampler2D ssao;

// Light source(s) informations
uniform int lightDirectionalCounter = 3;
uniform LightObject lightDirectionalArray[3];

uniform int gBufferView;
uniform float materialRoughness;
uniform float materialMetallicity;
uniform float ssaoVisibility;
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
float saturate(float f);
vec2 saturate(vec2 vec);
vec3 saturate(vec3 vec);


void main()
{
    // 获取G-Buffer信息
    vec3 worldPos = texture(gPosition, TexCoords).rgb; // 世界坐标
    vec3 normal = texture(gNormal, TexCoords).rgb; // 法线
    vec3 albedo = colorLinear(texture(gColor, TexCoords).rgb); // 反射率
    float ao = texture(ssao, TexCoords).r; // 环境光遮蔽
    float depth = texture(gPosition, TexCoords).a; // 深度

    vec3 V = normalize(- worldPos); // 视点方向
    vec3 N = normalize(normal); // 法线方向
    vec3 R = normalize(-reflect(V, N)); // 反射方向

    vec3 color = vec3(0.0f); // 最终颜色
    vec3 diffuse = vec3(0.0f); // 漫反射颜色
    vec3 specular = vec3(0.0f); // 镜面反射颜色

    for (int i = 0; i < lightDirectionalCounter; i++)
    {
        vec3 L = normalize(- lightDirectionalArray[i].direction); // 光源方向
        vec3 H = normalize(L + V); // 半程向量

        vec3 lightColor = colorLinear(lightDirectionalArray[i].color.rgb); // 光源颜色

        // BRDF项计算
        float NdotL = dot(N, L); // 法线和光线方向的点积
        float NdotV = dot(N, V); // 法线和视点方向的点积

        if(NdotL > 0)
        {
            // Lambertian计算
            diffuse = (albedo/PI) - (albedo/PI) * materialMetallicity;

            // Disney漫反射项
            float kDisney = KDisneyTerm(NdotL, NdotV, materialRoughness);

            // Fresnel (Schlick)计算 (F项)
            // F0 = 0.04 --> 电介质 UE4
            // F0 = 0.658 --> 玻璃
            vec3 F = FresnelSchlick(max(NdotV, 0.0), materialF0);

            // Distribution (GGX)计算 (D项)
            float D = DistributionGGX(N, H, materialRoughness);

            // Geometry attenuation (GGX-Smith)计算 (G项)
            float G = GeometryAttenuationGGXSmith(NdotL, NdotV, materialRoughness);

            // 镜面反射项计算
            specular = (F * D * G) / (4 * NdotL * NdotV);

            // 将颜色分量限制在0.0到1.0之间
            diffuse = saturate(diffuse);
            specular = saturate(specular);

            // SSAO
            vec3 ssao = vec3(ssaoVisibility * ao);

            // 计算最终颜色
            color += ssao * lightColor * NdotL * (diffuse * kDisney * (1.0f - specular) + specular);
        }
    }

    // Switching between the different buffers
    if(gBufferView == 1)
    {
        color = colorSRGB(color);
        colorOutput = vec4(color, 1.0);
    }
    // Position
    else if (gBufferView == 2)
        colorOutput = vec4(worldPos, 1.0f);
    // Normal
    else if (gBufferView == 3)
        colorOutput = vec4(normal, 1.0f);
    // Depth
    else if (gBufferView == 4)
        colorOutput = vec4(vec3(depth/50.0f), 1.0f);
    // SSAO
    else if (gBufferView == 5)
        colorOutput = vec4(vec3(ao), 1.0f);
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
    return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
}


vec3 FresnelSchlick(float NdotV, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - NdotV, 5.0);
}


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0) + 1.0) * (NdotH2 * (alpha2 - 1.0) + 1.0));
}


float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness;

    float ggxL = (2.0 * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0 - NdotL2)));
    float ggxV = (2.0 * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0 - NdotV2)));

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