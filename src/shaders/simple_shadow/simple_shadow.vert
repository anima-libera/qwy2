
#version 430 core

layout(location = 0) in vec3 in_coords;

layout(location = 0) uniform mat4 sun_camera;

void main()
{
	gl_Position = sun_camera * vec4(in_coords, 1.0);
}
