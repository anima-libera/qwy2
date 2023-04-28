
#include "chunk.hpp"
#include "nature.hpp"
#include "terrain_gen.hpp"
#include "threadpool.hpp"
#include "utils.hpp"
#include "structure.hpp"
#include "gameloop.hpp"
#include "entity.hpp"
#include <cassert>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>

namespace qwy2
{

/* TODO: Reduce code duplication accross this file. */

BlockCoords chunk_center_coords(ChunkCoords chunk_coords)
{
	return BlockCoords{
		chunk_coords.x * static_cast<int>(g_game->chunk_side),
		chunk_coords.y * static_cast<int>(g_game->chunk_side),
		chunk_coords.z * static_cast<int>(g_game->chunk_side)};
}

BlockCoords chunk_most_negativeward_block_coords(ChunkCoords chunk_coords)
{
	return chunk_center_coords(chunk_coords) - BlockCoords{
		static_cast<int>(g_game->chunk_side) / 2,
		static_cast<int>(g_game->chunk_side) / 2,
		static_cast<int>(g_game->chunk_side) / 2};
}

BlockCoords chunk_most_positiveward_block_coords(ChunkCoords chunk_coords)
{
	return chunk_center_coords(chunk_coords) + BlockCoords{
		static_cast<int>(g_game->chunk_side) / 2,
		static_cast<int>(g_game->chunk_side) / 2,
		static_cast<int>(g_game->chunk_side) / 2};
}

BlockRect chunk_block_rect(ChunkCoords chunk_coords)
{
	BlockCoords const center_coords = chunk_center_coords(chunk_coords);
	int const margin = g_game->chunk_side / 2;
	return BlockRect{
		BlockCoords{center_coords.x - margin, center_coords.y - margin, center_coords.z - margin},
		BlockCoords{center_coords.x + margin, center_coords.y + margin, center_coords.z + margin}};
}

BlockRect chunk_rect_block_rect(ChunkRect chunk_rect)
{
	BlockCoords const center_coords_min = chunk_center_coords(chunk_rect.coords_min);
	BlockCoords const center_coords_max = chunk_center_coords(chunk_rect.coords_max);
	int const margin = g_game->chunk_side / 2;
	return BlockRect{
		BlockCoords{
			center_coords_min.x - margin,
			center_coords_min.y - margin,
			center_coords_min.z - margin},
		BlockCoords{
			center_coords_max.x + margin,
			center_coords_max.y + margin,
			center_coords_max.z + margin}};
}

ChunkCoords containing_chunk_coords(BlockCoords coords)
{
	int const chunk_side_int = static_cast<int>(g_game->chunk_side);
	return ChunkCoords{
		(coords.x + (coords.x < 0 ? -1 : 1) * chunk_side_int / 2) / chunk_side_int,
		(coords.y + (coords.y < 0 ? -1 : 1) * chunk_side_int / 2) / chunk_side_int,
		(coords.z + (coords.z < 0 ? -1 : 1) * chunk_side_int / 2) / chunk_side_int};
}

ChunkCoords containing_chunk_coords(glm::vec3 coords)
{
	return containing_chunk_coords(
		BlockCoords{
			static_cast<int>(std::round(coords.x)),
			static_cast<int>(std::round(coords.y)),
			static_cast<int>(std::round(coords.z))});
}

ChunkRect containing_chunk_rect(BlockRect block_rect)
{
	return ChunkRect{
		containing_chunk_coords(block_rect.coords_min),
		containing_chunk_coords(block_rect.coords_max)};
}

AlignedBox block_rect_box(BlockRect block_rect)
{
	glm::vec3 const coords_min =
		static_cast<glm::vec3>(block_rect.coords_min) - glm::vec3{0.5f, 0.5f, 0.5f};
	glm::vec3 const coords_max =
		static_cast<glm::vec3>(block_rect.coords_max) + glm::vec3{0.5f, 0.5f, 0.5f};
	return AlignedBox{(coords_min + coords_max) / 2.0f, coords_max - coords_min};
}

inline static unsigned int chunk_volume()
{
	return g_game->chunk_side * g_game->chunk_side * g_game->chunk_side;
}

template<typename FieldValueType>
ChunkField<FieldValueType>::ChunkField():
	chunk_coords{0, 0, 0},
	data{nullptr}
{
	;
}

template<typename FieldValueType>
ChunkField<FieldValueType>::ChunkField(ChunkCoords chunk_coords):
	chunk_coords{chunk_coords},
	data{new FieldValueType[chunk_volume()]}
{
	;
}

template<typename FieldValueType>
ChunkField<FieldValueType>::ChunkField(ChunkCoords chunk_coords, ValueType* data):
	chunk_coords{chunk_coords},
	data{data}
{
	;
}

template<typename FieldValueType>
ChunkField<FieldValueType>::~ChunkField()
{
	//delete[] this->data;
}

template<typename FieldValueType>
FieldValueType& ChunkField<FieldValueType>::operator[](BlockCoords coords)
{
	BlockCoords const local_coords =
		coords - chunk_most_negativeward_block_coords(this->chunk_coords);
	return this->data[
		local_coords.x +
		g_game->chunk_side * local_coords.y + 
		g_game->chunk_side * g_game->chunk_side * local_coords.z];
}

template<typename FieldValueType>
FieldValueType const& ChunkField<FieldValueType>::operator[](BlockCoords coords) const
{
	BlockCoords const local_coords =
		coords - chunk_most_negativeward_block_coords(this->chunk_coords);
	return this->data[
		local_coords.x +
		g_game->chunk_side * local_coords.y + 
		g_game->chunk_side * g_game->chunk_side * local_coords.z];
}

template<typename FieldValueType>
FieldValueType* ChunkField<FieldValueType>::raw_data()
{
	return this->data;
}

template<typename FieldValueType>
void ChunkField<FieldValueType>::free_data()
{
	delete[] this->data;
}

template ChunkField<PtgFieldValue>::ChunkField(ChunkCoords chunk_coords);
template ChunkField<PtgFieldValue>::~ChunkField();
template PtgFieldValue& ChunkField<PtgFieldValue>::operator[](BlockCoords coords);
template PtgFieldValue const& ChunkField<PtgFieldValue>::operator[](BlockCoords coords) const;
template PtgFieldValue* ChunkField<PtgFieldValue>::raw_data();
template void ChunkField<PtgFieldValue>::free_data();

template void ChunkField<PttFieldValue>::free_data();

template ChunkField<Block>::~ChunkField();
template Block& ChunkField<Block>::operator[](BlockCoords coords);
template Block const& ChunkField<Block>::operator[](BlockCoords coords) const;
template Block* ChunkField<Block>::raw_data();
template void ChunkField<Block>::free_data();

bool Block::is_air() const
{
	return this->type_id == 0;
}

template<typename ChunkFieldType>
ChunkNeighborhood<ChunkFieldType>::ChunkNeighborhood()
{
	;
}

template<typename ChunkFieldType>
typename ChunkNeighborhood<ChunkFieldType>::ValueType&
	ChunkNeighborhood<ChunkFieldType>::operator[](BlockCoords coords)
{
	ChunkCoords const chunk_coords = containing_chunk_coords(coords);
	ChunkCoords const negativeward_chunk_coords = this->field_table[0].chunk_coords;
	ChunkCoords const local_chunk_coords = chunk_coords - negativeward_chunk_coords;
	ChunkFieldType& field = this->field_table[
		local_chunk_coords.x +
		local_chunk_coords.y * 3 +
		local_chunk_coords.z * 3 * 3];
	return field[coords];
}

template<typename ChunkFieldType>
typename ChunkNeighborhood<ChunkFieldType>::ValueType const&
	ChunkNeighborhood<ChunkFieldType>::operator[](BlockCoords coords) const
{
	ChunkCoords const chunk_coords = containing_chunk_coords(coords);
	ChunkCoords const negativeward_chunk_coords = this->field_table[0].chunk_coords;
	ChunkCoords const local_chunk_coords = chunk_coords - negativeward_chunk_coords;
	ChunkFieldType const& field = this->field_table[
		local_chunk_coords.x +
		local_chunk_coords.y * 3 +
		local_chunk_coords.z * 3 * 3];
	return field[coords];
}

template<typename ChunkFieldType>
ChunkRect ChunkNeighborhood<ChunkFieldType>::chunk_rect() const
{
	return ChunkRect{
		this->field_table[0].chunk_coords,
		this->field_table[3*3*3-1].chunk_coords};
}

template ChunkNeighborhood<ChunkPttField>::ValueType const&
	ChunkNeighborhood<ChunkPttField>::operator[](BlockCoords coords) const;
template ChunkRect ChunkNeighborhood<ChunkPttField>::chunk_rect() const;

ChunkPtgField generate_chunk_ptg_field(
	ChunkCoords chunk_coords,
	[[maybe_unused]] Nature const& nature)
{
	return nature.world_generator.plain_terrain_generator->generate_chunk_ptg_field(
		chunk_coords, nature);
}

ChunkPttField generate_chunk_ptt_field(
	ChunkCoords chunk_coords,
	ChunkNeighborhood<ChunkPtgField> const chunk_neighborhood_ptg_field,
	[[maybe_unused]] Nature const& nature)
{
	/* Placeholder. */
	ChunkPttField ptt_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		if (chunk_neighborhood_ptg_field[coords] >= 2)
		{
			ptt_field[coords] = 3 + chunk_neighborhood_ptg_field[coords];
		}
		else if (chunk_neighborhood_ptg_field[coords] == 0)
		{
			ptt_field[coords] = 0;
		}
		else if (chunk_neighborhood_ptg_field[coords + BlockCoords{0, 0, 1}] == 0 &&
			not nature.world_generator.stone_terrain)
		{
			ptt_field[coords] = 1;
		}
		else if (chunk_neighborhood_ptg_field[coords + BlockCoords{0, 0, 2}] == 0 &&
			not nature.world_generator.stone_terrain)
		{
			ptt_field[coords] = 2;
		}
		else
		{
			ptt_field[coords] = 3;
		}
	}
	return ptt_field;
}

ChunkBField generate_chunk_b_field(
	ChunkCoords chunk_coords,
	ChunkNeighborhood<ChunkPttField> const chunk_neighborhood_ptt_field,
	[[maybe_unused]] Nature const& nature)
{
	/* Placeholder. */
	ChunkBField b_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		b_field[coords].type_id = chunk_neighborhood_ptt_field[coords];
	}

	if (nature.world_generator.structures_enabled)
	{
		int const structure_bound_rect_max_radius = 12;
		BlockRect structure_head_rect_of_influence =
			chunk_rect_block_rect(chunk_neighborhood_ptt_field.chunk_rect());
		structure_head_rect_of_influence.coords_min.x += structure_bound_rect_max_radius;
		structure_head_rect_of_influence.coords_min.y += structure_bound_rect_max_radius;
		structure_head_rect_of_influence.coords_min.z += structure_bound_rect_max_radius;
		structure_head_rect_of_influence.coords_max.x -= structure_bound_rect_max_radius;
		structure_head_rect_of_influence.coords_max.y -= structure_bound_rect_max_radius;
		structure_head_rect_of_influence.coords_max.z -= structure_bound_rect_max_radius;

		/* Placeholder. */
		for (BlockCoords coords : structure_head_rect_of_influence)
		{
			float const coords_noise = nature.world_generator.noise_generator.base_noise(
				coords.x, coords.y, coords.z);
			float const structure_probability = 0.0001f;
			if (coords_noise < structure_probability)
			{
				BlockCoords head_start = coords;
				BlockRect bound_rect{head_start, structure_bound_rect_max_radius - 3};
				StructureInstance structure{head_start, bound_rect};
				structure.generate(b_field, chunk_coords, chunk_neighborhood_ptt_field, nature);
			}
		}
	}

	return b_field;
}

namespace
{

/* Generates a piece of mesh data corresponding to the given face of the given block,
 * and adds it to the given mesh. */
void generate_block_face_in_mesh(
	BlockFace const& face,
	Block const& block,
	ChunkNeighborhood<ChunkBField> const chunk_neighborhood_b_field,
	Nature const& nature,
	ChunkMeshData& mesh)
{
	auto const [index_axis, index_a, index_b] = face.indices_axis_a_b();

	BlockType const& type = nature.block_type_table[block.type_id];
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

	/* The ambiant occlusion trick used here was taken from 
	 * https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
	 * this cool blog post seem to be famous in the voxel engine scene. */

	auto const vertex_ambiant_occlusion = []
		(bool side_a, bool side_b, bool corner_ab)
		{
			if (side_a && side_b)
			{
				return 0;
			}
			else
			{
				int const sum =
					static_cast<int>(side_a) +
					static_cast<int>(side_b) +
					static_cast<int>(corner_ab);
				return 3 - sum;
			}
		};
	auto const vertex_ambiant_occlusion_TODO =
		/* TODO: Make all this less... wierdly presented? */
		[vertex_ambiant_occlusion, chunk_neighborhood_b_field, face,
			index_a=index_a, index_b=index_b]
		(int a, int b)
		{
			BlockCoords const external_coords = face.external_coords();
			BlockCoords coords;
			coords = external_coords;
			coords[index_a] += a;
			bool const side_a = not chunk_neighborhood_b_field[coords].is_air();
			coords = external_coords;
			coords[index_b] += b;
			bool const side_b = not chunk_neighborhood_b_field[coords].is_air();
			coords = external_coords;
			coords[index_a] += a;
			coords[index_b] += b;
			bool const corner_ab = not chunk_neighborhood_b_field[coords].is_air();
			int const value = vertex_ambiant_occlusion(side_a, side_b, corner_ab);
			return static_cast<float>(value) / 3.0f;
		};

	VertexDataClassic nn;
	nn.coords = coords_nn;
	nn.coords[index_a] += 0.0f;
	nn.coords[index_b] += 0.0f;
	nn.normal = normal;
	nn.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	nn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	nn.atlas_coords_min = atlas_coords_min_real;
	nn.atlas_coords_max = atlas_coords_max_real;
	nn.ambiant_occlusion = vertex_ambiant_occlusion_TODO(-1, -1);
	VertexDataClassic np;
	np.coords = coords_nn;
	np.coords[index_a] += 0.0f;
	np.coords[index_b] += 1.0f;
	np.normal = normal;
	np.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	np.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	np.atlas_coords_min = atlas_coords_min_real;
	np.atlas_coords_max = atlas_coords_max_real;
	np.ambiant_occlusion = vertex_ambiant_occlusion_TODO(-1, +1);
	VertexDataClassic pn;
	pn.coords = coords_nn;
	pn.coords[index_a] += 1.0f;
	pn.coords[index_b] += 0.0f;
	pn.normal = normal;
	pn.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	pn.atlas_coords_min = atlas_coords_min_real;
	pn.atlas_coords_max = atlas_coords_max_real;
	pn.ambiant_occlusion = vertex_ambiant_occlusion_TODO(+1, -1);
	VertexDataClassic pp;
	pp.coords = coords_nn;
	pp.coords[index_a] += 1.0f;
	pp.coords[index_b] += 1.0f;
	pp.normal = normal;
	pp.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pp.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	pp.atlas_coords_min = atlas_coords_min_real;
	pp.atlas_coords_max = atlas_coords_max_real;
	pp.ambiant_occlusion = vertex_ambiant_occlusion_TODO(+1, +1);

	/* The four vertices currently corming a square are now being given as two triangles.
	 * The diagonal of the square (nn-pp or np-pn) that will be the "cut" in the square
	 * to form triangles is selected based on the ambiant occlusion values of the vertices.
	 * Depending on the diagonal picked, the ambiant occlusion behaves differently on the face,
	 * and we make sure that this behavior is consistent. */
	bool const other_triangle_cut =
		nn.ambiant_occlusion + pp.ambiant_occlusion <= np.ambiant_occlusion + pn.ambiant_occlusion;
	std::array<VertexDataClassic, 6> const vertex_data_sequence =
		other_triangle_cut ?
			std::array<VertexDataClassic, 6>{np, nn, pn, np, pn, pp} :
			std::array<VertexDataClassic, 6>{nn, pn, pp, nn, pp, np};
	
	/* Does std::copy preallocate the appropriate size ? Probably...
	 * TODO: Find out and remove this std::vector::reserve call if redundant. */
	mesh.reserve(mesh.size() + vertex_data_sequence.size());
	if (reverse_vertex_order)
	{
		std::copy(vertex_data_sequence.rbegin(), vertex_data_sequence.rend(),
			std::back_inserter(mesh));
	}
	else
	{
		std::copy(vertex_data_sequence.begin(), vertex_data_sequence.end(),
			std::back_inserter(mesh));
	}
}

} /* Anonymous namespace. */

ChunkMeshData* generate_chunk_complete_mesh(
	ChunkCoords chunk_coords,
	ChunkNeighborhood<ChunkBField> const chunk_neighborhood_b_field,
	Nature const& nature)
{
	ChunkMeshData* mesh_data = new ChunkMeshData{};
	for (BlockCoords coords_interior : chunk_block_rect(chunk_coords))
	{
		Block const& block_interior = chunk_neighborhood_b_field[coords_interior];
		if (block_interior.is_air())
		{
			continue;
		}

		for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
		for (bool negativeward : {false, true})
		{
			BlockCoords coords_exterior;
			coords_exterior = coords_interior;
			coords_exterior[static_cast<unsigned int>(axis)] += negativeward ? -1 : +1;
			Block const& block_exterior = chunk_neighborhood_b_field[coords_exterior];
			if (not block_exterior.is_air())
			{
				continue;
			}

			generate_block_face_in_mesh(
				BlockFace{coords_interior, axis, negativeward},
				block_interior,
				chunk_neighborhood_b_field,
				nature,
				*mesh_data);
		}
	}
	return mesh_data;
}

ChunkEntityTable::ChunkEntityTable()
{
	;	
}

namespace
{

std::string chunk_file_name(ChunkCoords chunk_coords)
{
	std::stringstream file_name_stream;
	file_name_stream << "saves/" << g_game->save_name << "/chunks/chunk_"
		<< chunk_coords.x << "_" << chunk_coords.y << "_" << chunk_coords.z
		<< ".qwy2_chunk";
	return file_name_stream.str();
}

} /* Anonymous namespace. */

ChunkDiskStorage::ChunkDiskStorage()
{
	assert(false
		/* This default constructor is required by the `std::unsorted_map<ChunkDiskStorage>`
		 * but it makes no sense and should never be called. */);
	std::exit(EXIT_FAILURE);
}

ChunkDiskStorage::ChunkDiskStorage(ChunkCoords chunk_coords):
	chunk_coords(chunk_coords), modified(false), file_name(chunk_file_name(chunk_coords))
{
	this->exist = std::ifstream{this->file_name, std::ifstream::binary}.good();
}

ChunkDiskStorage search_disk_for_chunk(ChunkCoords chunk_coords)
{
	ChunkDiskStorage storage{chunk_coords};
	return storage;
}

ChunkBField read_disk_chunk_b_field(ChunkCoords chunk_coords,
	ChunkDiskStorage& chunk_disk_storage)
{
	/* TODO: Make this better. */
	unsigned int const size = sizeof (Block) * chunk_volume();
	Block* b_field_data = static_cast<Block*>(operator new(size));
	std::fstream file{chunk_disk_storage.file_name,
		std::ios::binary | std::ios::in | std::ios::out};
	file.read(static_cast<char*>(static_cast<void*>(b_field_data)), size);
	ChunkBField b_field{chunk_coords, b_field_data};
	return b_field;
}

void write_disk_chunk_b_field(ChunkCoords chunk_coords,
	ChunkDiskStorage& chunk_disk_storage, ChunkBField chunk_b_field)
{
	/* TODO: Make this better. */
	//std::cout << chunk_disk_storage.file_name << std::endl;
	std::fstream file;
	if (chunk_disk_storage.exist)
	{
		file.open(chunk_disk_storage.file_name,
			std::ios::binary | std::ios::in | std::ios::out);
	}
	else
	{
		file.open(chunk_disk_storage.file_name,
			std::ios::binary | std::ios::out);
	}
	file.seekp(0, std::ios_base::beg);
	unsigned int const size = sizeof (Block) * chunk_volume();
	file.write(static_cast<char*>(static_cast<void*>(chunk_b_field.raw_data())), size);
	file.flush();
	file.close();

	chunk_disk_storage.modified = false;
}

bool ChunkGrid::has_ptg_field(ChunkCoords chunk_coords) const
{
	return this->ptg_field.find(chunk_coords) != this->ptg_field.end();
}

bool ChunkGrid::has_ptt_field(ChunkCoords chunk_coords) const
{
	return this->ptt_field.find(chunk_coords) != this->ptt_field.end();
}

bool ChunkGrid::has_b_field(ChunkCoords chunk_coords) const
{
	return this->b_field.find(chunk_coords) != this->b_field.end();
}

bool ChunkGrid::has_complete_mesh(ChunkCoords chunk_coords) const
{
	return this->mesh.find(chunk_coords) != this->mesh.end();
}

bool ChunkGrid::has_entity_table(ChunkCoords chunk_coords) const
{
	return this->entity_table.find(chunk_coords) != this->entity_table.end();
}

bool ChunkGrid::has_disk_storage(ChunkCoords chunk_coords) const
{
	return this->disk.find(chunk_coords) != this->disk.end();
}

bool ChunkGrid::has_ptg_field_neighborhood(ChunkCoords center_chunk_coords) const
{
	for (int dx = -1; dx <= 1; dx++)
	for (int dy = -1; dy <= 1; dy++)
	for (int dz = -1; dz <= 1; dz++)
	{
		ChunkCoords const chunk_coords = center_chunk_coords + ChunkCoords{dx, dy, dz};
		auto iterator = this->ptg_field.find(chunk_coords);
		if (iterator == this->ptg_field.end())
		{
			return false;
		}
	}
	return true;
}

bool ChunkGrid::has_ptt_field_neighborhood(ChunkCoords center_chunk_coords) const
{
	for (int dx = -1; dx <= 1; dx++)
	for (int dy = -1; dy <= 1; dy++)
	for (int dz = -1; dz <= 1; dz++)
	{
		ChunkCoords const chunk_coords = center_chunk_coords + ChunkCoords{dx, dy, dz};
		auto iterator = this->ptt_field.find(chunk_coords);
		if (iterator == this->ptt_field.end())
		{
			return false;
		}
	}
	return true;
}

bool ChunkGrid::has_b_field_neighborhood(ChunkCoords center_chunk_coords) const
{
	for (int dx = -1; dx <= 1; dx++)
	for (int dy = -1; dy <= 1; dy++)
	for (int dz = -1; dz <= 1; dz++)
	{
		ChunkCoords const chunk_coords = center_chunk_coords + ChunkCoords{dx, dy, dz};
		auto iterator = this->b_field.find(chunk_coords);
		if (iterator == this->b_field.end())
		{
			return false;
		}
	}
	return true;
}

ChunkNeighborhood<ChunkPtgField> const
	ChunkGrid::get_ptg_field_neighborhood(ChunkCoords center_chunk_coords) const
{
	ChunkNeighborhood<ChunkPtgField> ptg_field_neighborhood;
	for (int dx = -1; dx <= 1; dx++)
	for (int dy = -1; dy <= 1; dy++)
	for (int dz = -1; dz <= 1; dz++)
	{
		ChunkCoords const chunk_coords = center_chunk_coords + ChunkCoords{dx, dy, dz};
		auto iterator = this->ptg_field.find(chunk_coords);
		assert(iterator != this->ptg_field.end());
		ChunkCoords const local_coords = ChunkCoords{dx + 1, dy + 1, dz + 1};
		unsigned int const index = local_coords.x + local_coords.y * 3 + local_coords.z * 3 * 3;
		ptg_field_neighborhood.field_table[index] = iterator->second;
	}
	return ptg_field_neighborhood;
}

ChunkNeighborhood<ChunkPttField> const
	ChunkGrid::get_ptt_field_neighborhood(ChunkCoords center_chunk_coords) const
{
	ChunkNeighborhood<ChunkPttField> ptt_field_neighborhood;
	for (int dx = -1; dx <= 1; dx++)
	for (int dy = -1; dy <= 1; dy++)
	for (int dz = -1; dz <= 1; dz++)
	{
		ChunkCoords const chunk_coords = center_chunk_coords + ChunkCoords{dx, dy, dz};
		auto iterator = this->ptt_field.find(chunk_coords);
		assert(iterator != this->ptt_field.end());
		ChunkCoords const local_coords = ChunkCoords{dx + 1, dy + 1, dz + 1};
		unsigned int const index = local_coords.x + local_coords.y * 3 + local_coords.z * 3 * 3;
		ptt_field_neighborhood.field_table[index] = iterator->second;
	}
	return ptt_field_neighborhood;
}

ChunkNeighborhood<ChunkBField> const
	ChunkGrid::get_b_field_neighborhood(ChunkCoords center_chunk_coords) const
{
	ChunkNeighborhood<ChunkBField> b_field_neighborhood;
	for (int dx = -1; dx <= 1; dx++)
	for (int dy = -1; dy <= 1; dy++)
	for (int dz = -1; dz <= 1; dz++)
	{
		ChunkCoords const chunk_coords = center_chunk_coords + ChunkCoords{dx, dy, dz};
		auto iterator = this->b_field.find(chunk_coords);
		assert(iterator != this->b_field.end());
		ChunkCoords const local_coords = ChunkCoords{dx + 1, dy + 1, dz + 1};
		unsigned int const index = local_coords.x + local_coords.y * 3 + local_coords.z * 3 * 3;
		b_field_neighborhood.field_table[index] = iterator->second;
	}
	return b_field_neighborhood;
}

bool ChunkGrid::block_is_air_or_unloaded(BlockCoords coords) const
{
	ChunkCoords const chunk_coords = containing_chunk_coords(coords);
	auto iterator = this->b_field.find(chunk_coords);
	if (iterator != this->b_field.end())
	{
		return iterator->second[coords].is_air();
	}
	else
	{
		return true;
	}
}

void ChunkGrid::set_block(Nature const* nature,
	BlockCoords coords, BlockTypeId new_type_id)
{
	ChunkCoords const chunk_coords = containing_chunk_coords(coords);

	/* Modify the B field (the actual blocks). */
	ChunkBField& b_field = this->b_field.at(chunk_coords);
	b_field[coords].type_id = new_type_id;

	/* Update the meshes.
	 * Due to concerns such as ambiant occlusion, nearby chunks may
	 * also have to also be remeshed. */
	BlockRect const concerned_blocks{coords, 2};
	ChunkRect const concerned_chunks =
		containing_chunk_rect(concerned_blocks);
	for (ChunkCoords const chunk_coords : concerned_chunks)
	{
		/* TODO: Make the remeshing in other threads with highest priority or something.
		 * Without this, when the remeshing happens to take too long (it happens sometimes)
		 * it blocks the main thread and the game freezes for a bit. */
		
		Mesh<VertexDataClassic>& mesh =
			this->mesh.at(chunk_coords);
		ChunkMeshData* data = generate_chunk_complete_mesh(chunk_coords,
			this->get_b_field_neighborhood(chunk_coords),
			*nature);
		mesh.vertex_data = std::move(*data);
		mesh.needs_update_opengl_data = true;
	}

	/* Mark the chunk as modified, which makes sure it will be saved to the disk (if required). */
	if (not this->has_disk_storage(chunk_coords))
	{
		this->disk.insert(std::make_pair(chunk_coords, ChunkDiskStorage{chunk_coords}));
	}
	ChunkDiskStorage& chunk_disk_storage = this->disk.at(chunk_coords);
	chunk_disk_storage.modified = true;
}

void ChunkGrid::add_entity(Entity* entity)
{
	BlockCoords coords{entity->coords};
	ChunkCoords chunk_coords = containing_chunk_coords(coords);
	if (not this->has_entity_table(chunk_coords))
	{
		this->entity_table.insert(std::make_pair(chunk_coords, ChunkEntityTable{}));
	}
	ChunkEntityTable& chunk_entity_table = this->entity_table.at(chunk_coords);
	
	for (Entity*& entry : chunk_entity_table.entities)
	{
		if (entry == nullptr)
		{
			entry = entity;
			return;
		}
	}
	chunk_entity_table.entities.push_back(entity);
}

void ChunkGrid::unload(ChunkCoords chunk_coords)
{
	std::cout << "Unload chunk " << chunk_coords << std::endl;
	if (this->has_ptg_field(chunk_coords))
	{
		auto it = this->ptg_field.find(chunk_coords);
		it->second.free_data();
		this->ptg_field.erase(it);
	}
	if (this->has_ptt_field(chunk_coords))
	{
		auto it = this->ptt_field.find(chunk_coords);
		it->second.free_data();
		this->ptt_field.erase(it);
	}
	if (this->has_b_field(chunk_coords))
	{
		this->save_b_field_if_necessary(chunk_coords);
		auto it = this->b_field.find(chunk_coords);
		it->second.free_data();
		this->b_field.erase(it);
	}
	if (this->has_complete_mesh(chunk_coords))
	{
		/* The destruction of the OpenGL buffer and the freeing of the CPU-side buffer
		 * is done in the `Mesh` destructor. */
		this->mesh.erase(this->mesh.find(chunk_coords));
	}
	if (this->has_disk_storage(chunk_coords))
	{
		this->disk.erase(this->disk.find(chunk_coords));
	}
}

void ChunkGrid::save_b_field_if_necessary(ChunkCoords chunk_coords)
{
	if (not this->has_disk_storage(chunk_coords))
	{
		this->disk.insert(std::make_pair(chunk_coords,ChunkDiskStorage{chunk_coords}));
	}
	ChunkDiskStorage& chunk_disk_storage = this->disk.at(chunk_coords);

	if (chunk_disk_storage.modified ||
		(not g_game->chunk_generation_manager.save_only_modified))
	{
		if (this->has_b_field(chunk_coords))
		{
			ChunkBField& b_field = this->b_field.at(chunk_coords);
			write_disk_chunk_b_field(chunk_coords, chunk_disk_storage, b_field);
		}
	}
}

void ChunkGrid::save_all_that_is_necessary()
{
	for (auto& [chunk_coords, b_field] : this->b_field)
	{
		this->save_b_field_if_necessary(chunk_coords);
	}
}

ChunkGenerationManager::ChunkGenerationManager():
	thread_pool{nullptr},
	chunk_grid{nullptr},
	generation_center{0.0f, 0.0f, 0.0f},
	generation_radius{50.0f},
	generation_enabled{true}
{
	;
}

void ChunkGenerationManager::manage(Nature const& nature)
{
	/* Priority to unloading chunks that are too far.
	 * It seems fast enough for now that it can be done in the main thread. */
	std::vector<ChunkCoords> chunks_to_unload;
	auto consider_unloading = [this, &chunks_to_unload](ChunkCoords chunk_coords){
		BlockCoords center_coords = chunk_center_coords(chunk_coords);
		float dist = glm::distance(
			static_cast<glm::vec3>(center_coords), this->generation_center);
		dist += g_game->chunk_side; /* Just to be sure the chunk is completely out. */
		if (this->generation_radius + this->unloading_margin < dist)
		{
			chunks_to_unload.push_back(chunk_coords);
		}
	};
	/* Should cover chunks that have a B field and chunks that have a mesh. */
	for (auto& [chunk_coords, field] : this->chunk_grid->b_field)
	{
		consider_unloading(chunk_coords);
	}
	/* Should cover chunks that have a PTG field and chunks that have a PTT field. */
	for (auto& [chunk_coords, field] : this->chunk_grid->ptg_field)
	{
		consider_unloading(chunk_coords);
	}
	/* Unloading a chunk invalidates iterators on the affected `std::unsorted_map`s,
	 * thus instead we iterate over a vector of chunk coordinates. */
	for (ChunkCoords chunk_coords : chunks_to_unload)
	{
		this->chunk_grid->unload(chunk_coords);
	}

	unsigned int const chunk_generation_radius = 1 + static_cast<unsigned int>(
		this->generation_radius / static_cast<float>(g_game->chunk_side));
	ChunkCoords const chunk_generation_center =
		containing_chunk_coords(this->generation_center);
	/* Cube of chunks that contains the zone to be generated. */
	ChunkRect const chunk_generation_rect =
		ChunkRect{chunk_generation_center, chunk_generation_radius};

	/* Make the list of chunks that are to be generated. */
	std::vector<ChunkCoords> chunk_generation_candidates;
	for (ChunkCoords const& walker : chunk_generation_rect)
	{
		if (this->chunk_grid->has_complete_mesh(walker))
		{
			/* Don't generate already generated chunks. */
			continue;
		}
		float const distance_to_center = glm::distance(
			static_cast<glm::vec3>(chunk_center_coords(walker)), this->generation_center);
		if (distance_to_center > this->generation_radius + g_game->chunk_side)
		{
			/* Don't generate chunks that are too far.
			 * The condition leaves a little room for chunks too far to get generated anyway,
			 * but it does not matter that much. */
			continue;
		}
		chunk_generation_candidates.push_back(walker);
	}

	/* Sort the candidate chunks per distance to the generation center
	 * to prioritize the closest chunks.
	 * The closest chunks are at the end of the vector for the popping to be fast. */
	std::sort(chunk_generation_candidates.begin(), chunk_generation_candidates.end(),
		[
			generation_center = this->generation_center
		](
			ChunkCoords const& left, ChunkCoords const& right
		){
			glm::vec3 const left_center =
				static_cast<glm::vec3>(left) * static_cast<float>(g_game->chunk_side);
			float const left_distance = glm::distance(left_center, generation_center);
			glm::vec3 const right_center =
				static_cast<glm::vec3>(right) * static_cast<float>(g_game->chunk_side);
			float const right_distance = glm::distance(right_center, generation_center);
			/* Note: Make sure that this order is strict (ie that it returns false when
			 * left and right are equal) or else std::sort will proceed to perform some
			 * undefined behavior (that corrupts memory on my machine ><). */
			return left_distance > right_distance;
		});
	
	for (std::optional<ChunkGeneratingData>& generating_data_opt : this->generating_data_vector)
	{
		if (generating_data_opt.has_value())
		{
			using namespace std::chrono_literals;
			ChunkGeneratingData& generating_data = generating_data_opt.value();
			if (generating_data.future.valid()
				&& generating_data.future.wait_for(0s) == std::future_status::ready)
			{
				/* Some data generation has been finished,
				 * it is time to pick it up and place it in the chunk grid where it belong. */
				SomeChunkData some_data = generating_data.future.get();
				ChunkCoords const chunk_coords = generating_data.chunk_coords;
				ChunkGeneratingStep const step = generating_data.step;

				#if 0
				std::cout << "Data generated: "
					<< chunk_coords << " "
					<< (
						step == ChunkGeneratingStep::GENERATE_PTG_FIELD ? "PTG" :
						step == ChunkGeneratingStep::GENERATE_PTT_FIELD ? "PTT" :
						step == ChunkGeneratingStep::GENERATE_B_FIELD ? "B" :
						step == ChunkGeneratingStep::MESH ? "mesh" :
						"?")
					<< std::endl;
				#endif

				switch (step)
				{
					case ChunkGeneratingStep::GENERATE_PTG_FIELD:
						assert(std::holds_alternative<ChunkPtgField>(some_data));
						assert(not this->chunk_grid->has_ptg_field(chunk_coords));
						this->chunk_grid->ptg_field.insert(std::make_pair(
							chunk_coords, std::get<ChunkPtgField>(some_data)));
					break;
					case ChunkGeneratingStep::GENERATE_PTT_FIELD:
						assert(std::holds_alternative<ChunkPttField>(some_data));
						assert(not this->chunk_grid->has_ptt_field(chunk_coords));
						this->chunk_grid->ptt_field.insert(std::make_pair(
							chunk_coords, std::get<ChunkPttField>(some_data)));
					break;
					case ChunkGeneratingStep::GENERATE_B_FIELD:
					case ChunkGeneratingStep::DISK_READ_B_FIELD:
						assert(std::holds_alternative<ChunkBField>(some_data));
						assert(not this->chunk_grid->has_b_field(chunk_coords));
						this->chunk_grid->b_field.insert(std::make_pair(
							chunk_coords, std::get<ChunkBField>(some_data)));
					break;
					case ChunkGeneratingStep::DISK_SEARCH:
						assert(std::holds_alternative<ChunkDiskStorage>(some_data));
						assert(not this->chunk_grid->has_disk_storage(chunk_coords));
						this->chunk_grid->disk.insert(std::make_pair(
							chunk_coords, std::get<ChunkDiskStorage>(some_data)));
					break;
					case ChunkGeneratingStep::MESH:
						assert(std::holds_alternative<ChunkMeshData*>(some_data));
						assert(not this->chunk_grid->has_complete_mesh(chunk_coords));
						{
							Mesh<VertexDataClassic> mesh;
							ChunkMeshData* chunk_mesh_data = std::get<ChunkMeshData*>(some_data);
							mesh.vertex_data = std::move(*chunk_mesh_data);
							delete chunk_mesh_data;
							mesh.needs_update_opengl_data = true;
							this->chunk_grid->mesh.insert(std::make_pair(
								chunk_coords, mesh));
						}
					break;
					default:
						assert(false);
					break;
				}

				generating_data_opt.reset();
			}
		}
		
		if (not generating_data_opt.has_value())
		{
			while ((not chunk_generation_candidates.empty()) && this->generation_enabled)
			{
				/* There is a chunk that would like to see some data generated. */
				ChunkCoords const chunk_coords = chunk_generation_candidates.back();
				chunk_generation_candidates.pop_back();

				if (not this->needs_generation_step(chunk_coords, ChunkGeneratingStep::MESH))
				{
					/* Nevermind, that chunk seem to be already finished. */
					continue;
				}
				std::optional<std::pair<ChunkCoords, ChunkGeneratingStep>> const required =
					this->required_generation_step(chunk_coords, ChunkGeneratingStep::MESH);
				if (not required.has_value())
				{
					/* The examined data is not quite ready to be generated now,
					 * and there is nothing to do for it instead of waiting,
					 * so we go on to see what else we can do. */
					continue;
				}
				auto const [required_chunk_coords, required_step] = required.value();

				#if 0
				std::cout << "Data request: "
					<< required_chunk_coords << " "
					<< (
						required_step == ChunkGeneratingStep::GENERATE_PTG_FIELD ? "PTG" :
						required_step == ChunkGeneratingStep::GENERATE_PTT_FIELD ? "PTT" :
						required_step == ChunkGeneratingStep::GENERATE_B_FIELD ? "B" :
						required_step == ChunkGeneratingStep::MESH ? "mesh" :
						"?")
					<< std::endl;
				#endif

				ChunkGeneratingData generating_data;
				generating_data.chunk_coords = required_chunk_coords;
				generating_data.step = required_step;
				switch (required_step)
				{
					case ChunkGeneratingStep::MESH:
						generating_data.future = this->thread_pool->give_task(std::bind(
							[](
								ChunkCoords chunk_coords,
								ChunkNeighborhood<ChunkBField> const
									chunk_neighborhood_b_field,
								Nature const& nature
							){
								return SomeChunkData{generate_chunk_complete_mesh(
									chunk_coords, chunk_neighborhood_b_field, nature)};
							},
							required_chunk_coords,
							this->chunk_grid->get_b_field_neighborhood(required_chunk_coords),
							std::cref(nature)));
					break;
					case ChunkGeneratingStep::DISK_SEARCH:
						generating_data.future = this->thread_pool->give_task(std::bind(
							[](
								ChunkCoords chunk_coords
							){
								return SomeChunkData{search_disk_for_chunk(
									chunk_coords)};
							},
							required_chunk_coords));
					break;
					case ChunkGeneratingStep::B_FIELD:
						assert(false
							/* The `B_FIELD` variant is not a true generating step, it is a dummy
							 * step that must be converted into `GENERATE_B_FIELD` or
							 * `DISK_READ_B_FIELD` to know via which way to generate the B field.
							 * If we end up here then it means something went wrong and there is a
							 * bug to fix. */);
						std::exit(EXIT_FAILURE);
					break;
					case ChunkGeneratingStep::DISK_READ_B_FIELD:
						generating_data.future = this->thread_pool->give_task(std::bind(
							[](
								ChunkCoords chunk_coords,
								ChunkDiskStorage& chunk_disk_storage
							){
								return SomeChunkData{read_disk_chunk_b_field(
									chunk_coords, chunk_disk_storage)};
							},
							required_chunk_coords,
							std::ref(this->chunk_grid->disk[required_chunk_coords])));
					break;
					case ChunkGeneratingStep::GENERATE_B_FIELD:
						generating_data.future = this->thread_pool->give_task(std::bind(
							[](
								ChunkCoords chunk_coords,
								ChunkNeighborhood<ChunkPttField> const
									chunk_neighborhood_ptt_field,
								Nature const& nature
							){
								return SomeChunkData{generate_chunk_b_field(
									chunk_coords, chunk_neighborhood_ptt_field, nature)};
							},
							required_chunk_coords,
							this->chunk_grid->get_ptt_field_neighborhood(required_chunk_coords),
							std::cref(nature)));
					break;
					case ChunkGeneratingStep::GENERATE_PTT_FIELD:
						generating_data.future = this->thread_pool->give_task(std::bind(
							[](
								ChunkCoords chunk_coords,
								ChunkNeighborhood<ChunkPtgField> const
									chunk_neighborhood_ptg_field,
								Nature const& nature
							){
								return SomeChunkData{generate_chunk_ptt_field(
									chunk_coords, chunk_neighborhood_ptg_field, nature)};
							},
							required_chunk_coords,
							this->chunk_grid->get_ptg_field_neighborhood(required_chunk_coords),
							std::cref(nature)));
					break;
					case ChunkGeneratingStep::GENERATE_PTG_FIELD:
						generating_data.future = this->thread_pool->give_task(std::bind(
							[](
								ChunkCoords chunk_coords,
								Nature const& nature
							){
								return SomeChunkData{generate_chunk_ptg_field(
									chunk_coords, nature)};
							},
							required_chunk_coords,
							std::cref(nature)));
					break;
					default:
						assert(false);
					break;
				}
				generating_data_opt = std::move(generating_data);
				break;
			}
		}
	}
}

namespace
{

/* Is the `given_step` being done by the `step_being_done`? */
bool given_step_is_being_done(ChunkGeneratingStep step_being_done, ChunkGeneratingStep given_step)
{
	assert(step_being_done != ChunkGeneratingStep::B_FIELD
		/* The `B_FIELD` variant is not a step that can be "being done", it is a dummy
		 * step that must be converted into `GENERATE_B_FIELD` or `DISK_READ_B_FIELD`
		 * to be done. If it ends up being considered as a step that is "being done"
		 * then it means something went wrong and there is a bug to fix. */);
	if (given_step == ChunkGeneratingStep::B_FIELD)
	{
		return
			step_being_done == ChunkGeneratingStep::GENERATE_B_FIELD ||
			step_being_done == ChunkGeneratingStep::DISK_READ_B_FIELD;
	}
	else
	{
		return step_being_done == given_step;
	}
}

} /* Anonymous namespace. */

bool ChunkGenerationManager::needs_generation_step(
	ChunkCoords chunk_coords, ChunkGeneratingStep step) const
{
	/* Check if the given step for the given chunk is already on its way. */
	for (std::optional<ChunkGeneratingData> const& generating_data_opt :
		this->generating_data_vector)
	{
		if (generating_data_opt.has_value()
			&& generating_data_opt.value().chunk_coords == chunk_coords
			&& given_step_is_being_done(generating_data_opt.value().step, step))
		{
			return false;
		}
	}

	/* Check if the given step is already done and stored in the chunk grid. */
	switch (step)
	{
		case ChunkGeneratingStep::GENERATE_PTG_FIELD:
			return not this->chunk_grid->has_ptg_field(chunk_coords);
		break;
		case ChunkGeneratingStep::GENERATE_PTT_FIELD:
			return not this->chunk_grid->has_ptt_field(chunk_coords);
		break;
		case ChunkGeneratingStep::DISK_SEARCH:
			return not this->chunk_grid->has_disk_storage(chunk_coords);
		break;
		case ChunkGeneratingStep::B_FIELD:
		case ChunkGeneratingStep::GENERATE_B_FIELD:
		case ChunkGeneratingStep::DISK_READ_B_FIELD:
			return not this->chunk_grid->has_b_field(chunk_coords);
		break;
		case ChunkGeneratingStep::MESH:
			return not this->chunk_grid->has_complete_mesh(chunk_coords);
		break;
		default:
			assert(false);
		break;
	}

	/* The given step for the given chunk is neither on its way nor already done,
	 * thus we can ask for it without risking to  */
	return true;
}

std::optional<std::pair<ChunkCoords, ChunkGeneratingStep>>
	ChunkGenerationManager::required_generation_step(
		ChunkCoords chunk_coords, ChunkGeneratingStep step) const
{
	assert(this->needs_generation_step(chunk_coords, step));
	switch (step)
	{
		case ChunkGeneratingStep::MESH:
			if (this->chunk_grid->has_b_field_neighborhood(chunk_coords))
			{
				return std::make_pair(chunk_coords, step);
			}
			else
			{
				for (int dx = -1; dx <= 1; dx++)
				for (int dy = -1; dy <= 1; dy++)
				for (int dz = -1; dz <= 1; dz++)
				{
					ChunkCoords const chunk_coords_neighboor =
						chunk_coords + ChunkCoords{dx, dy, dz};
					if (this->needs_generation_step(chunk_coords_neighboor,
						ChunkGeneratingStep::B_FIELD))
					{
						return this->required_generation_step(chunk_coords_neighboor,
							ChunkGeneratingStep::B_FIELD);
					}
				}
				return std::nullopt;
			}
		break;
		case ChunkGeneratingStep::B_FIELD:
			if (this->load_save_enabled &&
				this->chunk_grid->has_disk_storage(chunk_coords) &&
				this->chunk_grid->disk[chunk_coords].exist)
			{
				if (this->needs_generation_step(chunk_coords,
					ChunkGeneratingStep::B_FIELD))
				{
					return std::make_pair(chunk_coords, ChunkGeneratingStep::DISK_READ_B_FIELD);
				}
				else
				{
					return std::nullopt;
				}
			}
			else if (this->load_save_enabled &&
				not this->chunk_grid->has_disk_storage(chunk_coords))
			{
				if (this->needs_generation_step(chunk_coords,
					ChunkGeneratingStep::DISK_SEARCH))
				{
					return std::make_pair(chunk_coords, ChunkGeneratingStep::DISK_SEARCH);
				}
				else
				{
					return std::nullopt;
				}
			}
			else
			{
				return this->required_generation_step(chunk_coords,
					ChunkGeneratingStep::GENERATE_B_FIELD);
			}
		break;
		case ChunkGeneratingStep::GENERATE_B_FIELD:
			if (this->chunk_grid->has_ptt_field_neighborhood(chunk_coords))
			{
				return std::make_pair(chunk_coords, step);
			}
			else
			{
				for (int dx = -1; dx <= 1; dx++)
				for (int dy = -1; dy <= 1; dy++)
				for (int dz = -1; dz <= 1; dz++)
				{
					ChunkCoords const chunk_coords_neighboor =
						chunk_coords + ChunkCoords{dx, dy, dz};
					if (this->needs_generation_step(chunk_coords_neighboor,
						ChunkGeneratingStep::GENERATE_PTT_FIELD))
					{
						return this->required_generation_step(chunk_coords_neighboor,
							ChunkGeneratingStep::GENERATE_PTT_FIELD);
					}
				}
				return std::nullopt;
			}
		break;
		case ChunkGeneratingStep::GENERATE_PTT_FIELD:
			if (this->chunk_grid->has_ptg_field_neighborhood(chunk_coords))
			{
				return std::make_pair(chunk_coords, step);
			}
			else
			{
				for (int dx = -1; dx <= 1; dx++)
				for (int dy = -1; dy <= 1; dy++)
				for (int dz = -1; dz <= 1; dz++)
				{
					ChunkCoords const chunk_coords_neighboor =
						chunk_coords + ChunkCoords{dx, dy, dz};
					if (this->needs_generation_step(chunk_coords_neighboor,
						ChunkGeneratingStep::GENERATE_PTG_FIELD))
					{
						return this->required_generation_step(chunk_coords_neighboor,
							ChunkGeneratingStep::GENERATE_PTG_FIELD);
					}
				}
				return std::nullopt;
			}
		break;
		case ChunkGeneratingStep::GENERATE_PTG_FIELD:
			return std::make_pair(chunk_coords, step);
		break;
		default:
			assert(false);
		break;
	}
	assert(false);
	std::exit(EXIT_FAILURE); /* GCC doesn't get that assert(false) doesn't... whatever. */
}

} /* Qwy2 */
