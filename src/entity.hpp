
#ifndef QWY2_HEADER_ENTITY_
#define QWY2_HEADER_ENTITY_

#include "coords.hpp"
#include <glm/vec3.hpp>
#include <optional>

namespace qwy2
{

class EntityPhysics
{
public:
	glm::vec3 box_dimensions;
	bool is_falling;
	glm::vec3 motion;

public:
	EntityPhysics(glm::vec3 box_dimensions);
};

class Entity
{
public:
	glm::vec3 coords;
	std::optional<EntityPhysics> physics;

public:
	Entity(glm::vec3 coords);
	Entity(glm::vec3 coords, EntityPhysics physics);

	void apply_motion(float delta_time);
};

} /* qwy2 */

#endif /* QWY2_HEADER_ENTITY_ */
