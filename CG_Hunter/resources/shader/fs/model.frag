#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform sampler2D texture_diffuse1;

uniform samplerCube skybox;



void main()
{    
    // ambient
    float ambientStength = 0.1;
    vec3 ambient = ambientStength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // object color
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    float rate = 0.3;
    vec3 objectcolor = rate*texture(skybox, R).rgb+(1.0-rate)*texture(texture_diffuse1, TexCoords).rgb;
    //FragColor = texture(texture_diffuse1, TexCoords);
    FragColor =  vec4((ambient + diffuse + specular) * objectcolor, 1.0);
}