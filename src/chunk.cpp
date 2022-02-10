
#include "chunk.hpp"
#include "nature.hpp"
#include "shaders/blocks/blocks.hpp"
#include <iostream>
#include <iterator>

namespace qwy2 {

template<typename L>
CoordsInt<L>::CoordsInt(int x, int y, int z):
	x(x), y(y), z(z)
{
	;
}

template<typename L>
glm::vec3 CoordsInt<L>::to_float_coords() const
{
	return glm::vec3(this->x, this->y, this->z);
}

template<typename L>
bool CoordsInt<L>::operator==(CoordsInt const& other) const
{
	return this->arr == other.arr;
}

template<typename L>
std::size_t CoordsInt<L>::Hash::operator()(CoordsInt const& coords) const noexcept
{
	return
		std::hash<int>()(coords.x) ^
		std::hash<int>()(coords.y) ^
		std::hash<int>()(coords.z);
}

template class CoordsInt<BlockCoordsLevel>;
template class CoordsInt<ChunkCoordsLevel>;

template<typename L>
std::ostream& operator<<(std::ostream& out_stream, CoordsInt<L> const& coords)
{
	out_stream << "(" << coords.x << ", " << coords.y << ", " << coords.z << ")";
	return out_stream;
}

template std::ostream& operator<<(std::ostream& out_stream,
	CoordsInt<BlockCoordsLevel> const& coords);
template std::ostream& operator<<(std::ostream& out_stream,
	CoordsInt<ChunkCoordsLevel> const& coords);

template<typename L>
RectInt<L>::RectInt(CoordsInt<L> coords_min, CoordsInt<L> coords_max):
	coords_min(coords_min), coords_max(coords_max)
{
	;
}

template<typename L>
RectInt<L>::RectInt(CoordsInt<L> coords_center, unsigned int radius):
	coords_min(
		coords_center.x - static_cast<int>(radius - 1),
		coords_center.y - static_cast<int>(radius - 1),
		coords_center.z - static_cast<int>(radius - 1)),
	coords_max(
		coords_center.x + static_cast<int>(radius - 1),
		coords_center.y + static_cast<int>(radius - 1),
		coords_center.z + static_cast<int>(radius - 1))
{
	;
}

template<typename L>
template<Axis A>
unsigned int RectInt<L>::length() const
{
	const int axis = static_cast<int>(A);
	return this->coords_max.arr[axis] - this->coords_min.arr[axis] + 1;
}

//template unsigned int RectInt::length<Axis::X>() const;
//template unsigned int RectInt::length<Axis::Y>() const;
//template unsigned int RectInt::length<Axis::Z>() const;

template<typename L>
unsigned int RectInt<L>::volume() const
{
	return
		this->length<Axis::X>() *
		this->length<Axis::Y>() *
		this->length<Axis::Z>();
}

template<typename L>
bool RectInt<L>::contains(CoordsInt<L> const& coords) const
{
	return
		this->coords_min.x <= coords.x && coords.x <= this->coords_max.x &&
		this->coords_min.y <= coords.y && coords.y <= this->coords_max.y &&
		this->coords_min.z <= coords.z && coords.z <= this->coords_max.z;
}

template<typename L>
unsigned int RectInt<L>::to_index(CoordsInt<L> const& coords) const
{
	assert(this->contains(coords));
	return
		(coords.x - this->coords_min.x) +
		(coords.y - this->coords_min.y) * this->length<Axis::X>() +
		(coords.z - this->coords_min.z) * this->length<Axis::X>() * this->length<Axis::Y>();
}

template<typename L>
CoordsInt<L> RectInt<L>::walker_start() const
{
	return this->coords_min;
}

template<typename L>
bool RectInt<L>::walker_iterate(CoordsInt<L>& walker) const
{
	int axis = static_cast<int>(Axis::X);
	while (axis <= static_cast<int>(Axis::Z))
	{
		walker.arr[axis]++;
		if (walker.arr[axis] > this->coords_max.arr[axis])
		{
			walker.arr[axis] = this->coords_min.arr[axis];
			axis++;
		}
		else
		{
			return true;
		}
	}
	return false;
}

template class RectInt<BlockCoordsLevel>;
template class RectInt<ChunkCoordsLevel>;

template<typename L>
FaceInt<L>::FaceInt(CoordsInt<L> internal_coords, Axis axis, bool negativeward):
	internal_coords(internal_coords), axis(axis), negativeward(negativeward)
{
	;
}

template<typename L>
CoordsInt<L> FaceInt<L>::external_coords() const
{
	unsigned int index_axis = static_cast<int>(this->axis);
	CoordsInt<L> coords = this->internal_coords;
	coords.arr[index_axis] += this->negativeward ? -1 : 1;
	return coords;
}

template class FaceInt<BlockCoordsLevel>;
template class FaceInt<ChunkCoordsLevel>;

Block::Block():
	is_air(true)
{
	;
}

void Block::generate_face(Nature const& nature, BlockFace const& face,
	std::vector<BlockVertexData>& dst) const
{
	const unsigned int index_axis = static_cast<int>(face.axis);
	unsigned int index_a = index_axis == 0 ? 1 : 0;
	unsigned int index_b = index_axis == 2 ? 1 : 2;

	BlockType const& type = nature.block_type_table[this->type_index];
	AtlasRect atlas_rect = face.axis == Axis::Z ?
		(face.negativeward ? type.fase_bottom_rect : type.fase_top_rect) :
		type.fase_vertical_rect;

	const bool reverse_vertex_order =
		(face.axis == Axis::Y && not face.negativeward) ||
		(face.axis == Axis::X && face.negativeward) ||
		(face.axis == Axis::Z && face.negativeward);

	if ((face.axis == Axis::Y && not face.negativeward) ||
		(face.axis == Axis::X && face.negativeward))
	{
		std::swap(atlas_rect.atlas_coords_min.x, atlas_rect.atlas_coords_max.x);
	}
	if (face.axis == Axis::X || face.axis == Axis::Y)
	{
		std::swap(atlas_rect.atlas_coords_min.y, atlas_rect.atlas_coords_max.y);
	}
	//glm::vec2 atlas_coords_center =
	//	(atlas_rect.atlas_coords_min + atlas_rect.atlas_coords_max) / 2.0f;

	glm::vec3 normal{0.0f, 0.0f, 0.0f};
	normal[index_axis] = face.negativeward ? -1.0f : 1.0f;

	glm::vec3 coords_nn = face.internal_coords.to_float_coords() - glm::vec3(0.5f, 0.5f, 0.5f);
	coords_nn[index_axis] += face.negativeward ? 0.0f : 1.0f;

	BlockVertexData nn;
	nn.coords = coords_nn;
	nn.coords[index_a] += 0.0f;
	nn.coords[index_b] += 0.0f;
	nn.normal = normal;
	nn.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	nn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	nn.atlas_coords_min = atlas_rect.atlas_coords_min;
	nn.atlas_coords_max = atlas_rect.atlas_coords_max;
	BlockVertexData np;
	np.coords = coords_nn;
	np.coords[index_a] += 0.0f;
	np.coords[index_b] += 1.0f;
	np.normal = normal;
	np.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	np.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	np.atlas_coords_min = atlas_rect.atlas_coords_min;
	np.atlas_coords_max = atlas_rect.atlas_coords_max;
	BlockVertexData pn;
	pn.coords = coords_nn;
	pn.coords[index_a] += 1.0f;
	pn.coords[index_b] += 0.0f;
	pn.normal = normal;
	pn.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	pn.atlas_coords_min = atlas_rect.atlas_coords_min;
	pn.atlas_coords_max = atlas_rect.atlas_coords_max;
	BlockVertexData pp;
	pp.coords = coords_nn;
	pp.coords[index_a] += 1.0f;
	pp.coords[index_b] += 1.0f;
	pp.normal = normal;
	pp.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pp.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	pp.atlas_coords_min = atlas_rect.atlas_coords_min;
	pp.atlas_coords_max = atlas_rect.atlas_coords_max;

	const std::array<BlockVertexData, 6> vertex_data_sequence{nn, pn, pp, nn, pp, np};
	/* Does std::copy preallocate the appropriate size ? Probably...
	 * TODO: Find out and remove this std::vector::reserve call if redundant. */
	dst.reserve(dst.size() + vertex_data_sequence.size());
	if (reverse_vertex_order)
	{
		std::copy(vertex_data_sequence.rbegin(), vertex_data_sequence.rend(),
			std::back_inserter(dst));
	}
	else
	{
		std::copy(vertex_data_sequence.begin(), vertex_data_sequence.end(),
			std::back_inserter(dst));
	}
}

template<typename VertexDataType>
Mesh<VertexDataType>::Mesh():
	Mesh(GL_DYNAMIC_DRAW)
{
	;
}

template<typename VertexDataType>
Mesh<VertexDataType>::Mesh(GLenum opengl_buffer_usage):
	opengl_buffer_usage(opengl_buffer_usage)
{
	glGenBuffers(1, &this->openglid);
	assert(this->openglid != 0);
}

template<typename VertexDataType>
Mesh<VertexDataType>::~Mesh()
{
	glDeleteBuffers(1, &this->openglid);
}

template<typename VertexDataType>
void Mesh<VertexDataType>::update_opengl_data()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->openglid);
	glBufferData(GL_ARRAY_BUFFER, 
		this->vertex_data.size() * sizeof (VertexDataType),
		this->vertex_data.data(),
		this->opengl_buffer_usage);
}

template class Mesh<BlockVertexData>;

Chunk::Chunk(Nature& nature, BlockRect rect):
	rect(rect)
{
	this->block_grid.resize(rect.volume());
	nature.world_generator.generate_chunk_content(nature, *this);

	this->recompute_mesh(nature);
}

Block& Chunk::block(BlockCoords const& coords)
{
	assert(this->rect.contains(coords));
	return this->block_grid[this->rect.to_index(coords)];
}

void Chunk::recompute_mesh(Nature const& nature)
{
	mesh.vertex_data.clear();

	BlockCoords walker = this->rect.walker_start();
	do
	{
		if (not this->block(walker).is_air)
		{
			for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
			for (bool negativeward : {false, true})
			{
				BlockCoords neighbor = walker;
				neighbor.arr[static_cast<int>(axis)] += negativeward ? -1 : 1;
				if (this->rect.contains(neighbor) &&
					this->block(neighbor).is_air)
				{
					this->block(walker).generate_face(nature,
						BlockFace(walker, axis, negativeward),
						this->mesh.vertex_data);
				}
			}
		}
	}
	while (this->rect.walker_iterate(walker));

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh.openglid);
	glBufferData(GL_ARRAY_BUFFER, 
		this->mesh.vertex_data.size() * sizeof (decltype(this->mesh.vertex_data)::value_type),
		this->mesh.vertex_data.data(), GL_DYNAMIC_DRAW);
}

void Chunk::add_common_faces_to_mesh(Nature const& nature, ChunkGrid& chunk_grid,
	ChunkFace chunk_face, Chunk& touching_chunk)
{
	const unsigned int index_axis = static_cast<int>(chunk_face.axis);
	unsigned int index_a = index_axis == 0 ? 1 : 0;
	unsigned int index_b = index_axis == 2 ? 1 : 2;

	BlockRect rect = this->rect;
	if (chunk_face.negativeward)
	{
		rect.coords_max.arr[index_axis] = rect.coords_min.arr[index_axis];
	}
	else
	{
		rect.coords_min.arr[index_axis] = rect.coords_max.arr[index_axis];
	}

	for (int a = rect.coords_min.arr[index_a]; a <= rect.coords_max.arr[index_a]; a++)
	for (int b = rect.coords_min.arr[index_b]; b <= rect.coords_max.arr[index_b]; b++)
	{
		BlockCoords coords{0, 0, 0};
		coords.arr[index_axis] = rect.coords_min.arr[index_axis];
		coords.arr[index_a] = a;
		coords.arr[index_b] = b;
		BlockFace face{coords, chunk_face.axis, chunk_face.negativeward};

		if (not this->block(coords).is_air &&
			touching_chunk.block(face.external_coords()).is_air)
		{
			this->block(coords).generate_face(nature, face, this->mesh.vertex_data);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh.openglid);
	glBufferData(GL_ARRAY_BUFFER, 
		this->mesh.vertex_data.size() * sizeof (decltype(this->mesh.vertex_data)::value_type),
		this->mesh.vertex_data.data(), GL_DYNAMIC_DRAW);
}

ChunkGrid::ChunkGrid(int chunk_side):
	chunk_side(chunk_side)
{
	assert(1 < chunk_side && chunk_side % 2 == 1);
}

ChunkCoords ChunkGrid::containing_chunk_coords(BlockCoords coords) const
{
	return ChunkCoords(
		(coords.x + (coords.x < 0 ? -1 : 1) * this->chunk_side / 2) / this->chunk_side,
		(coords.y + (coords.y < 0 ? -1 : 1) * this->chunk_side / 2) / this->chunk_side,
		(coords.z + (coords.z < 0 ? -1 : 1) * this->chunk_side / 2) / this->chunk_side);
}

BlockRect ChunkGrid::chunk_rect(ChunkCoords chunk_coords) const
{
	const BlockCoords center{
		chunk_coords.x * this->chunk_side,
		chunk_coords.y * this->chunk_side,
		chunk_coords.z * this->chunk_side};
	const int margin = this->chunk_side / 2;
	return BlockRect(
		BlockCoords(center.x - margin, center.y - margin, center.z - margin),
		BlockCoords(center.x + margin, center.y + margin, center.z + margin));
}

BlockCoords ChunkGrid::containing_chunk_center_coords(BlockCoords coords) const
{
	return BlockCoords(
		(coords.x / this->chunk_side) * this->chunk_side,
		(coords.y / this->chunk_side) * this->chunk_side,
		(coords.z / this->chunk_side) * this->chunk_side);
}

BlockRect ChunkGrid::containing_chunk_rect(BlockCoords coords) const
{
	return this->chunk_rect(this->containing_chunk_coords(coords));
}

BlockRect ChunkGrid::containing_chunk_rect(glm::vec3 coords) const
{
	const BlockCoords coords_int{
		static_cast<int>(std::round(coords.x)),
		static_cast<int>(std::round(coords.y)),
		static_cast<int>(std::round(coords.z))};
	return this->containing_chunk_rect(coords_int);
}

Chunk* ChunkGrid::chunk(ChunkCoords chunk_coords)
{
	auto chunk_maybe = this->table.find(chunk_coords);
	if (chunk_maybe == this->table.end())
	{
		return nullptr;
	}
	else
	{
		Chunk* chunk = chunk_maybe->second;
		return chunk;
	}
}

Chunk* ChunkGrid::containing_chunk(BlockCoords coords)
{
	const ChunkCoords chunk_coords = this->containing_chunk_coords(coords);
	Chunk* chunk = this->chunk(chunk_coords);
	assert(chunk == nullptr || chunk->rect.contains(coords));
	return chunk;
}

Chunk* ChunkGrid::containing_chunk(glm::vec3 coords)
{
	const BlockCoords coords_int{
		static_cast<int>(std::round(coords.x)),
		static_cast<int>(std::round(coords.y)),
		static_cast<int>(std::round(coords.z))};
	return this->containing_chunk(coords_int);
}

Chunk* ChunkGrid::generate_chunk(Nature& nature, ChunkCoords chunk_coords)
{
	Chunk* chunk = this->table[chunk_coords] = new Chunk(nature, this->chunk_rect(chunk_coords));

	for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
	for (bool negativeward : {false, true})
	{
		ChunkFace chunk_face{chunk_coords, axis, negativeward};
		Chunk* touching_chunk = this->chunk(chunk_face.external_coords());
		if (touching_chunk != nullptr)
		{
			chunk->add_common_faces_to_mesh(nature, *this, chunk_face, *touching_chunk);

			ChunkFace chunk_face_mirror{chunk_face.external_coords(), axis, not negativeward};
			touching_chunk->add_common_faces_to_mesh(nature, *this, chunk_face_mirror, *chunk);
		}
	}

	return chunk;
}

} /* Qwy2 */
