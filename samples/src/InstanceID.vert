#version 330 core
layout(location = 0) in vec4 vPosition;
uniform samplerBuffer	tex_color;
uniform samplerBuffer	tex_offset;
out vec4	color;
void main()
{
	gl_Position = vPosition + vec4(texelFetch(tex_offset, gl_InstanceID).xy, 0, 1);
	color = texelFetch(tex_color, gl_InstanceID).rgba;

}
