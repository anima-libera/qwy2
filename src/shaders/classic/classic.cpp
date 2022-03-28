
#include "classic.hpp"
#include "embedded.hpp"
#include "chunk.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>
#include <cassert>

namespace qwy2
{

ErrorCode ShaderProgramClassic::init()
{
	return this->compile_given_src(
		g_shader_source_classic_vert,
		nullptr,
		g_shader_source_classic_frag,
		"classic vert", nullptr, "classic frag", "classic");
}	

void ShaderProgramClassic::update_uniform(Uniform uniform, UniformValue value)
{
	glUseProgram(this->openglid);

	switch (uniform)
	{
		case Uniform::USER_CAMERA_MATRIX:
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat4>(value)));
		break;

		case Uniform::ATLAS_TEXTURE_IMAGE_UNIT_OPENGLID:
		{
			glUniform1i(1, std::get<unsigned int>(value));
		}
		break;

		case Uniform::SUN_CAMERA_MATRIX:
			glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat4>(value)));
		break;

		case Uniform::SHADOW_DEPTH_TEXTURE_IMAGE_UNIT_OPENGLID:
		{
			glUniform1i(3, std::get<unsigned int>(value));
		}
		break;

		case Uniform::SUN_CAMERA_DIRECTION:
		{
			glm::vec3 const sun_camera_direction = std::get<glm::vec3>(value);
			glUniform3f(4,
				sun_camera_direction.x,
				sun_camera_direction.y,
				sun_camera_direction.z);
		}
		break;

		case Uniform::USER_CAMERA_DIRECTION:
		{
			glm::vec3 const user_camera_direction = std::get<glm::vec3>(value);
			glUniform3f(5,
				user_camera_direction.x,
				user_camera_direction.y,
				user_camera_direction.z);
		}
		break;

		case Uniform::ATLAS_SIDE:
			glUniform1f(6, std::get<float>(value));
		break;

		case Uniform::USER_COORDS:
		{
			glm::vec3 const user_coords = std::get<glm::vec3>(value);
			glUniform3f(7,
				user_coords.x,
				user_coords.y,
				user_coords.z);
		}
		break;

		case Uniform::FOG_COLOR:
		{
			glm::vec3 const fog_color = std::get<glm::vec3>(value);
			glUniform3f(8,
				fog_color.x,
				fog_color.y,
				fog_color.z);
		}
		break;

		case Uniform::FOG_DISTANCE_INF:
			glUniform1f(9, std::get<float>(value));
		break;

		case Uniform::FOG_DISTANCE_SUP:
			glUniform1f(10, std::get<float>(value));
		break;

		default:
			;
		break;
	}
}

void ShaderProgramClassic::draw(Mesh<VertexDataClassic> const& mesh)
{
	glUseProgram(this->openglid);
	constexpr unsigned int vertex_attrib_number = 6;
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glEnableVertexAttribArray(i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh.openglid);
	unsigned int attrib_index = 0;
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, coords)));
	glVertexAttribPointer(attrib_index++, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, normal)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, atlas_coords)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, atlas_coords_min)));
	glVertexAttribPointer(attrib_index++, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, atlas_coords_max)));
	glVertexAttribPointer(attrib_index++, 1, GL_FLOAT, GL_FALSE, sizeof(VertexDataClassic),
		reinterpret_cast<void*>(offsetof(VertexDataClassic, ambiant_occlusion)));
	assert(attrib_index == vertex_attrib_number);

	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_data.size());
	
	for (unsigned int i = 0; i < vertex_attrib_number; i++)
	{
		glDisableVertexAttribArray(i);
	}
}

} /* qwy2 */
