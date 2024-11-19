#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform bool light_up;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 tex_color = texture(sampler0, texcoord);

    if (light_up) {
        vec4 glow_color = vec4(1.0, 0, 0, 1.0);
        color = mix(tex_color * vec4(fcolor, 1.0), glow_color, 0.5); // Blend normal colour with the glow
    } else {
        color = tex_color * vec4(fcolor, 1.0);
    }
}
