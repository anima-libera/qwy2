
#include "window.hpp"
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cmath>

int main()
{
	using namespace qwy2;

	std::cout << "Hellow~" << std::endl;

	if (init_window_graphics() == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	using clock = std::chrono::high_resolution_clock;

	const auto clock_time_beginning = clock::now();
	float time = 0.0f; /* Time in seconds. */

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
				break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							running = false;
						break;
					}
				break;
			}
		}

		time = std::chrono::duration<double>(clock::now() - clock_time_beginning).count();
		
		glClearColor(time - std::floor(time), 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(g_window);
	}

	cleanup_window_graphics();

	return 0;
}
