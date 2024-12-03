#version 330 core
in vec4 fragColor;
out vec4 outColor;

uniform vec4 fcolor; // Should be vec4 if you're passing a vec4 from C++

void main()
{
    outColor = fcolor;
}