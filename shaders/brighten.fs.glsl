#version 330 core
in vec4 fragColor;
out vec4 outColor;

uniform vec4 fcolor; // White with desired opacity for brightening

void main()
{
    // Output the brighten color. 'fcolor' should be white with low alpha.
    outColor = fcolor;
}