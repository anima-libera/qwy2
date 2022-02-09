
#ifndef QWY2_HEADER_SUN_SHADER_
#define QWY2_HEADER_SUN_SHADER_

#include "shaders/shader.hpp"
#include "chunk.hpp"

namespace qwy2 {

class ShaderProgramSun: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniforms(UniformValues const& uniform_values) override;
	void draw(Mesh<BlockVertexData> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_SUN_SHADER_ */
