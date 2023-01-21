
#include "structure.hpp"
#include "nature.hpp"

namespace qwy2
{

StructureInstance::StructureInstance(BlockCoords head_start, BlockRect bound_rect):
	head_start(head_start), bound_rect(bound_rect)
{
	;
}

namespace
{

void set_block_if_in_chunk(ChunkBField& target_b_field, BlockCoords coords, BlockTypeId type)
{
	if (chunk_block_rect(target_b_field.chunk_coords).contains(coords))
	{
		target_b_field[coords].type_id = type;
	}
}

void move_head_at_random(BlockCoords& writing_head, Nature const& nature, int iteration_number)
{
	int moving_random = static_cast<int>(6.0f *
		nature.world_generator.noise_generator.base_noise(
			writing_head.x, writing_head.y, writing_head.z, iteration_number));
	bool move_negativeward = moving_random % 2;
	moving_random /= 2;
	int move_axis_index = moving_random;
	writing_head[move_axis_index] += move_negativeward ? -1 : 1;
}

} /* Anonymous namespace. */

void StructureInstance::generate(
	ChunkBField& target_b_field,
	ChunkCoords chunk_coords,
	ChunkNeighborhood<ChunkPttField> const& chunk_neighborhood_ptt_field,
	Nature const& nature)
{
	BlockCoords writing_head = this->head_start;
	while (true)
	{
		BlockCoords below_writing_head = writing_head - BlockCoords{0, 0, 1};
		if (chunk_neighborhood_ptt_field[below_writing_head] == 1)
		{
			break;
		}
		writing_head = below_writing_head;
		if (not this->bound_rect.contains(writing_head))
		{
			return;
		}
	}
	for (int i = 0; i < 40; i++)
	{
		set_block_if_in_chunk(target_b_field, writing_head, 4);
		move_head_at_random(writing_head, nature, i);
	}
}

} /* qwy2 */
