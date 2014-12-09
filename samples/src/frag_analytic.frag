#version 330 core
out vec4 fColor;
in vec2	coord;
uniform vec2	center;
#define	PI	3.14159
void main()
{
	float	intensity = 0.5*(cos(length(center - coord)*3.0*PI) + 1.0);
	fColor = vec4(intensity*vec3(1.0, 1.0, 1.0), 1.0);
}

