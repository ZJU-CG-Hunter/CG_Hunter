#version 330 core
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

const int MAX_BONES = 200;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bone[MAX_BONES];

void main()
{
    mat4 BoneTransform = mat4(1.0f);
    if(aNumOfBones > 0){
      BoneTransform = mat4(0.0f);
      for(int i = 0; i < aNumOfBones; i++){
        BoneTransform += bone[aBoneIDs[i]] * aWeights[i];
      }
    }

    Normal = mat3(transpose(inverse(model * BoneTransform))) * aNormal;
    Position = vec3(model * BoneTransform * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * BoneTransform * vec4(aPos, 1.0);
}
