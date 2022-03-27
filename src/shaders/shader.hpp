
#ifndef QWY2_HEADER_SHADER_
#define QWY2_HEADER_SHADER_

#include "opengl.hpp"
#include "utils.hpp"
#include <vector>
#include <variant>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace qwy2
{

/* Shader programs use global values called uniforms that can be set from the CPU side.
 * This enum lists all these uniforms (some of which are used by multiple shader programs).
 * To assign a value to one of these uniforms, use the `ShaderTable::update_uniform` method. */
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

/* This type can hold any value of which the type is the expected type for the listed above
 * uniforms used in the shaders of the game. */
using UniformValue = std::variant<unsigned int, float, glm::vec3, glm::mat4>;

/* Handles a shader program. */
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
	/* Compiles the handled shader program. */
	virtual ErrorCode init() = 0;

	/* Destroys the handled shader program. */
	void cleanup();

	/* Updates the value of the designated uniform (for the handled shader) to the given value,
	 * or do nothing if the handled shader does not use the designated uniform. */
	virtual void update_uniform(Uniform uniform, UniformValue value) = 0;
};

} /* qwy2 */

#endif /* QWY2_HEADER_SHADER_ */
