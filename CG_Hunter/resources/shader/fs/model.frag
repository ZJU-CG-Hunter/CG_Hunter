#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;


void main()
{    
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    float rate = 0.6;
    FragColor = vec4(rate*texture(skybox, R).rgb+(1.0-rate)*texture(texture_diffuse1, TexCoords).rgb , 1.0);
}