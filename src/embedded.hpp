
#ifndef QWY2_HEADER_EMBEDDED_
#define QWY2_HEADER_EMBEDDED_

/* If this macro is used as would be the extern keyword in a variable
 * declaration, and if filename_ is a C-style string literal, then the compilation
 * script "_comp.py" will parse the variable declaration as a special
 * declaration that requires the variable to be set to the content of the file
 * found at filename_ in the generated source file "embedded.cpp". The variable
 * declaration is expected to be in the global scope and to have a type
 * compatible with the generated literal. The escape_mode_ parameter can be
 * one of the following macros and determines the way the content to embed is
 * escaped into a literal. */
#define EMBEDDED(filename_, escape_mode_) extern
#define TEXT /* Escapes the file content as a string literal. */
#define BINARY /* Escapes the file content as an array of bytes. */
#define SIZE /* Just produces an integer literal of the size in bytes. */

EMBEDDED("shaders/classic/classic.vert", TEXT) char const g_shader_source_classic_vert[];
EMBEDDED("shaders/classic/classic.frag", TEXT) char const g_shader_source_classic_frag[];

EMBEDDED("shaders/line/line.vert", TEXT) char const g_shader_source_line_vert[];
EMBEDDED("shaders/line/line.frag", TEXT) char const g_shader_source_line_frag[];

EMBEDDED("shaders/shadow/shadow.vert", TEXT) char const g_shader_source_shadow_vert[];
EMBEDDED("shaders/shadow/shadow.frag", TEXT) char const g_shader_source_shadow_frag[];

#endif /* QWY2_HEADER_EMBEDDED_ */
