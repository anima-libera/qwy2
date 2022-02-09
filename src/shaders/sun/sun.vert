
#version 430 core

layout(location = 0) in vec3 in_coords;
layout(location = 1) in vec2 in_atlas_coords;

layout(location = 0) uniform mat4 camera;

out vec2 v_atlas_coords;

void main()
{
	gl_Position = camera * vec4(in_coords, 1.0);
	v_atlas_coords = in_atlas_coords;
}
