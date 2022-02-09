
#ifndef QWY2_HEADER_BLOCKS_SHADER_
#define QWY2_HEADER_BLOCKS_SHADER_

#include "shaders/shader.hpp"
#include "chunk.hpp"

namespace qwy2 {

class BlockVertexData
{
public:
	glm::vec3 coords;
	glm::vec3 normal;
	glm::vec2 atlas_coords;
};

class ShaderProgramBlocks: public ShaderProgram
{
public:
	virtual ErrorCode init() override;
	virtual void update_uniforms(UniformValues const& uniform_values) override;
	void draw(Mesh<BlockVertexData> const& mesh);
};

} /* qwy2 */

#endif /* QWY2_HEADER_BLOCKS_SHADER_ */
