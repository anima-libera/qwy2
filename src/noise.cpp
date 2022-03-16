
#include "noise.hpp"
#include "utils.hpp"
#include <chrono>
#include <cmath>

namespace qwy2
{

[[maybe_unused]] static inline unsigned int get_time_as_integer()
{
	using namespace std::chrono;
	auto not_an_integer = high_resolution_clock::now().time_since_epoch();
	auto integer = duration_cast<milliseconds>(not_an_integer).count();
	return integer;
}

static inline float interpolate_ratio_trig(float x)
{
	return (std::cos((1.0f - x) * TAU / 2) + 1.0f) / 2.0f;
}

static inline float interpolate_ratio_smoothstep(float x)
{
	return 3.0f * x * x - 2.0f * x * x * x;
}

/* Interpolates in the sense that x = 0 returns inf and x = 1 returns sup. */
static inline float interpolate(float x, float inf, float sup)
{
	float const ratio = interpolate_ratio_trig(x);
	//float const ratio = interpolate_ratio_smoothstep(x);
	return inf * (1.0f - ratio) + sup * ratio;
}

NoiseGenerator::NoiseGenerator(SeedType seed):
	seed{seed}
{
	;
}

float NoiseGenerator::base_noise(int x) const
{
	float const value =
		std::cos(static_cast<float>(x ^ 5467218 ^ this->seed) * 4561.3f);
	return value - std::floor(value);
}

float NoiseGenerator::base_noise(int x, int y) const
{
	float const value =
		std::cos(static_cast<float>(x ^ 6522762 ^ this->seed) * 4521.3f) +
		std::cos(static_cast<float>(y ^ 7162738 ^ this->seed) * 4581.3f) +
		std::cos(this->base_noise((x ^ y) + x + y) * 4581.3f);
	return value - std::floor(value);
}

float NoiseGenerator::base_noise(int x, int y, int z) const
{
	float const value =
		std::cos(static_cast<float>(x ^ 8133983 ^ this->seed) * 4531.3f) +
		std::cos(static_cast<float>(y ^ 2938398 ^ this->seed) * 4591.3f) +
		std::cos(static_cast<float>(z ^ 1863993 ^ this->seed) * 4511.3f) +
		std::cos(this->base_noise((x ^ y ^ z) + x + y + z) * 4511.3f);
	return value - std::floor(value);
}

float NoiseGenerator::base_noise(float x) const
{
	int const xi = static_cast<int>(std::floor(x));

	float const noise_i = this->base_noise(xi + 0);
	float const noise_s = this->base_noise(xi + 1);

	float const value_x = interpolate(x - std::floor(x), noise_i, noise_s);

	return value_x;
}

float NoiseGenerator::base_noise(float x, float y) const
{
	int const xi = static_cast<int>(std::floor(x));
	int const yi = static_cast<int>(std::floor(y));

	float const noise_i_i = this->base_noise(xi + 0, yi + 0);
	float const noise_i_s = this->base_noise(xi + 0, yi + 1);
	float const noise_s_i = this->base_noise(xi + 1, yi + 0);
	float const noise_s_s = this->base_noise(xi + 1, yi + 1);

	float const value_x_i = interpolate(x - std::floor(x), noise_i_i, noise_s_i);
	float const value_x_s = interpolate(x - std::floor(x), noise_i_s, noise_s_s);

	float const value_x_y = interpolate(y - std::floor(y), value_x_i, value_x_s);

	return value_x_y;
}

float NoiseGenerator::base_noise(float x, float y, float z) const
{
	int const xi = static_cast<int>(std::floor(x));
	int const yi = static_cast<int>(std::floor(y));
	int const zi = static_cast<int>(std::floor(z));

	float const noise_i_i_i = this->base_noise(xi + 0, yi + 0, zi + 0);
	float const noise_i_i_s = this->base_noise(xi + 0, yi + 0, zi + 1);
	float const noise_i_s_i = this->base_noise(xi + 0, yi + 1, zi + 0);
	float const noise_i_s_s = this->base_noise(xi + 0, yi + 1, zi + 1);
	float const noise_s_i_i = this->base_noise(xi + 1, yi + 0, zi + 0);
	float const noise_s_i_s = this->base_noise(xi + 1, yi + 0, zi + 1);
	float const noise_s_s_i = this->base_noise(xi + 1, yi + 1, zi + 0);
	float const noise_s_s_s = this->base_noise(xi + 1, yi + 1, zi + 1);

	float const value_x_i_i = interpolate(x - std::floor(x), noise_i_i_i, noise_s_i_i);
	float const value_x_i_s = interpolate(x - std::floor(x), noise_i_i_s, noise_s_i_s);
	float const value_x_s_i = interpolate(x - std::floor(x), noise_i_s_i, noise_s_s_i);
	float const value_x_s_s = interpolate(x - std::floor(x), noise_i_s_s, noise_s_s_s);

	float const value_x_y_i = interpolate(y - std::floor(y), value_x_i_i, value_x_s_i);
	float const value_x_y_s = interpolate(y - std::floor(y), value_x_i_s, value_x_s_s);

	float const value_x_y_z = interpolate(z - std::floor(z), value_x_y_i, value_x_y_s);

	return value_x_y_z;
}

} /* qwy2 */
