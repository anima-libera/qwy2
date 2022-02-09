
#include "camera.hpp"
#include "window.hpp"
#include <SDL2/SDL.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace qwy2 {

PerspectiveProjection::PerspectiveProjection()
{
	;
}

PerspectiveProjection::PerspectiveProjection(float fovy):
	fovy(fovy)
{
	;
}

glm::mat4 PerspectiveProjection::matrix(float near, float far) const
{
	const auto [width, height] = window_width_height();
	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	return glm::perspective(this->fovy, aspect_ratio, near, far);
}

OrthographicProjection::OrthographicProjection()
{
	;
}

OrthographicProjection::OrthographicProjection(float width, float height):
	width(width), height(height)
{
	;
}

glm::mat4 OrthographicProjection::matrix(float near, float far) const
{
	return glm::ortho(
		-this->width / 2.0f, this->width / 2.0f,
		-this->height / 2.0f, this->height / 2.0f,
		near, far);
}

template<typename P>
Camera<P>::Camera():
	position(glm::vec3()), direction(glm::vec3())
{
	this->compute_matrix();
}

template<typename P>
Camera<P>::Camera(glm::vec3 position, glm::vec3 direction):
	position(position), direction(direction)
{
	this->compute_matrix();
}

template<typename P>
Camera<P>::Camera(glm::vec3 position, glm::vec3 direction, P projection, float near, float far):
	position(position), direction(direction), projection(projection), near(near), far(far)
{
	this->compute_matrix();
}

template<typename P>
void Camera<P>::set_position(glm::vec3 position)
{
	this->position = position;
	this->compute_matrix();
}

template<typename P>
void Camera<P>::set_direction(glm::vec3 direction)
{
	this->direction = direction;
	this->compute_matrix();
}

template<typename P>
void Camera<P>::set_target_position(glm::vec3 target_position)
{
	this->direction = target_position - this->position;
	this->compute_matrix();
}

template<typename P>
void Camera<P>::set_projection(P projection)
{
	this->projection = projection;
	this->compute_matrix();
}

template<typename P>
glm::vec3 Camera<P>::get_direction() const
{
	return this->direction;
}

template<typename P>
void Camera<P>::compute_matrix()
{
	const glm::mat4 projection =
		this->projection.matrix(this->near, this->far);
	const glm::mat4 view =
		glm::lookAt(this->position, this->position + this->direction, this->up);
	this->matrix = projection * view;
}

template class Camera<PerspectiveProjection>;
template class Camera<OrthographicProjection>;

} /* qwy2 */
