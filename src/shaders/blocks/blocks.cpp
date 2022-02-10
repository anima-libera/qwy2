
#include "blocks.hpp"
#include "embedded.hpp"
#include <cstddef>
#include <iostream>

namespace qwy2 {

ErrorCode ShaderProgramBlocks::init()
{
	return this->compile_given_src(
		g_shader_source_blocks_vert,
		nullptr,
		g_shader_source_blocks_frag,
		"blocks vert", nullptr, "blocks frag", "blocks");
}	

void ShaderProgramBlocks::update_uniforms(UniformValues const& uniform_values)
{
	glUseProgram(this->openglid);
	int active_texture = 0;

	glUniformMatrix4fv(0, 1, GL_FALSE,
		reinterpret_cast<const GLfloat*>(&uniform_values.player_camera_matrix[0][0]));
	
	glActiveTexture(GL_TEXTURE0 + active_texture);
	glBindTexture(GL_TEXTURE_2D, uniform_values.atlas_texture_openglid);
	glUniform1i(1, active_texture);
	active_texture++;

	glUniformMatrix4fv(2, 1, GL_FALSE,
		reinterpret_cast<const GLfloat*>(&uniform_values.sun_camera_matrix[0][0]));
	
	glActiveTexture(GL_TEXTURE0 + active_texture);
	glBindTexture(GL_TEXTURE_2D, uniform_values.sun_depth_texture_openglid);
	glUniform1i(3, active_texture);
	active_texture++;

	glUniform3f(4,
		uniform_values.sun_direction.x,
		uniform_values.sun_direction.y,
		uniform_values.sun_direction.z);

	glUniform3f(5,
		uniform_values.player_camera_direction.x,
		uniform_values.player_camera_direction.y,
		uniform_values.player_camera_direction.z);

	glUniform1f(6, uniform_values.atlas_side);
}

void ShaderProgramBlocks::draw(Mesh<BlockVertexData> const& mesh)
{
	glUseProgram(this->openglid);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, coords)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, atlas_coords)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, atlas_coords_min)));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof (BlockVertexData),
		reinterpret_cast<void*>(offsetof(BlockVertexData, atlas_coords_max)));

	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_data.size());
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glUseProgram(0);
}

} /* qwy2 */
