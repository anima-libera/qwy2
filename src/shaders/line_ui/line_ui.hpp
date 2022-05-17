
#ifndef QWY2_HEADER_LINE_UI_SHADER_
#define QWY2_HEADER_LINE_UI_SHADER_

#include "shaders/shader.hpp"

namespace qwy2
{

class VertexDataLineUi
{
public:
	glm::vec2 coords;
	glm::vec3 color;
};

template<typename VertexDataType>
class Mesh;

class ShaderProgramLineUi: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniform(Uniform uniform, UniformValue value) override;
	void draw(Mesh<VertexDataLineUi> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_LINE_UI_SHADER_ */
