
#version 430 core

layout(location = 0) in vec3 in_coords;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_atlas_coords;

layout(location = 0) uniform mat4 camera;
layout(location = 2) uniform mat4 sun_camera;

out vec2 v_atlas_coords;
out vec3 v_normal;
out vec3 v_sun_coords;

void main()
{
	gl_Position = camera * vec4(in_coords, 1.0);
	v_atlas_coords = in_atlas_coords;
	v_normal = in_normal;
	vec4 sun_coords = sun_camera * vec4(in_coords, 1.0);
	v_sun_coords = sun_coords.xyz / sun_coords.w; //uhu..
	v_sun_coords = (v_sun_coords + 1.0) / 2.0;
}
