
#include "shadow.hpp"
#include "shaders/classic/classic.hpp"
#include "mesh.hpp"
#include "embedded.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <cstddef>

namespace qwy2
{

ErrorCode ShaderProgramShadow::init()
{
	return this->compile_given_src(
		g_shader_source_shadow_vert,
		nullptr,
		g_shader_source_shadow_frag,
		"shadow vert", nullptr, "shadow frag", "shadow");
}	

void ShaderProgramShadow::update_uniform(Uniform uniform, UniformValue value)
{
	glUseProgram(this->openglid);

	switch (uniform)
	{
		case Uniform::SUN_CAMERA_MATRIX:
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat4>(value)));
		break;

		case Uniform::ATLAS_TEXTURE_IMAGE_UNIT_OPENGLID:
		{
			glUniform1i(1, std::get<unsigned int>(value));
		}
		break;

		default:
			;
		break;
	}
}

void ShaderProgramShadow::draw(Mesh<VertexDataClassic> const& mesh)
{
	glUseProgram(this->openglid);
	constexpr unsigned int vertex_attrib_number = 2;
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glEnableVertexAttribArray(i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	unsigned int attrib_index = 0;
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, coords)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, atlas_coords)));
	assert(attrib_index == vertex_attrib_number);

	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_data.size());
	
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glDisableVertexAttribArray(i);
	}
}

} /* qwy2 */
