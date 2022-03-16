
#ifndef QWY2_HEADER_MESH_
#define QWY2_HEADER_MESH_

#include "opengl.hpp"
#include <vector>

namespace qwy2
{

/* Mesh of vertices in a vector and with a corresponding OpenGL buffer. */
template<typename VertexDataType>
class Mesh
{
public:
	std::vector<VertexDataType> vertex_data;
	GLenum opengl_buffer_usage;
	GLuint openglid;
	bool needs_update_opengl_data;

public:
	Mesh();
	Mesh(GLenum opengl_buffer_usage);
	~Mesh();

	void update_opengl_data();
};

} /* qwy2 */

#endif /* QWY2_HEADER_MESH_ */
