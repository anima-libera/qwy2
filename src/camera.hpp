
#ifndef QWY2_HEADER_CAMERA_
#define QWY2_HEADER_CAMERA_

#include <glm/glm.hpp>

namespace qwy2 {

class Camera
{
public:
	glm::mat4 matrix;

public:
	Camera();
};

} /* qwy2 */

#endif /* QWY2_HEADER_CAMERA_ */
