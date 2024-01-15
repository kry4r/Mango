#version 400

struct LightObject {
    vec3 position;
    vec4 color;
};


uniform int lightCounter = 3;
uniform LightObject lightArray[3];
uniform samplerCube cubemap;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform vec3 albedoDiffuseColor;
uniform vec3 F0;
uniform float roughness;
uniform float metallicity;

out vec4 colorOutput;

in vec3 worldPos;
in vec2 TexCoords;
in vec3 normal;
in vec3 baseColor;

float PI  = 3.14159265359f;



float Fd90(float NoL)
{
  return (2.0f * NoL * roughness) + 0.4f;
}


float KDisneyTerm(float NoL, float NoV)
{
  return (1.0f + Fd90(NoL) * pow(1.0f - NoL, 5.0f)) * (1.0f + Fd90(NoV) * pow(1.0f - NoV, 5.0f));
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


vec3 colorLinear(vec3 colorVector) {
  vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));

  return vec3(linearColor);
}


vec3 colorSRGB(vec3 colorVector) {
  vec3 srgbColor = pow(colorVector.rgb, vec3(1.0f / 2.2f));

  return vec3(srgbColor);
}



void main()
{
    vec3 V = normalize(viewPos - worldPos);
    vec3 N = normalize(normal);
    vec3 R = normalize(-reflect(V, N));

    vec3 color = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);
    vec3 envMap = texture(cubemap, R).rgb;

    for (int i = 0; i < lightCounter; i++)
    {
        vec3 L = normalize(lightArray[i].position - worldPos);
        vec3 H = normalize(L + V);

        vec3 lightColor = colorLinear(lightArray[i].color.rgb);
        float distanceL = distance(lightArray[i].position, worldPos);
        float attenuation = 1.0 / (distanceL * distanceL);

        // BRDF terms
        float NdotL = dot(N, L);
        float NdotV = dot(N, V);

        if(NdotL > 0)
        {
            // Lambertian computation
            diffuse = (albedoDiffuseColor/PI) - (albedoDiffuseColor/PI) * metallicity;


            // Disney diffuse term
            float kDisney = KDisneyTerm(NdotL, NdotV);


            // Fresnel (Schlick) computation (F term)
            // F0 = 0.04 --> dielectric UE4
            // F0 = 0.658 --> Glass
            vec3 F = FresnelSchlick(max(dot(N, V), 0.0), F0);


            // Distribution (GGX) computation (D term)
            float D = DistributionGGX(N, H, roughness);


            // Geometry attenuation (GGX-Smith) computation (G term)
            float G = GeometryAttenuationGGXSmith(NdotL, NdotV, roughness);


            // Specular component computation
            specular = (F * D * G) / (4 * NdotL * NdotV);


            // Attenuation computation
            diffuse *= attenuation;
            specular *= attenuation;


            color += lightColor * NdotL * (diffuse * kDisney * (1.0f - specular) + specular);
        }
    }

    color = pow(color, vec3(1.0/2.2));
    colorOutput = vec4(color, 1.0);
}