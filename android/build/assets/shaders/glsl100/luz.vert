#version 100
precision mediump float;


attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec2 vertexTexCoord2;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform vec3 matNormal;


varying vec3 FragPos;
varying vec3 Normal;
varying vec2 fragTexCoord;
varying vec2 fragTexCoord2;
varying vec4 fragColor;


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
