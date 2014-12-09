#version 330 core
in vec3 texcoords;
out vec4 fColor;
uniform samplerCube tex;
void main()
{
	fColor = texture(tex, texcoords);
}

	