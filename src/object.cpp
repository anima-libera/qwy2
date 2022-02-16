
#include "object.hpp"

namespace qwy2
{

AlignedBox::AlignedBox()
{
	;
}

AlignedBox::AlignedBox(glm::vec3 center, glm::vec3 dimensions):
	center{center}, dimensions{dimensions}
{
	;
}

BlockRect AlignedBox::containing_block_rect() const
{
	return BlockRect{
		BlockCoords{
			static_cast<int>(std::round(this->center.x - this->dimensions.x / 2.0f)),
			static_cast<int>(std::round(this->center.y - this->dimensions.y / 2.0f)),
			static_cast<int>(std::round(this->center.z - this->dimensions.z / 2.0f))},
		BlockCoords{
			static_cast<int>(std::round(this->center.x + this->dimensions.x / 2.0f)),
			static_cast<int>(std::round(this->center.y + this->dimensions.y / 2.0f)),
			static_cast<int>(std::round(this->center.z + this->dimensions.z / 2.0f))}};
}

std::pair<glm::vec3, glm::vec3> AlignedBox::coords_min_max() const
{
	return std::make_pair(
		glm::vec3{
			this->center.x - this->dimensions.x / 2.0f,
			this->center.y - this->dimensions.y / 2.0f,
			this->center.z - this->dimensions.z / 2.0f},
		glm::vec3{
			this->center.x + this->dimensions.x / 2.0f,
			this->center.y + this->dimensions.y / 2.0f,
			this->center.z + this->dimensions.z / 2.0f});
}

} /* qwy2 */
