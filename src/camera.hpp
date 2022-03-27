
#ifndef QWY2_HEADER_CAMERA_
#define QWY2_HEADER_CAMERA_

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "utils.hpp"

namespace qwy2
{

/* Set of values exclusive to the generation of a perspective projection matrix.
 * Perspective such that perspective lines eventually cross, this is not isometric. */
class PerspectiveProjection
{
private:
	float fovy; /* Camera angle ? (If so, which one ?) */
	float aspect_ratio; /* Ratio width / height. */

public:
	PerspectiveProjection();
	PerspectiveProjection(float fovy, float aspect_ratio);
	glm::mat4 matrix(float near, float far) const;
};

/* Set of values exclusive to the generation of an orthographic projection matrix.
 * It produces an isometric rendering in which perspective lines are parallel. */
class OrthographicProjection
{
private:
	float width;
	float height;

public:
	OrthographicProjection();
	OrthographicProjection(float width, float height);
	glm::mat4 matrix(float near, float far) const;
};

template<typename T>
inline constexpr bool is_projection =
	std::is_same_v<T, PerspectiveProjection> || 
	std::is_same_v<T, OrthographicProjection>;

/* View projection matrix allowing the rendering of 3D scenes onto 2D screens.
 * The projection part of the view projection matrix is handled by the projection
 * member as well as the near and far members. The view part is handled by the rest. */
template<typename P>
class Camera
{
	static_assert(is_projection<P>);

public:
	glm::mat4 matrix;
private:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	P projection;
	float near; /* Near clipping plane distance. */
	float far; /* Far clipping plane distance. */

public:
	Camera();
	Camera(P projection, float near, float far);
	void set_position(glm::vec3 position);
	void set_direction(glm::vec3 direction);
	void set_target_position(glm::vec3 target_position);
	void set_projection(P projection);
	glm::vec3 get_direction() const;

private:
	void compute_matrix();
};

} /* qwy2 */

#endif /* QWY2_HEADER_CAMERA_ */
