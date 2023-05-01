
/* Qwy2 header, contaning declarations for window-related initalization
 * and cleaning up, via SDL2 and OpenGL.
 * Implementations are in "window.cpp". */

#ifndef QWY2_HEADER_WINDOW_
#define QWY2_HEADER_WINDOW_

#include "utils.hpp"
#include <SDL2/SDL.h>
#include <tuple>

namespace qwy2
{

/* The unique global Qwy2 window.
 * It is created by init_window_graphics and destroyed by cleanup_window_graphics. */
extern SDL_Window* g_window;

/* The unique global SDL2-window-compatible OpenGL context used by Qwy2.
 * It is created by init_window_graphics and destroyed by cleanup_window_graphics. */
extern SDL_GLContext g_opengl_context;

/* Initializes the SDL2 library and the GLEW OpenGL extension loader.
 * Creates the unique global window g_window, the unique global OpenGL
 * context g_opengl_context, and an OpenGL VAO.
 * This function should be called once at the beginning of execution, and
 * a unique matching call to cleanup_window_graphics should be done at the end of
 * execution. */
ErrorCode init_window_graphics(bool fullscreen);

/* Cleans up and destroys what is initialized and created by init_window_graphics. */
void cleanup_window_graphics();

std::tuple<int, int> window_width_height();

} /* qwy2 */

#endif /* QWY2_HEADER_WINDOW_ */