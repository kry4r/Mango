#version 400 core

in vec3 TexCoords;

out vec4 color;

uniform samplerCube cubemap;


void main()
{
    //color = vec4(pow(texture(cubemap, TexCoords).rgb, vec3(1.0/2.2)), 1.0f);
    color = vec4(0.10f, 0.10f, 0.10f, 1.0f);
}