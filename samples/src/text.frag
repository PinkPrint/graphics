#version 410 core
in vec2 vTexCoords;
uniform sampler2D tex;
uniform vec4 color;
out vec4    fColor;

void main(void)
{
    fColor = vec4(1, 1, 1, texture(tex, vTexCoords).r) * color;
}
