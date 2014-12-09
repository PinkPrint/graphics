#version 410 core
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
uniform TMaterial	material[2];
uniform TLight		light;
void main()
{
	int	idx;
	vec3	n = normalize(vNormal);
	if(gl_FrontFacing)
	{
		idx = 0;
	}
	else
	{
		idx = 1;
		n = -n;
	}
	vec3	l;
	if(light.position.w == 1.0)
		l = normalize((V*light.position - vPosEye).xyz);		// positional light
	else
		l = normalize((V*light.position).xyz);	// directional light
	vec3	v = normalize(-vPosEye.xyz);
//	vec3	v = vec3(0,0,1);
	vec3	h = normalize(l + v);
	float	l_dot_n = max(dot(l, n), 0.0);
	vec3	ambient = light.ambient * material[idx].ambient;
	vec3	diffuse = light.diffuse * material[idx].diffuse * l_dot_n;
	vec3	specular = vec3(0.0);
	if(l_dot_n > 0.0)
	{
		specular = light.specular * material[idx].specular * pow(max(dot(h, n), 0.0), material[idx].shininess);
	}
	fColor = vec4(ambient + diffuse + specular, 1);
}


