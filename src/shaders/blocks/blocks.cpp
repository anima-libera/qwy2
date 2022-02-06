
#include "blocks.hpp"
#include "embedded.hpp"
#include <cstddef>

namespace qwy2 {

ErrorCode ShaderProgramBlocks::init()
{
	return this->compile_given_src(
		g_shader_source_bocks_vert,
		nullptr,
		g_shader_source_bocks_frag,
		"blocks vert", nullptr, "blocks frag", "blocks");
}	

void ShaderProgramBlocks::update_uniforms(UniformValues const& uniform_values)
{
	glUseProgram(this->openglid);
	glUniformMatrix4fv(0, 1, GL_FALSE,
		reinterpret_cast<const GLfloat*>(&uniform_values.camera_matrix[0][0]));
	glUniform1i(1, uniform_values.atlas_opengltextureid);
}

void ShaderProgramBlocks::draw(GLuint triangle_buffer_openglid, unsigned int vertex_count)
{
	glUseProgram(this->openglid);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_openglid);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, coords)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, atlas_coords)));

	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(0);
}

} /* qwy2 */
