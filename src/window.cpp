
#include "dbgmsg.hpp"
#include "opengl.hpp"
#include "utils.hpp"
#include <SDL2/SDL.h>
#include <tuple>

namespace qwy2 {

SDL_Window* g_window = nullptr;
SDL_GLContext g_opengl_context = nullptr;

static GLuint s_vao_id;

#define WINDOW_NAME "Qwy2"

ErrorCode init_window_graphics()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
	{
		error_sdl2_fail("SDL_Init");
		return ErrorCode::ERROR;
	}
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	g_window = SDL_CreateWindow(WINDOW_NAME,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600, 800,
		SDL_WINDOW_OPENGL);
	if (g_window == nullptr)
	{
		error_sdl2_fail("SDL_CreateWindow");
		return ErrorCode::ERROR;
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4) != 0)
	{
		error_sdl2_fail("SDL_GL_SetAttribute with SDL_GL_CONTEXT_MAJOR_VERSION");
		return ErrorCode::ERROR;
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0)
	{
		error_sdl2_fail("SDL_GL_SetAttribute with SDL_GL_CONTEXT_MINOR_VERSION");
		return ErrorCode::ERROR;
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0)
	{
		error_sdl2_fail("SDL_GL_SetAttribute with SDL_GL_CONTEXT_PROFILE_MASK");
		return ErrorCode::ERROR;
	}
	g_opengl_context = SDL_GL_CreateContext(g_window);
	if (g_opengl_context == nullptr)
	{
		error_sdl2_fail("SDL_GL_CreateContext");
		return ErrorCode::ERROR;
	}
	#ifdef USE_GLEW
		GLenum gnew_init_result = glewInit();
		if (gnew_init_result != GLEW_OK)
		{
			std::cerr << "GLEW error: glewInit failed: " <<
				"\"" << glewGetErrorString(gnew_init_result) << "\"" << std::endl;
		}
	#endif
	enable_opengl_dbgmsg();
	glEnable(GL_MULTISAMPLE);
	if (SDL_GL_SetSwapInterval(-1) != 0)
	{
		SDL_GL_SetSwapInterval(1);
	}
	glGenVertexArrays(1, &s_vao_id);
	glBindVertexArray(s_vao_id);
	return ErrorCode::OK;
}

void cleanup_window_graphics()
{
	glDeleteVertexArrays(1, &s_vao_id);
	SDL_GL_DeleteContext(g_opengl_context);
	g_opengl_context = nullptr;
	SDL_DestroyWindow(g_window);
	g_window = nullptr;
	SDL_Quit();
}

std::tuple<int, int> window_width_height()
{
	int width, height;
	SDL_GetWindowSize(g_window, &width, &height);
	return std::make_tuple(width, height);
}

} /* qwy2 */
