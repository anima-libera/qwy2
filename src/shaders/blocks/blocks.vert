
#version 430 core

layout(location = 0) in vec3 in_coords;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_atlas_coords;
layout(location = 3) in vec2 in_atlas_coords_min;
layout(location = 4) in vec2 in_atlas_coords_max;

layout(location = 0) uniform mat4 camera;
layout(location = 5) uniform vec3 camera_direction;
layout(location = 2) uniform mat4 sun_camera;

out vec2 v_atlas_coords;
out vec2 v_atlas_coords_min;
out vec2 v_atlas_coords_max;
out vec3 v_normal;
out vec3 v_sun_coords;

void main()
{
	gl_Position = camera * vec4(in_coords, 1.0);

	v_atlas_coords = in_atlas_coords;
	v_atlas_coords_min = in_atlas_coords_min;
	v_atlas_coords_max = in_atlas_coords_max;

	v_normal = in_normal;
	
	vec4 sun_coords = sun_camera * vec4(in_coords, 1.0);
	v_sun_coords = sun_coords.xyz / sun_coords.w; //uhu..
	v_sun_coords.xyz = (v_sun_coords.xyz + 1.0) / 2.0;
}
