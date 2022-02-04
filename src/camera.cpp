
#include "camera.hpp"
#include "window.hpp"
#include <SDL2/SDL.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace qwy2 {

Camera::Camera():
	position(glm::vec3()), direction(glm::vec3())
{
	this->compute_matrix();
}

Camera::Camera(glm::vec3 position, glm::vec3 direction):
	position(position), direction(direction)
{
	this->compute_matrix();
}

void Camera::set_position(glm::vec3 position)
{
	this->position = position;
	this->compute_matrix();
}

void Camera::set_direction(glm::vec3 direction)
{
	this->direction = direction;
	this->compute_matrix();
}

void Camera::compute_matrix()
{
	auto [width, height] = window_width_height();
	const float aspect_ratio =
		static_cast<float>(width) / static_cast<float>(height);
	const glm::mat4 projection =
		glm::perspective(this->fovy, aspect_ratio, this->near, this->far);
	const glm::mat4 view =
		glm::lookAt(this->position, this->position + this->direction, this->up);
	this->matrix = projection * view;
}

} /* qwy2 */
