#version 330 core
layout(location = 0) in vec4 vPosition;
#define	PI	3.14159
void main()
{
	mat4	m;
	float	i = float(gl_InstanceID-1);
	float	theta = i*20.0*PI/180.0;
	m[0][0] = cos(theta);
	m[0][1] = -sin(theta);
	m[1][0] = sin(theta);
	m[1][1] = cos(theta);
	m[2][2] = 1;
	m[3][3] = 1;
	m[3][0] = i*0.7;
	gl_Position = m*vPosition;
}
