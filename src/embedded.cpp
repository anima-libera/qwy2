
/* This file is overwritten at each compilation.
 * Do not modify, see "embedded.hpp" or "_comp.py" instead. */


/* Content of "shaders/blocks/blocks.vert". */
extern const char g_shader_source_bocks_vert[] = "\n#version 430 core\n\nlayout(location = 0) in vec3 in_coords;\nlayout(location = 1) in vec2 in_atlas_coords;\n\nlayout(location = 0) uniform mat4 camera;\n\nout vec2 v_atlas_coords;\n\nvoid main()\n{\n\tgl_Position = camera * vec4(in_coords, 1.0);\n\tv_atlas_coords = in_atlas_coords;\n}\n";

/* Content of "shaders/blocks/blocks.frag". */
extern const char g_shader_source_bocks_frag[] = "\n#version 430 core\n\nin vec2 v_atlas_coords;\n\nlayout(location = 1) uniform sampler2D u_atlas;\n\nout vec4 out_color;\n\nvoid main()\n{\n\tout_color = vec4(texture(u_atlas, v_atlas_coords));\n\tif (out_color.a < 0.001)\n\t{\n\t\tdiscard;\n\t}\n}\n";
