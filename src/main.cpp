
#include "config.hpp"
#include "gameloop.hpp"
#include <iostream>

int main(int argc, char const* const* argv)
{
	using namespace qwy2;

	#ifndef DEBUG
		#define BUILD_STRING "release build"
	#else
		#define BUILD_STRING "debug build"
	#endif
	std::cout << "Qwy2 - Version: Indev 0.0.0 - " << BUILD_STRING << std::endl;

	Config config{};
	if (config.parse_command_line(argc, argv) == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	g_game = new Game{};
	g_game->init(config);
	g_game->loop();

	return 0;
}
