#version 410 core
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4	MVP;
uniform mat4	MV;
uniform mat3	matNormal;
out vec3	vNormal;
out vec4	vPosEye;
void main()
{
	vPosEye = MV*aPosition;
	vNormal = normalize(matNormal*aNormal);
	gl_Position = MVP*aPosition;
}
