
#include "sun.hpp"
#include "shaders/blocks/blocks.hpp"
#include "embedded.hpp"
#include <cstddef>

namespace qwy2 {

ErrorCode ShaderProgramSun::init()
{
	return this->compile_given_src(
		g_shader_source_sun_vert,
		nullptr,
		g_shader_source_sun_frag,
		"sun vert", nullptr, "sun frag", "sun");
}	

void ShaderProgramSun::update_uniforms(UniformValues const& uniform_values)
{
	glUseProgram(this->openglid);
	int active_texture = 0;

	glUniformMatrix4fv(0, 1, GL_FALSE,
		reinterpret_cast<const GLfloat*>(&uniform_values.sun_camera_matrix[0][0]));

	glActiveTexture(GL_TEXTURE0 + active_texture);
	glBindTexture(GL_TEXTURE_2D, uniform_values.atlas_texture_openglid);
	glUniform1i(1, active_texture);
	active_texture++;
}

void ShaderProgramSun::draw(Mesh<BlockVertexData> const& mesh)
{
	glUseProgram(this->openglid);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, coords)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, atlas_coords)));

	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_data.size());
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(0);
}

} /* qwy2 */
