
#ifndef QWY2_HEADER_DBGMSG_
#define QWY2_HEADER_DBGMSG_

/* Enables the OpenGL debug messages through callback calls feature.
 * OpenGL 4.3 or above is requiered for this function to work. */
void enable_opengl_dbgmsg();

/* Disables what is enabled by enable_opengl_dbgmsg. */
void disable_opengl_dbgmsg();

/* Prints an error message to stderr. Should be called with the name of the
 * SDL2 function that failed, or a quick description the operation involving
 * the SDL2 that failed. */
void error_sdl2_fail(const char* operation);

#endif /* QWY2_HEADER_DBGMSG_ */
