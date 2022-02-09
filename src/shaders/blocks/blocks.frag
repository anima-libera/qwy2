
#version 430 core

in vec2 v_atlas_coords;
in vec3 v_normal;
in vec3 v_sun_coords;

layout(location = 1) uniform sampler2D u_atlas;
layout(location = 3) uniform sampler2D u_sun_depth;
layout(location = 4) uniform vec3 u_sun_direction;

out vec4 out_color;

void main()
{
	out_color = texture(u_atlas, v_atlas_coords);
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
