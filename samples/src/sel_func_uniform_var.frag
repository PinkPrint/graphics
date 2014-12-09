#version 330 core
uniform int idx_func;

out vec4 fColor;

vec4 func0(void)
{
	return vec4(0, 0, 1, 1);
}
vec4 func1(void)
{
	return vec4(1, 0, 0, 1);
}
void main()
{
	if(idx_func==0)
		fColor = func0();
	else
		fColor = func1();
}

