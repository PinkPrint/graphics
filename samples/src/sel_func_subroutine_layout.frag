#version 430 core
out vec4	fColor;
subroutine vec4 type_func();

layout(index = 7) subroutine (type_func) vec4 func_first()
{
	return vec4(0,0,1,1);
}

layout(index = 10) subroutine (type_func) vec4 func_second()
{
	return vec4(1,0,0,1);
}

subroutine uniform type_func func;

void main()
{
	fColor = func();
}

