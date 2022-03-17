
#include "line.hpp"
#include "embedded.hpp"
#include "chunk.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>

namespace qwy2
{

ErrorCode ShaderProgramLine::init()
{
	return this->compile_given_src(
		g_shader_source_line_vert,
		nullptr,
		g_shader_source_line_frag,
		"line vert", nullptr, "line frag", "line");
}	

void ShaderProgramLine::update_uniform(Uniform uniform, UniformValue value)
{
	glUseProgram(this->openglid);

	switch (uniform)
	{
		case Uniform::USER_CAMERA_MATRIX:
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat4>(value)));
		break;

		default:
			;
		break;
	}
}

void ShaderProgramLine::draw(Mesh<VertexDataLine> const& mesh)
{
	glUseProgram(this->openglid);
	constexpr unsigned int vertex_attrib_number = 2;
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glEnableVertexAttribArray(i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	unsigned int attrib_index = 0;
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataLine),
		reinterpret_cast<void*>(offsetof(VertexDataLine, coords)));
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataLine),
		reinterpret_cast<void*>(offsetof(VertexDataLine, color)));
	assert(attrib_index == vertex_attrib_number);

	glDrawArrays(GL_LINES, 0, mesh.vertex_data.size());
	
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glDisableVertexAttribArray(i);
	}
}

} /* qwy2 */
