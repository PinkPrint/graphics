#version 400 core
out vec4	fColor;
subroutine vec4 type_func();

subroutine (type_func) vec4 func_first()
{
	return vec4(0,0,1,1);
}

subroutine (type_func) vec4 func_second()
{
	return vec4(1,0,0,1);
}

subroutine uniform type_func func;

void main()
{
	fColor = func();
}

