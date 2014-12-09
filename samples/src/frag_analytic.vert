#version 330 core
layout(location = 0) in vec4 vPosition;
out vec2	coord;
void main()
{
	gl_Position = vPosition;
	coord = vPosition.xy;
}
