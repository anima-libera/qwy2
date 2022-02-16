
#version 430 core

layout(location = 0) in vec3 in_coords;
layout(location = 1) in vec3 in_color;

layout(location = 0) uniform mat4 user_camera;

out vec3 v_color;

void main()
{
	gl_Position = user_camera * vec4(in_coords, 1.0);

	v_color = in_color;
}
