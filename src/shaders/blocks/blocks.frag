
#version 430 core

in vec2 v_atlas_coords;
in vec2 v_atlas_coords_min;
in vec2 v_atlas_coords_max;
in vec3 v_normal;
in vec3 v_sun_coords;

layout(location = 1) uniform sampler2D u_atlas;
layout(location = 6) uniform float u_atlas_side;
layout(location = 3) uniform sampler2D u_sun_depth;
layout(location = 4) uniform vec3 u_sun_direction;

out vec4 out_color;

void main()
{
	// Clamp atlas coords in the assigned texture to stop bleeding.
	float texel_side = (1.0 / u_atlas_side) / 2.0;
	vec2 atlas_coords = clamp(v_atlas_coords,
		min(v_atlas_coords_min, v_atlas_coords_max) + vec2(1.0, 1.0) * texel_side,
		max(v_atlas_coords_min, v_atlas_coords_max) - vec2(1.0, 1.0) * texel_side);

	out_color = texture(u_atlas, atlas_coords);
	if (out_color.a < 0.001)
	{
		discard;
	}


	float light = -dot(v_normal, normalize(u_sun_direction));
	bool shadow = v_sun_coords.z > texture(u_sun_depth, v_sun_coords.xy).r;
	if (shadow || light < 0)
	{
		light = 0;
	}
	const float shadow_ratio = 0.7;
	out_color.rgb *= light * shadow_ratio + (1.0 - shadow_ratio);
}
