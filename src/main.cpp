
#include "config.hpp"
#include "gameloop.hpp"

using namespace qwy2;

int main(int argc, char const* const* argv)
{
	Config config{argc, argv};

	Game game{};
	game.loop(config);

	return 0;
}
