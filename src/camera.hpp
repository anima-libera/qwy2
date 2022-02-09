
#ifndef QWY2_HEADER_CAMERA_
#define QWY2_HEADER_CAMERA_

#include <glm/glm.hpp>
#include "utils.hpp"

namespace qwy2 {

class PerspectiveProjection
{
private:
	float fovy = TAU / 8.0f;

public:
	PerspectiveProjection();
	PerspectiveProjection(float fovy);
	glm::mat4 matrix(float near, float far) const;
};

class OrthographicProjection
{
private:
	float width = 100.0f;
	float height = 100.0f;

public:
	OrthographicProjection();
	OrthographicProjection(float width, float height);
	glm::mat4 matrix(float near, float far) const;
};

template<typename T>
inline constexpr bool is_projection =
	std::is_same_v<T, PerspectiveProjection> || 
	std::is_same_v<T, OrthographicProjection>;

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
	float near = 0.1f;
	float far = 400.0f;

public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 direction);
	Camera(glm::vec3 position, glm::vec3 direction, P projection, float near, float far);
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
