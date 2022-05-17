
#include "line_ui.hpp"
#include "embedded.hpp"
#include "mesh.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <cstddef>

namespace qwy2
{

ErrorCode ShaderProgramLineUi::init()
{
	return this->compile_given_src(
		g_shader_source_line_ui_vert,
		nullptr,
		g_shader_source_line_ui_frag,
		"line ui vert", nullptr, "line ui frag", "line ui");
}	

void ShaderProgramLineUi::update_uniform(Uniform uniform, UniformValue value)
{
	glUseProgram(this->openglid);

	switch (uniform)
	{
		default:
			;
		break;
	}
}

void ShaderProgramLineUi::draw(Mesh<VertexDataLineUi> const& mesh)
{
	glUseProgram(this->openglid);
	constexpr unsigned int vertex_attrib_number = 2;
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glEnableVertexAttribArray(i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	unsigned int attrib_index = 0;
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLineUi),
		reinterpret_cast<void*>(offsetof(VertexDataLineUi, coords)));
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataLineUi),
		reinterpret_cast<void*>(offsetof(VertexDataLineUi, color)));
	assert(attrib_index == vertex_attrib_number);

	glDrawArrays(GL_LINES, 0, mesh.vertex_data.size());
	
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glDisableVertexAttribArray(i);
	}
}

} /* qwy2 */
