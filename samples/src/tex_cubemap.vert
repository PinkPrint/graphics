#version 410 core
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
out vec3 vRefEye;
uniform mat4	MVP;
uniform mat4	MV;
uniform mat3	M_normal;
void main()
{
	vec3	n = M_normal*aNormal;
	vec3	v = normalize(-(MV*aPosition).xyz);
	vRefEye = reflect(-v, n);
	gl_Position = MVP*aPosition;
}
	