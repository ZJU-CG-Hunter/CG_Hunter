#version 460 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  aTangent;
layout (location = 4) in vec3  aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4  aWeights;
layout (location = 7) in int   aNumOfBones;

layout (std140) uniform Matrices
{
  mat4 projection;
  mat4 view;
};

uniform mat4 model[200];
uniform mat4 lightSpaceMatrix;

void main()
{
  gl_Position = lightSpaceMatrix * model[gl_InstanceID] * vec4(aPos, 1.0);
}

