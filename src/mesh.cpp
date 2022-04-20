
#include "mesh.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include <cassert>

namespace qwy2
{

template<typename VertexDataType>
Mesh<VertexDataType>::Mesh():
	Mesh{GL_DYNAMIC_DRAW}
{
	;
}

template<typename VertexDataType>
Mesh<VertexDataType>::Mesh(GLenum opengl_buffer_usage):
	opengl_buffer_usage{opengl_buffer_usage}, openglid{0}
{
	#if 0
	glGenBuffers(1, &this->openglid);
	assert(this->openglid != 0);
	#endif
}

template<typename VertexDataType>
Mesh<VertexDataType>::~Mesh()
{
	if (this->openglid != 0)
	{
		glDeleteBuffers(1, &this->openglid);
	}
}

template<typename VertexDataType>
void Mesh<VertexDataType>::update_opengl_data()
{
	if (this->openglid == 0)
	{
		glGenBuffers(1, &this->openglid);
		assert(this->openglid != 0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, this->openglid);
	glBufferData(GL_ARRAY_BUFFER, 
		this->vertex_data.size() * sizeof(VertexDataType),
		this->vertex_data.data(),
		this->opengl_buffer_usage);
	/* TODO: Optimize, maybe using glBufferSubData. */

	this->needs_update_opengl_data = false;
}

template class Mesh<VertexDataClassic>;
template class Mesh<VertexDataLine>;

} /* qwy2 */
