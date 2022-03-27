
#include "camera.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>

namespace qwy2
{

PerspectiveProjection::PerspectiveProjection():
	fovy{TAU / 8.0f}, aspect_ratio{1.0f}
{
	;
}

PerspectiveProjection::PerspectiveProjection(float fovy, float aspect_ratio):
	fovy{fovy}, aspect_ratio{aspect_ratio}
{
	;
}

glm::mat4 PerspectiveProjection::matrix(float near, float far) const
{
	return glm::perspective(this->fovy, this->aspect_ratio, near, far);
}

OrthographicProjection::OrthographicProjection():
	width{100.0f}, height{100.0f}
{
	;
}

OrthographicProjection::OrthographicProjection(float width, float height):
	width{width}, height{height}
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
	position{}, direction{}, near{0.1f}, far{400.0f}
{
	this->compute_matrix();
}

template<typename P>
Camera<P>::Camera(P projection, float near, float far):
	projection{projection}, near{near}, far{far}
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
	glm::mat4 const projection =
		this->projection.matrix(this->near, this->far);
	glm::mat4 const view =
		glm::lookAt(this->position, this->position + this->direction, this->up);
	this->matrix = projection * view;
}

template class Camera<PerspectiveProjection>;
template class Camera<OrthographicProjection>;

} /* qwy2 */
