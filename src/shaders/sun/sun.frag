
#version 430 core

in vec2 v_atlas_coords;

layout(location = 1) uniform sampler2D u_atlas;

void main()
{
	vec4 out_color = texture(u_atlas, v_atlas_coords);
	if (out_color.a < 0.001)
	{
		discard;
	}
}
