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
    vec4 viewPos = view * model * vec4(position, 1.0f);
    worldPos = viewPos.xyz;
    gl_Position = projection * viewPos;
    TexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    normal = normalMatrix * Normal;




//    worldPos = vec3(model * vec4(position, 1.0f));
//    normal = mat3(transpose(inverse(model))) * Normal;
//    TexCoords = texCoords;

//    gl_Position = projection * view * model * vec4(position, 1.0f);
}