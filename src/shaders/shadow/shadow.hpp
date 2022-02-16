
#ifndef QWY2_HEADER_SHADOW_SHADER_
#define QWY2_HEADER_SHADOW_SHADER_

#include "shaders/shader.hpp"

namespace qwy2
{

class VertexDataClassic;

template<typename VertexDataType>
class Mesh;

/* The shader program used to render the world viewed from the sun camera,
 * to get a depth buffer used to render cool dynamic shadows. */
class ShaderProgramShadow: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniforms(UniformValues const& uniform_values) override;
	void draw(Mesh<VertexDataClassic> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_SHADOW_SHADER_ */
