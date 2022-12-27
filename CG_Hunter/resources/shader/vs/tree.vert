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

out VS_OUT {
  out vec3 Normal;
  out vec3 Position;
  out vec2 TexCoords;
  out vec3 FragPos;
  out vec4 FragPosLightSpace;
}vs_out;


layout (std140) uniform Matrices
{
  mat4 projection;
  mat4 view;
};

uniform mat4 lightSpaceMatrix;
uniform mat4 model[200];


void main()
{
  vs_out.Normal = mat3(transpose(inverse(model[gl_InstanceID]))) * aNormal;
  vs_out.Position = vec3(model[gl_InstanceID] * vec4(aPos, 1.0));
  vs_out.TexCoords = aTexCoords;
  vs_out.FragPos = vec3(model[gl_InstanceID]* vec4(aPos, 1.0));
  vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

  gl_Position = projection * view * model[gl_InstanceID]* vec4(aPos, 1.0);
}
