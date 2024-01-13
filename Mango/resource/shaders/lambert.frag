#version 330 core

#define NR_POINT_LIGHTS 1

out vec4 colorOutput;

in vec3 FragPosition;
in vec2 TexCoords;
in vec3 Normal;

vec3 lightPosition = vec3(2.0f, -0.5f, 1.0f);

vec4 albedoColor = vec4(0.35f, 0.60f, 0.35f, 1.0f);
vec4 lightColor = vec4(1.0f);

vec4 colorLinear(vec4 colorVector);
vec4 colorSRGB(vec4 colorVector);


void main()
{
    vec3 N = normalize(Normal);
    vec3 L0 = normalize(lightPosition - FragPosition);

    float lambert = max(dot(N, L0), 0.0f);

    vec4 albedoCorrected = colorLinear(albedoColor);
    vec4 lightCorrected = colorLinear(lightColor);

    vec4 diffuse = vec4(albedoCorrected.rgb * lightCorrected.rgb * lambert, 1.0f);
    diffuse.rgb += vec3(0.003);

    colorOutput = colorSRGB(diffuse);
}


vec4 colorLinear(vec4 colorVector) {
  vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));

  return vec4(linearColor, colorVector.a);
}


vec4 colorSRGB(vec4 colorVector) {
  vec3 srgbColor = pow(colorVector.rgb, vec3(1.0f / 2.2f));

  return vec4(srgbColor, colorVector.a);
}