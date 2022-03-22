
#include "config.hpp"
#include "gameloop.hpp"
#include <cstdlib>

using namespace qwy2;

int main(int argc, char const* const* argv)
{
	Config config{};
	if (config.parse_command_line(argc, argv) == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	Game game{};
	game.loop(config);

	return 0;
}
