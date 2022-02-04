
#ifndef QWY2_HEADER_CAMERA_
#define QWY2_HEADER_CAMERA_

#include <glm/glm.hpp>
#include "utils.hpp"

namespace qwy2 {

class Camera
{
public:
	glm::mat4 matrix;
private:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	float fovy = TAU / 8.0f;
	float near = 0.1f;
	float far = 100.0f;

public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 direction);
	void set_position(glm::vec3 position);
	void set_direction(glm::vec3 direction);

private:
	void compute_matrix();
};

} /* qwy2 */

#endif /* QWY2_HEADER_CAMERA_ */
