
#ifndef QWY2_HEADER_UTILS_
#define QWY2_HEADER_UTILS_

#include <cassert>

namespace qwy2
{

inline constexpr float TAU = 6.28318530717f;

enum class ErrorCode
{
	OK,
	ERROR,
};

/* The cool modulo operator, the one that gives 7 for -3 mod 10. */
inline int cool_mod(int a, int b)
{
	assert(b > 0);
	if (a >= 0)
	{
		return a % b;
	}
	else
	{
		return (b - ((-a) % b)) % b;
	}
}

} /* qwy2 */

#endif /* QWY2_HEADER_UTILS_ */
