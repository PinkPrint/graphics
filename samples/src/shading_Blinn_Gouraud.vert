#version 410 core
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4	MVP;
uniform mat4	MV;
uniform mat4	V;
uniform mat3	matNormal;
struct TMaterial
{
	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
	vec3	emission;
	float	shininess;
};
struct TLight
{
	vec4	position;
	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
};
uniform TMaterial	material;
uniform TLight		light;
out vec3	vColor;
void main()
{
	vec3	n = normalize(matNormal*aNormal);
	vec4	vPosEye = MV*aPosition;
	vec3	l;
	if(light.position.w == 1.0)
		l = normalize((V*light.position - vPosEye).xyz);
	else
		l = normalize((V*light.position).xyz);
	vec3	v = normalize(-vPosEye.xyz);
	vec3	h = normalize(l + v);
	float	l_dot_n = max(dot(l, n), 0.0);
	vec3	ambient = light.ambient * material.ambient;
	vec3	diffuse = light.diffuse * material.diffuse * l_dot_n;
	vec3	specular = vec3(0.0);
	if(l_dot_n > 0.0)
	{
		specular = light.specular * material.specular * pow(max(dot(h, n), 0.0), material.shininess);
	}
	vColor = ambient + diffuse + specular;
	gl_Position = MVP*aPosition;
}
