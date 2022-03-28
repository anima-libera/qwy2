
#ifndef QWY2_HEADER_CLASSIC_SHADER_
#define QWY2_HEADER_CLASSIC_SHADER_

#include "shaders/shader.hpp"

namespace qwy2
{

/* Data held by one vertex in a mesh that can be rendered by the classic shader program. */
class VertexDataClassic
{
public:
	glm::vec3 coords; /* Coords in the world, block level. */
	glm::vec3 normal;

	/* The atlas coords of the vertex are given alogside the atlas rectangle area that contains
	 * the texture, so that the fragment shader can clamp the effective atlas coords to the
	 * rectangle and avoid bleeding on the rest of the atlas (due to floating point error). */
	glm::vec2 atlas_coords;
	glm::vec2 atlas_coords_min;
	glm::vec2 atlas_coords_max;

	float ambiant_occlusion;

	/* TODO: Make all this packed and smaller in memory! */
};

template<typename VertexDataType>
class Mesh;

/* The central shader program that should performs the rendering of most of the meshes
 * representing parts and objects of the world. */
class ShaderProgramClassic: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniform(Uniform uniform, UniformValue value) override;
	void draw(Mesh<VertexDataClassic> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_CLASSIC_SHADER_ */
