#version 400 core

in vec2 TexCoords;
out vec4 colorOutput;

float middleGrey = 0.18f;

uniform sampler2D screenTexture;
uniform sampler2D ssao;

uniform int gBufferView;
uniform bool ssaoMode;
uniform bool fxaaMode;
uniform float cameraAperture;
uniform float cameraShutterSpeed;
uniform float cameraISO;
uniform vec2 screenTextureSize;

float FXAA_SPAN_MAX = 8.0f;
float FXAA_REDUCE_MUL = 1.0f/8.0f;
float FXAA_REDUCE_MIN = 1.0f/128.0f;

vec3 colorLinear(vec3 colorVector);
vec3 colorSRGB(vec3 colorVector);
vec3 ReinhardTM(vec3 color);
float computeSOBExposure(float aperture, float shutterSpeed, float iso);
vec3 computeFxaa();


void main()
{
    vec3 color;

    if(gBufferView == 1)
    {
        if(fxaaMode)
            color = computeFxaa();  // Don't know if applying FXAA first is a good idea, especially with effects such as motion blur and DoF...
        else
            color = texture(screenTexture, TexCoords).rgb;

        if(ssaoMode)
        {
            float ssao = texture(ssao, TexCoords).r;
            color *= ssao;
        }

        color *= computeSOBExposure(cameraAperture, cameraShutterSpeed, cameraISO);
        color = ReinhardTM(color);

        colorOutput = vec4(colorSRGB(color), 1.0f);
    }

    else    // No tonemapping or linear/sRGB conversion if we want to visualize the different buffers
    {
        color = texture(screenTexture, TexCoords).rgb;
        colorOutput = vec4(color, 1.0f);
    }
}



vec3 computeFxaa()
{
    vec2 screenTextureOffset = screenTextureSize;
    vec3 luma = vec3(0.299f, 0.587f, 0.114f);

    vec3 offsetNW = texture(screenTexture, TexCoords.xy + (vec2(-1.0f, -1.0f) * screenTextureOffset)).xyz;
    vec3 offsetNE = texture(screenTexture, TexCoords.xy + (vec2(1.0f, -1.0f) * screenTextureOffset)).xyz;
    vec3 offsetSW = texture(screenTexture, TexCoords.xy + (vec2(-1.0f, 1.0f) * screenTextureOffset)).xyz;
    vec3 offsetSE = texture(screenTexture, TexCoords.xy + (vec2(1.0f, 1.0f) * screenTextureOffset)).xyz;
    vec3 offsetM  = texture(screenTexture, TexCoords.xy).xyz;

    float lumaNW = dot(luma, offsetNW);
    float lumaNE = dot(luma, offsetNE);
    float lumaSW = dot(luma, offsetSW);
    float lumaSE = dot(luma, offsetSE);
    float lumaM  = dot(luma, offsetNW);

    vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)),
                     ((lumaNW + lumaSW) - (lumaNE + lumaSE)));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (FXAA_REDUCE_MUL * 0.25f), FXAA_REDUCE_MIN);
    float dirCorrection = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * dirCorrection)) * screenTextureOffset;

    vec3 resultA = 0.5f * (texture(screenTexture, TexCoords.xy + (dir * vec2(1.0f / 3.0f - 0.5f))).xyz +
                                    texture(screenTexture, TexCoords.xy + (dir * vec2(2.0f / 3.0f - 0.5f))).xyz);

    vec3 resultB = resultA * 0.5f + 0.25f * (texture(screenTexture, TexCoords.xy + (dir * vec2(0.0f / 3.0f - 0.5f))).xyz +
                                             texture(screenTexture, TexCoords.xy + (dir * vec2(3.0f / 3.0f - 0.5f))).xyz);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float lumaResultB = dot(luma, resultB);

    if(lumaResultB < lumaMin || lumaResultB > lumaMax)
        return vec3(resultA);
    else
        return vec3(resultB);
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