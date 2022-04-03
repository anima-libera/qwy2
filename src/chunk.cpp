
#include "chunk.hpp"
#include "nature.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include <iostream>
#include <iterator>

namespace qwy2
{

Block::Block():
	is_air{true}
{
	;
}

void Block::generate_face(Nature const& nature, BlockFace const& face,
	std::vector<VertexDataClassic>& dst) const
{
	auto const [index_axis, index_a, index_b] = face.indices_axis_a_b();

	BlockType const& type = nature.block_type_table[this->type_index];
	AtlasRect atlas_rect = face.axis == Axis::Z ?
		(face.negativeward ? type.fase_bottom_rect : type.fase_top_rect) :
		type.fase_vertical_rect;

	bool const reverse_vertex_order =
		(face.axis == Axis::X && face.negativeward) ||
		(face.axis == Axis::Y && not face.negativeward) ||
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

	glm::vec3 coords_nn =
		static_cast<glm::vec3>(face.internal_coords) - glm::vec3{0.5f, 0.5f, 0.5f};
	coords_nn[index_axis] += face.negativeward ? 0.0f : 1.0f;

	VertexDataClassic nn;
	nn.coords = coords_nn;
	nn.coords[index_a] += 0.0f;
	nn.coords[index_b] += 0.0f;
	nn.normal = normal;
	nn.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	nn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	nn.atlas_coords_min = atlas_coords_min_real;
	nn.atlas_coords_max = atlas_coords_max_real;
	nn.ambiant_occlusion = 1.0f;
	VertexDataClassic np;
	np.coords = coords_nn;
	np.coords[index_a] += 0.0f;
	np.coords[index_b] += 1.0f;
	np.normal = normal;
	np.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	np.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	np.atlas_coords_min = atlas_coords_min_real;
	np.atlas_coords_max = atlas_coords_max_real;
	np.ambiant_occlusion = 1.0f;
	VertexDataClassic pn;
	pn.coords = coords_nn;
	pn.coords[index_a] += 1.0f;
	pn.coords[index_b] += 0.0f;
	pn.normal = normal;
	pn.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	pn.atlas_coords_min = atlas_coords_min_real;
	pn.atlas_coords_max = atlas_coords_max_real;
	pn.ambiant_occlusion = 1.0f;
	VertexDataClassic pp;
	pp.coords = coords_nn;
	pp.coords[index_a] += 1.0f;
	pp.coords[index_b] += 1.0f;
	pp.normal = normal;
	pp.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pp.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	pp.atlas_coords_min = atlas_coords_min_real;
	pp.atlas_coords_max = atlas_coords_max_real;
	pp.ambiant_occlusion = 1.0f;

	std::array<VertexDataClassic, 6> const vertex_data_sequence{nn, pn, pp, nn, pp, np};
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

IsolatedChunk* generate_chunk(ChunkCoords chunk_coords, BlockRect rect, Nature const& nature)
{
	IsolatedChunk* chunk = new IsolatedChunk{};
	chunk->chunk_coords = chunk_coords;
	chunk->rect = rect;
	chunk->is_all_air = false;
	chunk->block_grid.resize(chunk->rect.volume());

	/* Generate content. */
	nature.world_generator.generate_chunk_content(nature, *chunk);

	/* Generate mesh data. */
	for (BlockCoords const& walker : chunk->rect)
	{
		Block& block = chunk->block_grid[chunk->rect.to_index(walker)];
		if (not block.is_air)
		{
			for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
			for (bool negativeward : {false, true})
			{
				BlockCoords neighbor = walker;
				neighbor[static_cast<int>(axis)] += negativeward ? -1 : 1;
				if (chunk->rect.contains(neighbor) &&
					chunk->block_grid[chunk->rect.to_index(neighbor)].is_air)
				{
					block.generate_face(nature,
						BlockFace{walker, axis, negativeward},
						chunk->vertex_data);
				}
			}
		}
	}

	return chunk;
}

Chunk::Chunk(BlockRect rect):
	rect{rect}, is_generated{false}, is_just_generated{false}
{
	;
}

Block& Chunk::block(BlockCoords const& coords)
{
	assert(this->rect.contains(coords));
	return this->block_grid[this->rect.to_index(coords)];
}

Block const& Chunk::block(BlockCoords const& coords) const
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

	//this->mesh.update_opengl_data();
	this->mesh.needs_update_opengl_data = true;
}

void Chunk::add_common_faces_to_mesh(Nature const& nature,
	ChunkFace chunk_face, Chunk& touching_chunk)
{
	unsigned int const index_axis = static_cast<int>(chunk_face.axis);
	unsigned int const index_a = index_axis == 0 ? 1 : 0;
	unsigned int const index_b = index_axis == 2 ? 1 : 2;

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

	//this->mesh.update_opengl_data();
	this->mesh.needs_update_opengl_data = true;
}

ChunkGrid::ChunkGrid()
{
	;
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

Chunk const* ChunkGrid::chunk(ChunkCoords chunk_coords) const
{
	auto chunk_maybe = this->table.find(chunk_coords);
	if (chunk_maybe == this->table.end())
	{
		return nullptr;
	}
	else
	{
		Chunk const* chunk = chunk_maybe->second;
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

Chunk const* ChunkGrid::containing_chunk(BlockCoords coords) const
{
	ChunkCoords const chunk_coords = this->containing_chunk_coords(coords);
	Chunk const* chunk = this->chunk(chunk_coords);
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

Chunk const* ChunkGrid::containing_chunk(glm::vec3 coords) const
{
	BlockCoords const coords_int{
		static_cast<int>(std::round(coords.x)),
		static_cast<int>(std::round(coords.y)),
		static_cast<int>(std::round(coords.z))};
	return this->containing_chunk(coords_int);
}

Chunk* ChunkGrid::add_generated_chunk(IsolatedChunk* generating_chunk, ChunkCoords chunk_coords,
	Nature const& nature)
{
	Chunk* chunk = this->table[chunk_coords] = new Chunk{generating_chunk->rect};
	chunk->is_generated = true;
	chunk->is_just_generated = true;

	chunk->block_grid = std::move(generating_chunk->block_grid);

	chunk->mesh.vertex_data = std::move(generating_chunk->vertex_data);
	chunk->mesh.needs_update_opengl_data = true;

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

bool ChunkGrid::block_is_air_or_not_generated(BlockCoords coords) const
{
	Chunk const* chunk = this->containing_chunk(coords);
	if (chunk == nullptr)
	{
		return true;
	}
	else
	{
		return chunk->block(coords).is_air;
	}
}

} /* Qwy2 */
