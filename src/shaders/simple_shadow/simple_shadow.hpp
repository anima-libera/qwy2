
#ifndef QWY2_HEADER_SIMPLE_SHADOW_SHADER_
#define QWY2_HEADER_SIMPLE_SHADOW_SHADER_

#include "shaders/shader.hpp"

namespace qwy2
{

/* Data held by one vertex in a mesh that can be rendered by the simple shader program. */
class VertexDataSimple;

template<typename VertexDataSimple>
class Mesh;

/* Same as the `shadow` shader, but with `VertexDataSimple` instead of `VertexDataClassic`. */
class ShaderProgramSimpleShadow: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniform(Uniform uniform, UniformValue value) override;
	void draw(Mesh<VertexDataSimple> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_SIMPLE_SHADOW_SHADER_ */
