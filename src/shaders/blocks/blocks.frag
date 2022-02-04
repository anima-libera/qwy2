
#version 430 core

in vec2 v_atlas_coords;

layout(location = 1) uniform sampler2D u_atlas;

out vec4 out_color;

void main()
{
	out_color = vec4(texture(u_atlas, v_atlas_coords));
	if (out_color.a < 0.001)
	{
		discard;
	}
}
