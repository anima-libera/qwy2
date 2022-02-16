
#include "debug_message.hpp"
#include "opengl.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <string_view>

namespace qwy2
{

using namespace std::literals::string_view_literals;

static std::string_view opengl_debug_message_source_name(GLenum source)
{
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
			return "API"sv;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			return "WINDOW_SYSTEM"sv;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			return "SHADER_COMPILER"sv;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			return "THIRD_PARTY"sv;
		case GL_DEBUG_SOURCE_APPLICATION:
			return "APPLICATION"sv;
		case GL_DEBUG_SOURCE_OTHER:
			return "OTHER"sv;
		default:
			return "NOT_A_SOURCE"sv;
	}
}

static std::string_view opengl_debug_message_type_name(GLenum type)
{
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
			return "ERROR"sv;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			return "DEPRECATED_BEHAVIOR"sv;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			return "UNDEFINED_BEHAVIOR"sv;
		case GL_DEBUG_TYPE_PORTABILITY:
			return "PORTABILITY"sv;
		case GL_DEBUG_TYPE_PERFORMANCE:
			return "PERFORMANCE"sv;
		case GL_DEBUG_TYPE_MARKER:
			return "MARKER"sv;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			return "PUSH_GROUP"sv;
		case GL_DEBUG_TYPE_POP_GROUP:
			return "POP_GROUP"sv;
		case GL_DEBUG_TYPE_OTHER:
			return "OTHER"sv;
		default:
			return "NOT_A_TYPE"sv;
	}
}

static std::string_view opengl_debug_message_severity_name(GLenum type)
{
	switch (type)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			return "HIGH"sv;
		case GL_DEBUG_SEVERITY_MEDIUM:
			return "MEDIUM"sv;
		case GL_DEBUG_SEVERITY_LOW:
			return "LOW"sv;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			return "NOTIFICATION"sv;
		default:
			return "NOT_A_SEVERITY"sv;
	}
}

/* Debug message callback given to glDebugMessageCallback. Prints an error
 * message to stderr. */
static void GLAPIENTRY opengl_debug_message_callback(
	GLenum source, GLenum type, GLuint id, GLenum severity, [[maybe_unused]] GLsizei length,
	GLchar const* message, [[maybe_unused]] void const* user_param)
{
	#ifndef ENABLE_OPENGL_NOTIFICATIONS
		/* Filter out non-error debug messages if not opted-in. */
		if (type != GL_DEBUG_TYPE_ERROR)
		{
			return;
		}
		/* Note: The printing of non-error debug messages will often be similar to
		 * > OpenGL debug message (NOTIFICATION severity) API:OTHER(131185)
		 * > "Buffer detailed info: Buffer object 5
		 * > (bound to GL_ARRAY_BUFFER_ARB, usage hint is GL_DYNAMIC_DRAW)
		 * > will use VIDEO memory as the source for buffer object operations."
		 * (it looks like that on my machine). */
	#endif
	(type == GL_DEBUG_TYPE_ERROR ? std::cerr : std::cout)
		<< "OpenGL debug message "
		<< "(" << opengl_debug_message_severity_name(severity) << " severity) "
		<< opengl_debug_message_source_name(source) << ":" << opengl_debug_message_type_name(type)
		<< "(" << id << ") "
		<< (type == GL_DEBUG_TYPE_ERROR ? "\x1b[31m" : "\x1b[34m") << message << "\x1b[39m"
		<< std::endl;
}

void enable_opengl_debug_message()
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(opengl_debug_message_callback, nullptr);
}

void disable_opengl_debug_message()
{
	glDisable(GL_DEBUG_OUTPUT);
}

void error_sdl2_fail(std::string_view operation)
{
	std::cerr << "SDL2 error: " << operation << " failed: "
		<< "\x1b[31m\"" << SDL_GetError() << "\"\x1b[39m" << std::endl;
}

} /* qwy2 */
