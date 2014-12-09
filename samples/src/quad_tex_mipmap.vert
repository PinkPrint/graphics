#version 410 core
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;
out vec2 texcoord;
uniform mat4	MVP;
void main()
{
	gl_Position = MVP*vPosition;
	texcoord = vTexCoord;
}
	