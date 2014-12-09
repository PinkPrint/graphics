#version 330 core
layout(location = 0) in vec3 aPosition;
out vec3 texcoords;
uniform mat4	MVP;
void main()
{
	gl_Position = MVP*vec4(aPosition,1);
	texcoords = aPosition.xyz;
}
	