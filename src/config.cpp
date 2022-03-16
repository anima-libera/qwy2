
#include "config.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

namespace qwy2
{

Config::Config(int argc, char const* const* argv)
{
	for (unsigned int i = 1; i < static_cast<unsigned int>(argc); i++)
	{
		if (std::strcmp(argv[i], "--no-cursor-capture") == 0)
		{
			capture_cursor = false;
		}
		else if (std::strcmp(argv[i], "--loaded-radius") == 0)
		{
			i++;
			assert(static_cast<int>(i) < argc);
			std::stringstream arg{argv[i]};
			arg >> loaded_radius;
		}
		else if (std::strcmp(argv[i], "--chunk-side") == 0)
		{
			i++;
			assert(static_cast<int>(i) < argc);
			std::stringstream arg{argv[i]};
			arg >> chunk_side;
		}
		else
		{
			std::cout << "Command line error: Unknown argument " << argv[i] << std::endl;
		}
	}
}

} /* qwy2 */
