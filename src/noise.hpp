
#ifndef QWY2_HEADER_NOISE_
#define QWY2_HEADER_NOISE_

#include <glm/vec3.hpp>
#include <cstdint>

namespace qwy2
{

class NoiseGenerator
{
public:
	using SeedType = std::int32_t;

public:
	SeedType seed;

public:
	NoiseGenerator(SeedType seed);

public:
	float base_noise(int x) const;
	float base_noise(int x, int y) const;
	float base_noise(int x, int y, int z) const;
	float base_noise(int x, int y, int z, int w) const;
	float base_noise(float x) const;
	float base_noise(float x, float y) const;
	float base_noise(float x, float y, float z) const;
	float base_noise(float x, float y, float z, int w) const;
	float base_noise(float x, float y, float z, float w) const;
	/* TODO: Overload base_noise with block-level coords types,
	 * and with template parameter packs. */
};

} /* qwy2 */

#endif /* QWY2_HEADER_NOISE_ */
