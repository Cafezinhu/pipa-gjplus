#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec2 vertexTexCoord2;
in vec4 vertexColor;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform vec3 matNormal;


out vec3 FragPos;
out vec3 Normal;
out vec2 fragTexCoord;
out vec2 fragTexCoord2;
out vec4 fragColor;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    gl_Position = mvp*vec4(vertexPosition, 1.0);
    FragPos = vec3(matModel*vec4(vertexPosition, 1.0));  
    Normal =  vertexNormal;
    fragTexCoord = vertexTexCoord;
    fragTexCoord2 = vertexTexCoord2;
    fragColor = vertexColor;
} 
