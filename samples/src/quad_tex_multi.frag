#version 410 core
in vec2 texcoord;
out vec4 fColor;
uniform sampler2D tex1; 
uniform sampler2D tex2; 
void main()
{
	fColor = mix(texture(tex1, texcoord), texture(tex2, texcoord), 0.5);
}

