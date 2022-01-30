
#include "window.hpp"
#include <cstdlib>
#include <iostream>

int main()
{
	using namespace qwy2;

	std::cout << "Hellow~" << std::endl;

	if (init_window_graphics() == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	SDL_Delay(1000);

	cleanup_window_graphics();

	return 0;
}
