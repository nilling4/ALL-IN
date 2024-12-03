#version 330 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color; // Use vec4 if passing alpha

uniform mat3 transform;
uniform mat3 projection;

out vec4 fragColor;

void main()
{
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
    fragColor = in_color;
}