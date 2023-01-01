#version 460 core
out vec4 FragColor;

uniform vec3 cameraPos;
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;
uniform samplerCube skybox;

in VS_OUT {
  in vec3 Normal;
  in vec3 Position;
  in vec2 TexCoords;
  in vec3 FragPos;
  in vec4 FragPosLightSpace;
}fs_in;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{    
  // ambient
  float ambientStength = 0.3;
  vec3 ambient = ambientStength * lightColor;

  // diffuse
  vec3 norm = normalize(fs_in.Normal);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular
  float specularStrength = 0.5;
  vec3 viewDir = normalize(cameraPos - fs_in.Position);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColor;

  // object color
  vec3 I = -viewDir;
  vec3 R = reflect(I, norm);
  float rate = 0.3;
  vec3 objectcolor = rate*texture(skybox, R).rgb+(1.0-rate)*texture(texture_diffuse1, fs_in.TexCoords).rgb;

  // calculate shadow
  float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * objectcolor;  //
  

  //FragColor = texture(texture_diffuse1, TexCoords);
  //FragColor =  vec4((ambient + diffuse + specular) * objectcolor, 1.0);
  FragColor = vec4(lighting, 1.0);

}