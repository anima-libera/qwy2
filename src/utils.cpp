
#include "utils.hpp"
#include <ostream>

namespace qwy2
{

std::ostream& operator<<(std::ostream& out_stream, glm::vec3 const& vec3)
{
	out_stream << "(" << vec3.x << ", " << vec3.y << ", " << vec3.z << ")";
	return out_stream;
}

} /* qwy2 */
