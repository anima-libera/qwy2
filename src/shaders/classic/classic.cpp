
#include "classic.hpp"
#include "embedded.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>
#include <iostream>

namespace qwy2 {

ErrorCode ShaderProgramClassic::init()
{
	return this->compile_given_src(
		g_shader_source_classic_vert,
		nullptr,
		g_shader_source_classic_frag,
		"classic vert", nullptr, "classic frag", "classic");
}	

void ShaderProgramClassic::update_uniforms(UniformValues const& uniform_values)
{
	glUseProgram(this->openglid);
	int active_texture = 0;

	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(uniform_values.user_camera_matrix));
	
	glActiveTexture(GL_TEXTURE0 + active_texture);
	glBindTexture(GL_TEXTURE_2D, uniform_values.atlas_texture_openglid);
	glUniform1i(1, active_texture);
	active_texture++;

	glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(uniform_values.sun_camera_matrix));
	
	glActiveTexture(GL_TEXTURE0 + active_texture);
	glBindTexture(GL_TEXTURE_2D, uniform_values.shadow_depth_texture_openglid);
	glUniform1i(3, active_texture);
	active_texture++;

	glUniform3f(4,
		uniform_values.sun_camera_direction.x,
		uniform_values.sun_camera_direction.y,
		uniform_values.sun_camera_direction.z);

	glUniform3f(5,
		uniform_values.user_camera_direction.x,
		uniform_values.user_camera_direction.y,
		uniform_values.user_camera_direction.z);

	glUniform1f(6, uniform_values.atlas_side);
}

void ShaderProgramClassic::draw(Mesh<ClassicVertexData> const& mesh)
{
	glUseProgram(this->openglid);
	constexpr unsigned int vertex_attrib_number = 5;
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glEnableVertexAttribArray(i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	unsigned int attrib_index = 0;
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof (ClassicVertexData),
		reinterpret_cast<void*>(offsetof(ClassicVertexData, coords)));
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof (ClassicVertexData),
		reinterpret_cast<void*>(offsetof(ClassicVertexData, normal)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof (ClassicVertexData),
		reinterpret_cast<void*>(offsetof(ClassicVertexData, atlas_coords)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof (ClassicVertexData),
		reinterpret_cast<void*>(offsetof(ClassicVertexData, atlas_coords_min)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof (ClassicVertexData),
		reinterpret_cast<void*>(offsetof(ClassicVertexData, atlas_coords_max)));
	assert(attrib_index == vertex_attrib_number);

	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_data.size());
	
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glDisableVertexAttribArray(i);
	}
}

} /* qwy2 */
