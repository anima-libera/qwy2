

#ifndef QWY2_HEADER_CONFIG_
#define QWY2_HEADER_CONFIG_

namespace qwy2
{

class Config
{
public:
	bool capture_cursor = true;
	float loaded_radius = 160.0f;
	unsigned int chunk_side = 45;

public:
	Config(int argc, char const* const* argv);
};

} /* qwy2 */

#endif /* QWY2_HEADER_CONFIG_ */
