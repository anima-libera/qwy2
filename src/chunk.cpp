
#include "chunk.hpp"
#include "nature.hpp"
#include "shaders/classic/classic.hpp"
#include <iostream>
#include <iterator>

namespace qwy2
{

template<CoordsLevel L>
CoordsInt<L>::CoordsInt(int x, int y, int z):
	x{x}, y{y}, z{z}
{
	;
}

template<CoordsLevel L>
int& CoordsInt<L>::operator[](int index)
{
	return (&x)[index];
}

template<CoordsLevel L>
int const& CoordsInt<L>::operator[](int index) const
{
	return (&x)[index];
}

template<CoordsLevel L>
glm::vec3 CoordsInt<L>::to_float_coords() const
{
	return glm::vec3{this->x, this->y, this->z};
}

template<CoordsLevel L>
bool CoordsInt<L>::operator==(CoordsInt const& other) const
{
	/* TODO: Find some C++17-idiomatic way to remove this kind of code duplication. */
	return this->x == other.x && this->y == other.y && this->z == other.z;
}

template<CoordsLevel L>
bool CoordsInt<L>::operator!=(CoordsInt const& other) const
{
	return not (*this == other);
}

template<CoordsLevel L>
std::size_t CoordsInt<L>::Hash::operator()(CoordsInt const& coords) const noexcept
{
	return
		std::hash<int>()(coords.x ^ 10000) ^
		std::hash<int>()(coords.y ^ 54678) ^
		std::hash<int>()(coords.z ^ 22263);
}

template class CoordsInt<CoordsLevel::BLOCK>;
template class CoordsInt<CoordsLevel::CHUNK>;

template<CoordsLevel L>
std::ostream& operator<<(std::ostream& out_stream, CoordsInt<L> const& coords)
{
	out_stream << "(" << coords.x << ", " << coords.y << ", " << coords.z << ")";
	return out_stream;
}

template std::ostream& operator<<(std::ostream& out_stream,
	CoordsInt<CoordsLevel::BLOCK> const& coords);
template std::ostream& operator<<(std::ostream& out_stream,
	CoordsInt<CoordsLevel::CHUNK> const& coords);

template<CoordsLevel L>
RectInt<L>::RectInt(CoordsInt<L> coords_min, CoordsInt<L> coords_max):
	coords_min{coords_min}, coords_max{coords_max}
{
	assert(coords_min.x <= coords_max.x);
	assert(coords_min.y <= coords_max.y);
	assert(coords_min.z <= coords_max.z);
}

template<CoordsLevel L>
RectInt<L>::RectInt(CoordsInt<L> coords_center, unsigned int radius):
	coords_min{
		coords_center.x - static_cast<int>(radius - 1),
		coords_center.y - static_cast<int>(radius - 1),
		coords_center.z - static_cast<int>(radius - 1)},
	coords_max{
		coords_center.x + static_cast<int>(radius - 1),
		coords_center.y + static_cast<int>(radius - 1),
		coords_center.z + static_cast<int>(radius - 1)}
{
	;
}

template<CoordsLevel L>
template<Axis A>
unsigned int RectInt<L>::length() const
{
	int const axis = static_cast<int>(A);
	return this->coords_max[axis] - this->coords_min[axis] + 1;
}

template<CoordsLevel L>
unsigned int RectInt<L>::volume() const
{
	return
		this->length<Axis::X>() *
		this->length<Axis::Y>() *
		this->length<Axis::Z>();
}

template<CoordsLevel L>
bool RectInt<L>::contains(CoordsInt<L> const& coords) const
{
	return
		this->coords_min.x <= coords.x && coords.x <= this->coords_max.x &&
		this->coords_min.y <= coords.y && coords.y <= this->coords_max.y &&
		this->coords_min.z <= coords.z && coords.z <= this->coords_max.z;
}

template<CoordsLevel L>
unsigned int RectInt<L>::to_index(CoordsInt<L> const& coords) const
{
	assert(this->contains(coords));
	return
		(coords.x - this->coords_min.x) +
		(coords.y - this->coords_min.y) * this->length<Axis::X>() +
		(coords.z - this->coords_min.z) * this->length<Axis::X>() * this->length<Axis::Y>();
}

template<CoordsLevel L>
RectInt<L>::Iterator::Iterator(CoordsInt<L> starting_coords, RectInt const* containing_rect):
	current_coords{starting_coords}, containing_rect{containing_rect}
{
	;
}

template<CoordsLevel L>
CoordsInt<L> const& RectInt<L>::Iterator::operator*() const
{
	return this->current_coords;
}

template<CoordsLevel L>
typename RectInt<L>::Iterator& RectInt<L>::Iterator::operator++()
{
	int axis = static_cast<int>(Axis::X);
	while (axis <= static_cast<int>(Axis::Z))
	{
		this->current_coords[axis]++;
		if (this->current_coords[axis] > this->containing_rect->coords_max[axis])
		{
			this->current_coords[axis] = this->containing_rect->coords_min[axis];
			axis++;
		}
		else
		{
			return *this;
		}
	}
	/* Going past-the-end. */
	*this = this->containing_rect->end();
	return *this;
}

template<CoordsLevel L>
typename RectInt<L>::Iterator RectInt<L>::Iterator::operator++(int)
{
	RectInt<L>::Iterator copy = *this;
	++(*this);
	return copy;
}

template<CoordsLevel L>
bool RectInt<L>::Iterator::operator==(typename RectInt<L>::Iterator const& right) const
{
	assert(this->containing_rect == right.containing_rect);
	return this->current_coords == right.current_coords;
}

template<CoordsLevel L>
bool RectInt<L>::Iterator::operator!=(typename RectInt<L>::Iterator const& right) const
{
	assert(this->containing_rect == right.containing_rect);
	return this->current_coords != right.current_coords;
}

template<CoordsLevel L>
typename RectInt<L>::Iterator RectInt<L>::begin() const
{
	return RectInt<L>::Iterator{this->coords_min, this};
}

template<CoordsLevel L>
typename RectInt<L>::Iterator RectInt<L>::end() const
{
	CoordsInt<L> past_the_end = this->coords_max;
	past_the_end.x++;
	return RectInt<L>::Iterator{past_the_end, this};
}

template class RectInt<CoordsLevel::BLOCK>;
template class RectInt<CoordsLevel::CHUNK>;

template<CoordsLevel L>
FaceInt<L>::FaceInt(CoordsInt<L> internal_coords, Axis axis, bool negativeward):
	internal_coords{internal_coords}, axis{axis}, negativeward{negativeward}
{
	;
}

template<CoordsLevel L>
CoordsInt<L> FaceInt<L>::external_coords() const
{
	unsigned int index_axis = static_cast<int>(this->axis);
	CoordsInt<L> coords = this->internal_coords;
	coords[index_axis] += this->negativeward ? -1 : 1;
	return coords;
}

template class FaceInt<CoordsLevel::BLOCK>;
template class FaceInt<CoordsLevel::CHUNK>;

Block::Block():
	is_air{true}
{
	;
}

void Block::generate_face(Nature const& nature, BlockFace const& face,
	std::vector<ClassicVertexData>& dst) const
{
	unsigned int const index_axis = static_cast<int>(face.axis);
	unsigned int index_a = index_axis == 0 ? 1 : 0;
	unsigned int index_b = index_axis == 2 ? 1 : 2;

	BlockType const& type = nature.block_type_table[this->type_index];
	AtlasRect atlas_rect = face.axis == Axis::Z ?
		(face.negativeward ? type.fase_bottom_rect : type.fase_top_rect) :
		type.fase_vertical_rect;

	bool const reverse_vertex_order =
		(face.axis == Axis::Y && not face.negativeward) ||
		(face.axis == Axis::X && face.negativeward) ||
		(face.axis == Axis::Z && face.negativeward);

	glm::vec2 atlas_coords_min_real = atlas_rect.atlas_coords_min;
	glm::vec2 atlas_coords_max_real = atlas_rect.atlas_coords_max;

	if ((face.axis == Axis::Y && not face.negativeward) ||
		(face.axis == Axis::X && face.negativeward))
	{
		std::swap(atlas_rect.atlas_coords_min.x, atlas_rect.atlas_coords_max.x);
	}
	if (face.axis == Axis::X || face.axis == Axis::Y)
	{
		std::swap(atlas_rect.atlas_coords_min.y, atlas_rect.atlas_coords_max.y);
	}	

	glm::vec3 normal{0.0f, 0.0f, 0.0f};
	normal[index_axis] = face.negativeward ? -1.0f : 1.0f;

	glm::vec3 coords_nn = face.internal_coords.to_float_coords() - glm::vec3{0.5f, 0.5f, 0.5f};
	coords_nn[index_axis] += face.negativeward ? 0.0f : 1.0f;

	ClassicVertexData nn;
	nn.coords = coords_nn;
	nn.coords[index_a] += 0.0f;
	nn.coords[index_b] += 0.0f;
	nn.normal = normal;
	nn.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	nn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	nn.atlas_coords_min = atlas_coords_min_real;
	nn.atlas_coords_max = atlas_coords_max_real;
	ClassicVertexData np;
	np.coords = coords_nn;
	np.coords[index_a] += 0.0f;
	np.coords[index_b] += 1.0f;
	np.normal = normal;
	np.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	np.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	np.atlas_coords_min = atlas_coords_min_real;
	np.atlas_coords_max = atlas_coords_max_real;
	ClassicVertexData pn;
	pn.coords = coords_nn;
	pn.coords[index_a] += 1.0f;
	pn.coords[index_b] += 0.0f;
	pn.normal = normal;
	pn.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	pn.atlas_coords_min = atlas_coords_min_real;
	pn.atlas_coords_max = atlas_coords_max_real;
	ClassicVertexData pp;
	pp.coords = coords_nn;
	pp.coords[index_a] += 1.0f;
	pp.coords[index_b] += 1.0f;
	pp.normal = normal;
	pp.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pp.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	pp.atlas_coords_min = atlas_coords_min_real;
	pp.atlas_coords_max = atlas_coords_max_real;

	std::array<ClassicVertexData, 6> const vertex_data_sequence{nn, pn, pp, nn, pp, np};
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
	Mesh{GL_DYNAMIC_DRAW}
{
	;
}

template<typename VertexDataType>
Mesh<VertexDataType>::Mesh(GLenum opengl_buffer_usage):
	opengl_buffer_usage{opengl_buffer_usage}
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
		this->vertex_data.size() * sizeof(VertexDataType),
		this->vertex_data.data(),
		this->opengl_buffer_usage);
}

template class Mesh<ClassicVertexData>;

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

	for (BlockCoords const& walker : this->rect)
	{
		if (not this->block(walker).is_air)
		{
			for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
			for (bool negativeward : {false, true})
			{
				BlockCoords neighbor = walker;
				neighbor[static_cast<int>(axis)] += negativeward ? -1 : 1;
				if (this->rect.contains(neighbor) &&
					this->block(neighbor).is_air)
				{
					this->block(walker).generate_face(nature,
						BlockFace{walker, axis, negativeward},
						this->mesh.vertex_data);
				}
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh.openglid);
	glBufferData(GL_ARRAY_BUFFER, 
		this->mesh.vertex_data.size() * sizeof(decltype(this->mesh.vertex_data)::value_type),
		this->mesh.vertex_data.data(), GL_DYNAMIC_DRAW);
}

void Chunk::add_common_faces_to_mesh(Nature const& nature,
	ChunkFace chunk_face, Chunk& touching_chunk)
{
	unsigned int const index_axis = static_cast<int>(chunk_face.axis);
	unsigned int index_a = index_axis == 0 ? 1 : 0;
	unsigned int index_b = index_axis == 2 ? 1 : 2;

	BlockRect rect = this->rect;
	if (chunk_face.negativeward)
	{
		rect.coords_max[index_axis] = rect.coords_min[index_axis];
	}
	else
	{
		rect.coords_min[index_axis] = rect.coords_max[index_axis];
	}

	for (int a = rect.coords_min[index_a]; a <= rect.coords_max[index_a]; a++)
	for (int b = rect.coords_min[index_b]; b <= rect.coords_max[index_b]; b++)
	{
		BlockCoords coords{0, 0, 0};
		coords[index_axis] = rect.coords_min[index_axis];
		coords[index_a] = a;
		coords[index_b] = b;
		BlockFace face{coords, chunk_face.axis, chunk_face.negativeward};

		if (not this->block(coords).is_air &&
			touching_chunk.block(face.external_coords()).is_air)
		{
			this->block(coords).generate_face(nature, face, this->mesh.vertex_data);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh.openglid);
	glBufferData(GL_ARRAY_BUFFER, 
		this->mesh.vertex_data.size() * sizeof(decltype(this->mesh.vertex_data)::value_type),
		this->mesh.vertex_data.data(), GL_DYNAMIC_DRAW);
}

ChunkGrid::ChunkGrid(int chunk_side):
	chunk_side{chunk_side}
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
	BlockCoords const center{
		chunk_coords.x * this->chunk_side,
		chunk_coords.y * this->chunk_side,
		chunk_coords.z * this->chunk_side};
	int const margin = this->chunk_side / 2;
	return BlockRect{
		BlockCoords{center.x - margin, center.y - margin, center.z - margin},
		BlockCoords{center.x + margin, center.y + margin, center.z + margin}};
}

BlockCoords ChunkGrid::containing_chunk_center_coords(BlockCoords coords) const
{
	return BlockCoords{
		(coords.x / this->chunk_side) * this->chunk_side,
		(coords.y / this->chunk_side) * this->chunk_side,
		(coords.z / this->chunk_side) * this->chunk_side};
}

BlockRect ChunkGrid::containing_chunk_rect(BlockCoords coords) const
{
	return this->chunk_rect(this->containing_chunk_coords(coords));
}

BlockRect ChunkGrid::containing_chunk_rect(glm::vec3 coords) const
{
	BlockCoords const coords_int{
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
	ChunkCoords const chunk_coords = this->containing_chunk_coords(coords);
	Chunk* chunk = this->chunk(chunk_coords);
	assert(chunk == nullptr || chunk->rect.contains(coords));
	return chunk;
}

Chunk* ChunkGrid::containing_chunk(glm::vec3 coords)
{
	BlockCoords const coords_int{
		static_cast<int>(std::round(coords.x)),
		static_cast<int>(std::round(coords.y)),
		static_cast<int>(std::round(coords.z))};
	return this->containing_chunk(coords_int);
}

Chunk* ChunkGrid::generate_chunk(Nature& nature, ChunkCoords chunk_coords)
{
	Chunk* chunk = this->table[chunk_coords] = new Chunk{nature, this->chunk_rect(chunk_coords)};

	for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
	for (bool negativeward : {false, true})
	{
		ChunkFace chunk_face{chunk_coords, axis, negativeward};
		Chunk* touching_chunk = this->chunk(chunk_face.external_coords());
		if (touching_chunk != nullptr)
		{
			chunk->add_common_faces_to_mesh(nature, chunk_face, *touching_chunk);

			ChunkFace chunk_face_mirror{chunk_face.external_coords(), axis, not negativeward};
			touching_chunk->add_common_faces_to_mesh(nature, chunk_face_mirror, *chunk);
		}
	}

	return chunk;
}

} /* Qwy2 */
