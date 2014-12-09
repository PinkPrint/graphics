#version 410 core
in vec3 vRefEye;
out vec4 fColor;
uniform samplerCube tex;
void main()
{
	fColor = texture(tex, vRefEye);
}

