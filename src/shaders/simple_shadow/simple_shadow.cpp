
#include "simple_shadow.hpp"
#include "shaders/simple/simple.hpp"
#include "embedded.hpp"
#include "mesh.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <cstddef>

namespace qwy2
{

ErrorCode ShaderProgramSimpleShadow::init()
{
	return this->compile_given_src(
		g_shader_source_simple_shadow_vert,
		nullptr,
		g_shader_source_simple_shadow_frag,
		"simple shadow vert", nullptr, "simple shadow frag", "simple shadow");
}	

void ShaderProgramSimpleShadow::update_uniform(Uniform uniform, UniformValue value)
{
	glUseProgram(this->openglid);

	switch (uniform)
	{
		case Uniform::SUN_CAMERA_MATRIX:
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat4>(value)));
		break;

		default:
			;
		break;
	}
}

void ShaderProgramSimpleShadow::draw(Mesh<VertexDataSimple> const& mesh)
{
	glUseProgram(this->openglid);
	constexpr unsigned int vertex_attrib_number = 1;
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glEnableVertexAttribArray(i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	unsigned int attrib_index = 0;
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataSimple),
		reinterpret_cast<void*>(offsetof(VertexDataSimple, coords)));
	assert(attrib_index == vertex_attrib_number);

	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_data.size());
	
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glDisableVertexAttribArray(i);
	}
}

} /* qwy2 */
