
#version 430 core

in vec2 v_atlas_coords;
in vec2 v_atlas_coords_min;
in vec2 v_atlas_coords_max;
in vec3 v_normal;
in vec3 v_sun_camera_space_coords;

layout(location = 1) uniform sampler2D u_atlas;
layout(location = 6) uniform float u_atlas_side;
layout(location = 3) uniform sampler2D u_shadow_depth;
layout(location = 4) uniform vec3 u_sun_camera_direction;

out vec4 out_color;

void main()
{
	/* Clamp atlas coords in the assigned texture to stop bleeding. */
	/* TODO: Do it in the mesh construction! */
	float texel_side = (1.0 / u_atlas_side) / 2.0;
	vec2 atlas_coords = clamp(v_atlas_coords,
		v_atlas_coords_min + vec2(1.0, 1.0) * texel_side,
		v_atlas_coords_max - vec2(1.0, 1.0) * texel_side);

	out_color = texture(u_atlas, atlas_coords);
	if (out_color.a < 0.001)
	{
		discard;
	}

	/* Shadow calculation and effect. */
	float light = -dot(v_normal, normalize(u_sun_camera_direction));
	bool shadow = v_sun_camera_space_coords.z > texture(u_shadow_depth, v_sun_camera_space_coords.xy).r;
	if (shadow || light < 0)
	{
		light = 0;
	}
	const float shadow_ratio = 0.7; /* How dark can it get in the shadows. */
	out_color.rgb *= light * shadow_ratio + (1.0 - shadow_ratio);
}
