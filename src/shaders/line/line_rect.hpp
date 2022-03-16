
#ifndef QWY2_HEADER_LINE_RECT_
#define QWY2_HEADER_LINE_RECT_

#include "chunk.hpp"
#include "line.hpp"
#include "coords.hpp"

namespace qwy2
{

/* TODO: Remove this unoptimized temporary solution. Use instancing or something. */
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
