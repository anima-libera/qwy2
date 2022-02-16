
#ifndef QWY2_HEADER_LINE_SHADER_
#define QWY2_HEADER_LINE_SHADER_

#include "shaders/shader.hpp"

namespace qwy2
{

class VertexDataLine
{
public:
	glm::vec3 coords; /* Coords in the world, block level. */
	glm::vec3 color;
};

template<typename VertexDataType>
class Mesh;

class ShaderProgramLine: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniforms(UniformValues const& uniform_values) override;
	void draw(Mesh<VertexDataLine> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_LINE_SHADER_ */
