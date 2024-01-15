#version 400

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    
    float shininess;
}; 

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 1

in vec3 fragPosition;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

vec3 CalcPointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 colorLinear(vec3 colorVector);
vec4 colorSRGB(vec4 colorVector);


void main()
{    
    vec3 result = vec3(0.0f);
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 norm = normalize(Normal);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], material, norm, fragPosition, viewDir);

    color = colorSRGB(vec4(result, 1.0f));
}


vec3 CalcPointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * colorLinear(vec3(texture(mat.texture_diffuse1, TexCoords)));
    vec3 diffuse = light.diffuse * diff * colorLinear(vec3(texture(mat.texture_diffuse1, TexCoords)));
    vec3 specular = light.specular * spec * colorLinear(vec3(texture(mat.texture_specular1, TexCoords)));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}


vec3 colorLinear(vec3 colorVector) {
  vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));

  return vec3(linearColor);
}


vec4 colorSRGB(vec4 colorVector) {
  vec3 srgbColor = pow(colorVector.rgb, vec3(1.0f / 2.2f));

  return vec4(srgbColor, colorVector.a);
}

