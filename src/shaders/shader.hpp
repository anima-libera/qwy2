
#ifndef QWY2_HEADER_SHADER_
#define QWY2_HEADER_SHADER_

#include "opengl.hpp"
#include "utils.hpp"
#include <glm/glm.hpp>

namespace qwy2 {

class UniformValues
{
public:
	unsigned int atlas_texture_openglid;
	float atlas_side;
	glm::mat4 player_camera_matrix;
	glm::vec3 player_camera_direction;
	glm::mat4 sun_camera_matrix;
	glm::vec3 sun_direction;
	unsigned int sun_depth_texture_openglid;
};

class ShaderProgram
{
public:
	GLint openglid = 0;

protected:
	ErrorCode compile_given_src(
		const char* src_vert, const char* src_geom, const char* src_frag,
		const char* debug_info_vert, const char* debug_info_geom, const char* debug_info_frag,
		const char* debug_info);
public:
	virtual ErrorCode init() = 0;
	void cleanup();
	virtual void update_uniforms(UniformValues const& uniform_values) = 0;
};

} /* qwy2 */

#endif /* QWY2_HEADER_SHADER_ */
