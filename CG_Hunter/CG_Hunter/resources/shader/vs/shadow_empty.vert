#version 460 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  aTangent;
layout (location = 4) in vec3  aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4  aWeights;
layout (location = 7) in int   aNumOfBones;

const int MAX_BONES = 1000;

out vec2 TexCoords;


layout (std140) uniform Matrices
{
  mat4 model;
  mat4 projection;
  mat4 view;
  mat4 bones[MAX_BONES];
};

uniform mat4 lightSpaceMatrix;


void main()
{
    mat4 BoneTransform = mat4(1.0f);
    if(aNumOfBones > 0){
      BoneTransform = mat4(0.0f);
      for(int i = 0; i < aNumOfBones; i++){
        BoneTransform += bones[aBoneIDs[i]] * aWeights[i];
      }
    }
    TexCoords = aTexCoords;
    gl_Position = lightSpaceMatrix * model * BoneTransform * vec4(aPos, 1.0);
}

