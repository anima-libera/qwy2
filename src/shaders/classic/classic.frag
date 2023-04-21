
#version 430 core

in vec2 v_atlas_coords;
in vec2 v_atlas_coords_min;
in vec2 v_atlas_coords_max;
in vec3 v_normal;
in vec3 v_sun_camera_space_coords;
in vec3 v_coords;
in float v_ambient_occlusion;

layout(location =  1) uniform sampler2D u_atlas;
layout(location =  6) uniform float u_atlas_side;
layout(location =  3) uniform sampler2D u_shadow_depth;
layout(location =  4) uniform vec3 u_sun_camera_direction;
layout(location =  7) uniform vec3 u_user_coords;
layout(location =  8) uniform vec3 u_fog_color;
layout(location =  9) uniform float u_fog_distance_inf;
layout(location = 10) uniform float u_fog_distance_sup;

out vec4 out_color;

void main()
{
	/* Clamp atlas coords in the assigned texture to stop bleeding. */
	/* TODO: Do it in the mesh construction! Is it possible tho ? */
	const float texel_side = (1.0 / u_atlas_side) / 2.0;
	const vec2 atlas_coords = clamp(v_atlas_coords,
		v_atlas_coords_min + vec2(1.0, 1.0) * texel_side,
		v_atlas_coords_max - vec2(1.0, 1.0) * texel_side);

	out_color = texture(u_atlas, atlas_coords);
	if (out_color.a < 0.001)
	{
		discard;
	}

	/* Shadow calculation and effect. */
	/* TODO: Make `shadow_ratio` a parameter. */
	/* TODO: Make `ao_ratio_max` a parameter. */
	float light = -dot(v_normal, normalize(u_sun_camera_direction));
	const float shadow_depth = texture(u_shadow_depth, v_sun_camera_space_coords.xy).r;
	const bool is_in_shadow = v_sun_camera_space_coords.z > shadow_depth;
	if (is_in_shadow || light < 0.0)
	{
		light *= 0.0;
	}
	const float shadow_ratio = 0.7; /* How dark is it in the shadows. */
	out_color.rgb *= light * shadow_ratio + (1.0 - shadow_ratio);
	const float ao_ratio_max = 0.7; /* How dark is it in corners (ambiant occlusion). */
	const float ao_ratio = ao_ratio_max / (light + 1.0); 
	out_color.rgb *= v_ambient_occlusion * ao_ratio + (1.0 - ao_ratio);

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
