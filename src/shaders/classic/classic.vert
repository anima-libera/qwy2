
#version 430 core

layout(location = 0) in vec3 in_coords;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_atlas_coords;
layout(location = 3) in vec2 in_atlas_coords_min;
layout(location = 4) in vec2 in_atlas_coords_max;
layout(location = 5) in float in_ambient_occlusion;

layout(location = 0) uniform mat4 u_user_camera;
layout(location = 5) uniform vec3 u_user_camera_direction;
layout(location = 2) uniform mat4 u_sun_camera;

out vec2 v_atlas_coords;
out vec2 v_atlas_coords_min;
out vec2 v_atlas_coords_max;
out vec3 v_normal;
out vec3 v_sun_camera_space_coords;
out vec3 v_coords;
out float v_ambient_occlusion;

void main()
{
	gl_Position = u_user_camera * vec4(in_coords, 1.0);

	v_atlas_coords = in_atlas_coords;
	v_atlas_coords_min = in_atlas_coords_min;
	v_atlas_coords_max = in_atlas_coords_max;

	v_normal = in_normal;
	
	/* Coords of the vertex in the sun camera space,
	 * and then in the shadow depth buffer space (0.0 ~ 1.0 instead of -1.0 ~ +1.0),
	 * to compare the fragment depths to their shadow depth buffer analog. */
	vec4 sun_coords = u_sun_camera * vec4(in_coords, 1.0);
	v_sun_camera_space_coords = sun_coords.xyz / sun_coords.w; //uhu..
	v_sun_camera_space_coords.xyz = (v_sun_camera_space_coords.xyz + 1.0) / 2.0;

	v_coords = in_coords;

	v_ambient_occlusion = in_ambient_occlusion;
}
