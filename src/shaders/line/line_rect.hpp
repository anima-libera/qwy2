
#ifndef QWY2_HEADER_LINE_RECT_
#define QWY2_HEADER_LINE_RECT_

#include "line.hpp"
#include "mesh.hpp"

namespace qwy2
{

class AlignedBox;

/* Temporary solution to generate a mesh representing the edges of a 3D rectangle. */
class LineRectDrawer
{
public:
	Mesh<VertexDataLine> mesh;
	glm::vec3 color;

public:
	LineRectDrawer();
	void set_box(AlignedBox const& aligned_box);
};

} /* qwy2 */

#endif /* QWY2_HEADER_LINE_RECT_ */
