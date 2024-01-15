#version 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec2 TexCoords;
out vec3 normal;
out vec3 baseColor;


void main()
{
    worldPos = vec3(model * vec4(position, 1.0f));
    normal = mat3(transpose(inverse(model))) * Normal;
    TexCoords = texCoords;
    baseColor = position;

    gl_Position = projection * view * model * vec4(position, 1.0f);
}