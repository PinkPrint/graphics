#version 410 core
#define	LIGHT_TYPE_DIRECTIONAL	0
#define	LIGHT_TYPE_POSITIONAL	1
#define	LIGHT_TYPE_SPOT			2
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
struct TSpot
{
	float	cutoff;
	float	exponent;
	vec3	direction;
};
struct TAttenuation
{
	float	k0;
	float	k1;
	float	k2;
};
struct TLight
{
	int		type;
	vec4	position;
	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
	TSpot	spot;
	TAttenuation	attenuation;
};
uniform TMaterial	material;
uniform TLight		light;
void main()
{
	vec3	n = normalize(vNormal);
	vec3	l;
	float	distance, attenuation;
	if(light.position.w == 0.0)
	{
		l = normalize((V*light.position).xyz);	// directional light
		attenuation = 1.0;
	}
	else
	{
		l = (V*light.position - vPosEye).xyz;		// positional light
		distance = length(l);
		l = normalize(l);
		attenuation = 1.0/(light.attenuation.k0 + light.attenuation.k1*distance + light.attenuation.k2*distance*distance);
	}
	if(light.type == LIGHT_TYPE_SPOT)
	{
		float	spotCos = dot(-l, normalize(mat3(V)*light.spot.direction));
		if(spotCos < light.spot.cutoff)
		{
			attenuation = 0.0;
		}
		else
		{
			attenuation *= pow(spotCos, light.spot.exponent);
		}
	}
	vec3	v = normalize(-vPosEye.xyz);
	vec3	h = normalize(l + v);
	float	l_dot_n = max(dot(l, n), 0.0);
	vec3	ambient = light.ambient * material.ambient;
	vec3	specular = vec3(0.0);
	vec3	diffuse = vec3(0.0);
	if(l_dot_n > 0.0)
	{
		specular = light.specular * material.specular * pow(max(dot(h, n), 0.0), material.shininess);
		diffuse = light.diffuse * material.diffuse * l_dot_n;
	}
	fColor = vec4(ambient + attenuation*(diffuse + specular), 1);
}


