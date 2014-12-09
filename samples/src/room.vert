#version 330 core
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;
uniform mat4	MVP;
out	vec4		color;
void main()
{
	gl_Position = MVP*vPosition;	
	color = vec4(vColor,1);
}