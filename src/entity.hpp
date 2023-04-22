
#ifndef QWY2_HEADER_ENTITY_
#define QWY2_HEADER_ENTITY_

#include "coords.hpp"
#include "mesh.hpp"
#include "shaders/simple/simple.hpp"
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

class EntityRendering
{
public:
	Mesh<VertexDataSimple> mesh;

public:
	EntityRendering();
	void draw(glm::vec3 coords, bool shadow);
};

class Entity
{
public:
	glm::vec3 coords;
	std::optional<EntityPhysics> physics;
	std::optional<EntityRendering> rendering;

public:
	Entity(glm::vec3 coords);
	Entity(glm::vec3 coords, EntityPhysics physics);

	void apply_motion(float delta_time);

	void draw();
	void draw_shadow();
};

} /* qwy2 */

#endif /* QWY2_HEADER_ENTITY_ */
