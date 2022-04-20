
#include "config.hpp"
#include "gameloop.hpp"
#include <iostream>

using namespace qwy2;

int main(int argc, char const* const* argv)
{
	std::cout << "Qwy2 - Version Indev (0.0.0)" << std::endl;

	Config config{};
	if (config.parse_command_line(argc, argv) == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	Game game{config};
	game.loop();

	return 0;
}
