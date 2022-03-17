
#ifndef QWY2_HEADER_SHADER_
#define QWY2_HEADER_SHADER_

#include "opengl.hpp"
#include "utils.hpp"
#include <vector>
#include <variant>
#include <glm/glm.hpp>

namespace qwy2
{

enum class Uniform
{
	ATLAS_TEXTURE_IMAGE_UNIT_OPENGLID,
	ATLAS_SIDE,
	USER_CAMERA_MATRIX,
	USER_CAMERA_DIRECTION,
	SUN_CAMERA_MATRIX,
	SUN_CAMERA_DIRECTION,
	SHADOW_DEPTH_TEXTURE_IMAGE_UNIT_OPENGLID,
	USER_COORDS,
	FOG_COLOR,
	FOG_DISTANCE_INF,
	FOG_DISTANCE_SUP,
};

using UniformValue = std::variant<unsigned int, float, glm::vec3, glm::mat4>;

class ShaderProgram
{
public:
	GLint openglid = 0;

protected:
	ErrorCode compile_given_src(
		char const* src_vert, char const* src_geom, char const* src_frag,
		char const* debug_info_vert, char const* debug_info_geom, char const* debug_info_frag,
		char const* debug_info);
public:
	virtual ErrorCode init() = 0;
	void cleanup();
	virtual void update_uniform(Uniform uniform, UniformValue value) = 0;
};

} /* qwy2 */

#endif /* QWY2_HEADER_SHADER_ */
