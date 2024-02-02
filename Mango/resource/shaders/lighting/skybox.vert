#version 400 core

in vec2 position;
out vec3 envMapCoords;

uniform mat4 inverseView;
uniform mat4 inverseProj;


void main()
{
    vec4 unprojCoords = (inverseProj * vec4(position, vec2(1.0f)));
    envMapCoords = (inverseView * unprojCoords).xyz;

    gl_Position = vec4(position, vec2(1.0f));
}