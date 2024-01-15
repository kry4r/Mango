#version 400

in vec3 TexCoords;

out vec4 color;

uniform samplerCube cubemap;


void main()
{
    color = vec4(pow(texture(cubemap, TexCoords).rgb, vec3(1.0/2.2)), 1.0f);
}