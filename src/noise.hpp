
#ifndef QWY2_HEADER_NOISE_
#define QWY2_HEADER_NOISE_

#include <glm/glm.hpp>
#include <cstdint>

namespace qwy2 {

class NoiseGenerator
{
public:
	using SeedType = std::uint32_t;

public:
	SeedType seed;

public:
	NoiseGenerator(SeedType seed);

public:
	float base_noise(int x);
	float base_noise(int x, int y);
	float base_noise(int x, int y, int z);
	float base_noise(float x);
	float base_noise(float x, float y);
	float base_noise(float x, float y, float z);
};

} /* qwy2 */

#endif /* QWY2_HEADER_NOISE_ */