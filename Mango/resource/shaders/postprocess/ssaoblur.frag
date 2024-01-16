#version 400 core

in vec2 TexCoords;
out float ssaoBlurOutput;

uniform sampler2D ssaoInput;
uniform int ssaoBlurSize;


void main()
{
   vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
   float result = 0.0;

   for (int x = 0; x < ssaoBlurSize; ++x)
   {
      for (int y = 0; y < ssaoBlurSize; ++y)
      {
         vec2 offset = (vec2(-2.0) + vec2(float(x), float(y))) * texelSize;
         result += texture(ssaoInput, TexCoords + offset).r;
      }
   }

   ssaoBlurOutput = result / float(ssaoBlurSize * ssaoBlurSize);
}