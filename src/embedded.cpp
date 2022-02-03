
/* This file is overwritten at each compilation.
 * Do not modify, see "embedded.hpp" or "_comp.py" instead. */


/* Content of "shaders/blocks/blocks.vert". */
extern const char g_shader_source_bocks_vert[] = "\n#version 430 core\n\nlayout(location = 0) in vec3 in_coords;\nlayout(location = 1) in vec3 in_color;\n\nlayout(location = 0) uniform mat4 camera;\n\nout vec3 v_color;\n\nvoid main()\n{\n\tgl_Position = camera * vec4(in_coords, 1.0);\n\tv_color = in_color;\n}\n";

/* Content of "shaders/blocks/blocks.frag". */
extern const char g_shader_source_bocks_frag[] = "\n#version 430 core\n\nin vec3 v_color;\n\nout vec4 out_color;\n\nvoid main()\n{\n\tout_color = vec4(v_color, 1.0);\n}\n";
