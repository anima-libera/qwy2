
#ifndef QWY2_HEADER_SIMPLE_SHADER_
#define QWY2_HEADER_SIMPLE_SHADER_

#include "shaders/shader.hpp"

namespace qwy2
{

/* Data held by one vertex in a mesh that can be rendered by the simple shader program. */
class VertexDataSimple
{
public:
	glm::vec3 coords; /* Coords in the world, block level. */
	glm::vec3 normal;
	glm::vec3 color;
};

template<typename VertexDataSimple>
class Mesh;

/* The central shader program that should performs the rendering of most of the meshes
 * representing parts and objects of the world. */
class ShaderProgramSimple: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniform(Uniform uniform, UniformValue value) override;
	void draw(Mesh<VertexDataSimple> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_SIMPLE_SHADER_ */
