
#ifndef QWY2_HEADER_DEBUG_MESSAGE_
#define QWY2_HEADER_DEBUG_MESSAGE_

#include <string_view>

namespace qwy2
{

/* Enables the OpenGL debug messages through callback calls feature.
 * OpenGL 4.3 or above is requiered for this function to work. */
void enable_opengl_debug_message();

/* Disables what is enabled by enable_opengl_debug_message. */
void disable_opengl_debug_message();

/* Prints an error message to stderr. Should be called with the name of the
 * SDL2 function that failed, or a quick description the operation involving
 * the SDL2 that failed. */
void error_sdl2_fail(std::string_view operation);

} /* qwy2 */

#endif /* QWY2_HEADER_DEBUG_MESSAGE_ */
