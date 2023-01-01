#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140) uniform Matrices
{
  mat4 projection;
  mat4 view;
};

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos.x, -aPos.y, aPos.z,  1.0);
    gl_Position = pos.xyww;
}  