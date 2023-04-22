
#ifndef QWY2_HEADER_EMBEDDED_
#define QWY2_HEADER_EMBEDDED_

/* The EMBEDDED macro is magic as it is scanned for by the buildsystem (in this header file only)
 * and triggers the generation of a variable definition in the generted "embedded.cpp" source file.
 * It sould be used as a prefix to a variable declaration, and given a file path (starting after
 * the `src` directory). The generated definition will give the content of the file to the defined
 * variable (see "embedded.cpp" after building to see the results). The macro aslo takes an escape
 * mode parameter, it sould be one of the macros defined below and specify how the file content
 * is turned into a C++ literal (and for the case of SIZE it does not even embed content but
 * it initializes the variable to the size (in bytes) of the file).
 * The type of the declared variable should be chosen to work with the generated literal.
 * The `buildsystem/embed.py` script is responsible for the handling of the embedding. */
#define EMBEDDED(filename_, escape_mode_) extern
#define TEXT /* Escapes the file content as a string literal. */
#define BINARY /* Escapes the file content as an array of bytes. */
#define SIZE /* Just produces the size in bytes of the file content, as an integer literal. */

EMBEDDED("shaders/classic/classic.vert", TEXT) char const g_shader_source_classic_vert[];
EMBEDDED("shaders/classic/classic.frag", TEXT) char const g_shader_source_classic_frag[];

EMBEDDED("shaders/line/line.vert", TEXT) char const g_shader_source_line_vert[];
EMBEDDED("shaders/line/line.frag", TEXT) char const g_shader_source_line_frag[];

EMBEDDED("shaders/shadow/shadow.vert", TEXT) char const g_shader_source_shadow_vert[];
EMBEDDED("shaders/shadow/shadow.frag", TEXT) char const g_shader_source_shadow_frag[];

EMBEDDED("shaders/simple/simple.vert", TEXT) char const g_shader_source_simple_vert[];
EMBEDDED("shaders/simple/simple.frag", TEXT) char const g_shader_source_simple_frag[];

EMBEDDED("shaders/simple_shadow/simple_shadow.vert", TEXT) char const g_shader_source_simple_shadow_vert[];
EMBEDDED("shaders/simple_shadow/simple_shadow.frag", TEXT) char const g_shader_source_simple_shadow_frag[];

EMBEDDED("shaders/line_ui/line_ui.vert", TEXT) char const g_shader_source_line_ui_vert[];
EMBEDDED("shaders/line_ui/line_ui.frag", TEXT) char const g_shader_source_line_ui_frag[];

EMBEDDED("default_commands.qwy2", TEXT) char const g_default_commands[];

#endif /* QWY2_HEADER_EMBEDDED_ */
