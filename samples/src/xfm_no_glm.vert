#version 330 core
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;
uniform mat4	P;
uniform mat4	T;
uniform mat4	Rx;
uniform mat4	Ry;
out vec4 color;
void main()
{
	gl_Position = P*T*Rx*Ry*vPosition;	// very inefficient!!! You should mutiply all the matrices first
										// on the client-side and pass the resulting matrix to the vertex shader.
	color = vColor;
}
