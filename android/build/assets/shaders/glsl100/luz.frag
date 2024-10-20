#version 100
precision mediump float;

varying vec3 Normal;
varying vec3 FragPos;
varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform sampler2D texture0;
uniform vec3 viewPos;

void main()
{
  float ambientStrength = 0.5;
  float specularStrength = 0.5;

  
  vec3 ambient = ambientStrength * lightColor;

  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);

  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = specularStrength * spec * lightColor;


  vec4 texelColor = texture2D(texture0, fragTexCoord)*fragColor;

  vec3 result = (ambient + diffuse + specular) * texelColor.rgb;
  gl_FragColor = vec4(result, 1.0);
} 
