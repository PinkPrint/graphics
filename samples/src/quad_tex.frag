#version 330 core
in vec2 texcoord;
out vec4 fColor;
uniform sampler2D tex;
uniform vec4		color;
void main()
{
	fColor = mix(texture(tex, texcoord), color, 0.1);
}

