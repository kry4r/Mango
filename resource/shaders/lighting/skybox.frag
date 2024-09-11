#version 400 core

in vec3 envMapCoords;
out vec4 colorOutput;

float PI  = 3.14159265359f;
float middleGrey = 0.18f;

uniform sampler2D envMap;
uniform float cameraAperture;
uniform float cameraShutterSpeed;
uniform float cameraISO;

vec2 getSphericalCoord(vec3 normalCoord);
vec3 colorLinear(vec3 colorVector);
vec3 colorSRGB(vec3 colorVector);
vec3 ReinhardTM(vec3 color);
float computeSOBExposure(float aperture, float shutterSpeed, float iso);


void main()
{
    vec3 envColor = texture(envMap, getSphericalCoord(normalize(envMapCoords))).rgb;

    envColor *= computeSOBExposure(cameraAperture, cameraShutterSpeed, cameraISO);
    envColor = ReinhardTM(envColor);

    colorOutput = vec4(colorSRGB(envColor), 1.0f);
//    colorOutput = vec4(0.15f, 0.15f, 0.15f, 1.0f);
}



vec2 getSphericalCoord(vec3 normalCoord)
{
  float phi = acos(-normalCoord.y);
  float theta = atan(1.0f * normalCoord.x, -normalCoord.z) + PI;

  return vec2(theta / (2.0f * PI), phi / PI);
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


float computeSOBExposure(float aperture, float shutterSpeed, float iso)
{
    float lAvg = (1000.0f / 65.0f) * sqrt(aperture) / (iso * shutterSpeed);

    return middleGrey / lAvg;
}