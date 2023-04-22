
#version 430 core

in vec3 v_normal;
in vec3 v_sun_camera_space_coords;
in vec3 v_coords;
in vec3 v_color;

layout(location =  3) uniform sampler2D u_shadow_depth;
layout(location =  4) uniform vec3 u_sun_camera_direction;
layout(location =  7) uniform vec3 u_user_coords;
layout(location =  8) uniform vec3 u_fog_color;
layout(location =  9) uniform float u_fog_distance_inf;
layout(location = 10) uniform float u_fog_distance_sup;

out vec4 out_color;

void main()
{
	out_color = vec4(v_color, 1.0);

	/* Shadow calculation and effect. */
	/* TODO: Make `shadow_ratio` a parameter. */
	float light = -dot(v_normal, normalize(u_sun_camera_direction));
	const float shadow_depth = texture(u_shadow_depth, v_sun_camera_space_coords.xy).r;
	const bool is_in_shadow = v_sun_camera_space_coords.z > shadow_depth;
	if (is_in_shadow || light < 0.0)
	{
		light *= 0.0;
	}
	const float shadow_ratio = 0.7; /* How dark is it in the shadows. */
	out_color.rgb *= light * shadow_ratio + (1.0 - shadow_ratio);

	/* Sun gold-ish color. */
	/* TODO: Make `sun_light_color` a parameter. */
	const vec3 sun_light_color = vec3(0.5, 0.35, 0.0);
	out_color.rgb = mix(out_color.rgb,
		out_color.rgb * (vec3(1.0, 1.0, 1.0) + sun_light_color),
		light);

	/* Fog effect. */
	const float distance_to_user = distance(v_coords, u_user_coords);
	//const float fog_ratio =
	//	(clamp(distance_to_user, u_fog_distance_inf, u_fog_distance_sup) - u_fog_distance_inf)
	//	/ (u_fog_distance_sup - u_fog_distance_inf);
	const float fog_ratio = smoothstep(u_fog_distance_inf, u_fog_distance_sup, distance_to_user);
	out_color.rgb = mix(out_color.rgb, u_fog_color, fog_ratio);
}
