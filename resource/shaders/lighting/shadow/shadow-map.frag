#version 400 core

void main()
{
    // We only need the depth value for the shadow map
    gl_FragDepth = gl_FragCoord.z;
}