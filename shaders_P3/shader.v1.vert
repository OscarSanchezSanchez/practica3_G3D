#version 330 core

layout(location = 0)in vec3 inPos;	
layout(location = 1)in vec3 inColor;
layout(location = 3)in vec2 inTexCoord;
layout(location = 2)in vec3 inNormal;

//Uniform light
uniform vec3 lightPosition;
uniform vec3 lightIntensity;

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;

out vec3 color;
out vec3 pos;
out vec3 norm;
out vec2 texCoord;

//Out light
out vec3 lPosition;
out vec3 lIntensity;

void main()
{
	color = inColor;
	texCoord = inTexCoord;
	lPosition = lightPosition;
	lIntensity = lightIntensity;
	norm = (normal * vec4(inNormal, 0.0)).xyz;
	pos = (modelView * vec4(inPos, 1.0)).xyz;
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}
