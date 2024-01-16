#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 texCoords;

out vec3 worldPos;
out vec2 TexCoords;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{

}