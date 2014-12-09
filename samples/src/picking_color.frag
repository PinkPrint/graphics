#version 330 core
out vec4	fColor;
uniform int id;
void main()
{
	fColor = vec4(float(id)/255.0,1,1,1);
}

