
/* This file is overwritten at each compilation.
 * Do not modify, see "embedded.hpp" or "_comp.py" instead. */


/* Content of "shaders/blocks/blocks.vert". */
extern const char g_shader_source_blocks_vert[] = "\n#version 430 core\n\nlayout(location = 0) in vec3 in_coords;\nlayout(location = 1) in vec3 in_normal;\nlayout(location = 2) in vec2 in_atlas_coords;\n\nlayout(location = 0) uniform mat4 camera;\nlayout(location = 2) uniform mat4 sun_camera;\n\nout vec2 v_atlas_coords;\nout vec3 v_normal;\nout vec3 v_sun_coords;\n\nvoid main()\n{\n\tgl_Position = camera * vec4(in_coords, 1.0);\n\tv_atlas_coords = in_atlas_coords;\n\tv_normal = in_normal;\n\tvec4 sun_coords = sun_camera * vec4(in_coords, 1.0);\n\tv_sun_coords = sun_coords.xyz / sun_coords.w; //uhu..\n\tv_sun_coords = (v_sun_coords + 1.0) / 2.0;\n}\n";

/* Content of "shaders/blocks/blocks.frag". */
extern const char g_shader_source_blocks_frag[] = "\n#version 430 core\n\nin vec2 v_atlas_coords;\nin vec3 v_normal;\nin vec3 v_sun_coords;\n\nlayout(location = 1) uniform sampler2D u_atlas;\nlayout(location = 3) uniform sampler2D u_sun_depth;\nlayout(location = 4) uniform vec3 u_sun_direction;\n\nout vec4 out_color;\n\nvoid main()\n{\n\tout_color = texture(u_atlas, v_atlas_coords);\n\tif (out_color.a < 0.001)\n\t{\n\t\tdiscard;\n\t}\n\n\n\tfloat light = -dot(v_normal, normalize(u_sun_direction));\n\tbool shadow = v_sun_coords.z > texture(u_sun_depth, v_sun_coords.xy).r;\n\tif (shadow || light < 0)\n\t{\n\t\tlight = 0;\n\t}\n\tconst float shadow_ratio = 0.7;\n\tout_color.rgb *= light * shadow_ratio + (1.0 - shadow_ratio);\n}\n";

/* Content of "shaders/sun/sun.vert". */
extern const char g_shader_source_sun_vert[] = "\n#version 430 core\n\nlayout(location = 0) in vec3 in_coords;\nlayout(location = 1) in vec2 in_atlas_coords;\n\nlayout(location = 0) uniform mat4 camera;\n\nout vec2 v_atlas_coords;\n\nvoid main()\n{\n\tgl_Position = camera * vec4(in_coords, 1.0);\n\tv_atlas_coords = in_atlas_coords;\n}\n";

/* Content of "shaders/sun/sun.frag". */
extern const char g_shader_source_sun_frag[] = "\n#version 430 core\n\nin vec2 v_atlas_coords;\n\nlayout(location = 1) uniform sampler2D u_atlas;\n\nvoid main()\n{\n\tvec4 out_color = texture(u_atlas, v_atlas_coords);\n\tif (out_color.a < 0.001)\n\t{\n\t\tdiscard;\n\t}\n}\n";
