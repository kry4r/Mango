#version 400

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;

    float shininess;
};

struct LightObject {
    vec3 position;
    vec4 color;
};


uniform int lightCounter = 3;
uniform LightObject lightArray[3];
uniform Material material;

out vec4 colorOutput;

in vec3 FragPosition;
in vec2 TexCoords;
in vec3 Normal;

vec4 albedoColor = vec4(1.0f);


vec4 colorLinear(vec4 colorVector);
vec4 colorSRGB(vec4 colorVector);


void main()
{
    vec4 diffuse = vec4(0.0f);

    for (int i = 0; i < lightCounter; i++)
    {
        vec3 N = normalize(Normal);
        vec3 L = normalize(lightArray[i].position - FragPosition);

        float lambert = max(dot(N, L), 0.0f);

        vec4 albedoCorrected = colorLinear(albedoColor);
        vec4 lightCorrected = colorLinear(lightArray[i].color);

        diffuse += vec4(albedoCorrected.rgb * lightCorrected.rgb * lambert, 1.0f);
    }
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