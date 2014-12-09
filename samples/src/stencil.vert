#version 330 core
layout(location = 0) in vec4 vPosition;
out vec3 color;
void main()
{
	gl_Position = vPosition;
	color = vec3(1,1,1) - vPosition.rgb;
}
