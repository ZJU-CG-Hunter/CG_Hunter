#version 460 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  aTangent;
layout (location = 4) in vec3  aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4  aWeights;
layout (location = 7) in int   aNumOfBones;


out vec3 Normal;
out vec3 Position;
out vec2 TexCoords;

const int MAX_BONES = 500;

layout (std140, binding = 2) uniform Matrices
{
  mat4 model;
  mat4 projection;
  mat4 view;
  mat4 bones[MAX_BONES];
};

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
