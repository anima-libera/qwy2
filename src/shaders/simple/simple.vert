
#version 430 core

layout(location = 0) in vec3 in_coords;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;

layout(location = 0) uniform mat4 u_user_camera;
layout(location = 5) uniform vec3 u_user_camera_direction;
layout(location = 2) uniform mat4 u_sun_camera;

out vec3 v_normal;
out vec3 v_sun_camera_space_coords;
out vec3 v_coords;
out vec3 v_color;

void main()
{
	gl_Position = u_user_camera * vec4(in_coords, 1.0);

	v_color = in_color;

	v_normal = in_normal;
	
	/* Coords of the vertex in the sun camera space,
	 * and then in the shadow depth buffer space (0.0 ~ 1.0 instead of -1.0 ~ +1.0),
	 * to compare the fragment depths to their shadow depth buffer analog. */
	vec4 sun_coords = u_sun_camera * vec4(in_coords, 1.0);
	v_sun_camera_space_coords = sun_coords.xyz / sun_coords.w; //uhu..
	v_sun_camera_space_coords.xyz = (v_sun_camera_space_coords.xyz + 1.0) / 2.0;

	v_coords = in_coords;
}
