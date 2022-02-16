
#ifndef QWY2_HEADER_OBJECT_
#define QWY2_HEADER_OBJECT_

#include "chunk.hpp"
#include <utility>
#include <glm/vec3.hpp>

namespace qwy2
{

/* A 3D float rect aligned with the blocks. */
class AlignedBox
{
public:
	glm::vec3 center;
	glm::vec3 dimensions;

public:
	AlignedBox();
	AlignedBox(glm::vec3 center, glm::vec3 dimensions);
	BlockRect containing_block_rect() const;
	std::pair<glm::vec3, glm::vec3> coords_min_max() const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_OBJECT_ */
