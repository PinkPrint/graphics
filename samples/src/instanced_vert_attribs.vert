#version 330 core
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in float offset_y;
out vec4	color;
void main()
{
	gl_Position = vPosition + vec4(0.25*float(gl_InstanceID-3), offset_y, 0, 0);
//	gl_Position = vPosition;
	color = vColor;

}
