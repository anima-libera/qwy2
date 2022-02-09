
#ifndef QWY2_HEADER_SUN_SHADER_
#define QWY2_HEADER_SUN_SHADER_

#include "shaders/shader.hpp"

namespace qwy2 {

class ShaderProgramSun: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniforms(UniformValues const& uniform_values) override;
	void draw(GLuint triangle_buffer_openglid, unsigned int vertex_count);
};

} /* qwy2 */

#endif /* QWY2_HEADER_SUN_SHADER_ */
