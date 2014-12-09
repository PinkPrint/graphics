#version 330 core
out vec4 fColor;
in vec4 vPosEye;
in vec3	vNormal;
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
void main()
{
	vec3	n = normalize(vNormal);
	vec3	l;
	if(light.position.w == 1.0)
		l = normalize((V*light.position - vPosEye).xyz);
	else
		l = normalize((V*light.position).xyz);
	vec3	v = normalize(-vPosEye.xyz);
	vec3	r = reflect(-l, n);
	float	l_dot_n = max(dot(l, n), 0.0);
	vec3	ambient = light.ambient * material.ambient;
	vec3	diffuse = light.diffuse * material.diffuse * l_dot_n;
	vec3	specular = vec3(0.0);
	if(l_dot_n > 0.0)
	{
		specular = light.specular * material.specular * pow(max(dot(r, v), 0.0), material.shininess);
	}
	fColor = vec4(ambient + diffuse + specular, 1);
}


