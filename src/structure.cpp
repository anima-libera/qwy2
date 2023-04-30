
#include "structure.hpp"
#include "nature.hpp"

namespace qwy2
{

void StructureGenerationProgram::perform(StructureGenerationContext& context) const
{
	for (StructureGenerationStep* step : this->steps)
	{
		if (context.is_done)
		{
			return;
		}
		step->perform(context);
		context.step_number++;
	}
}

namespace structure_generation_step
{

void SearchGround::perform(StructureGenerationContext& context) const
{
	while (true)
	{
		if (context.bound_rect.contains(context.head) &&
			context.bound_rect.contains(context.head - BlockCoords{0, 0, 1}) &&
			context.chunk_neighborhood_ptt_field[context.head] == 0 &&
			context.chunk_neighborhood_ptt_field[context.head - BlockCoords{0, 0, 1}] != 0)
		{
			/* We are just one block above ground. */
			break;
		}
		else if ((not context.bound_rect.contains(context.head)) ||
			(not context.bound_rect.contains(context.head - BlockCoords{0, 0, 1})))
		{
			/* We can't go down anymore. */
			context.is_done = true;
			return;
		}
		else
		{
			/* We can go down and we have not found the ground yet. */
			context.head = context.head - BlockCoords{0, 0, 1};
		}
	}
}

void MoveAtRandom::perform(StructureGenerationContext& context) const
{
	int moving_random = static_cast<int>(6.0f *
		context.nature.world_generator.noise_generator.base_noise(
			context.head.x, context.head.y, context.head.z, context.step_number));
	BlockCoords new_head = context.head;
	new_head[moving_random / 2] += moving_random % 2 == 0 ? -1 : 1;
	if (not context.bound_rect.contains(new_head))
	{
		/* Uh what to do here? We can't move in the randomly chosen direction. */
		return;
	}
	context.head = new_head;
}

void MoveUpwards::perform(StructureGenerationContext& context) const
{
	BlockCoords new_head = context.head + BlockCoords{0, 0, 1};
	if (not context.bound_rect.contains(new_head))
	{
		/* Uh what to do here? We can't move in the randomly chosen direction. */
		return;
	}
	context.head = new_head;
}

PlaceBlock::PlaceBlock(BlockTypeId block_type_id, bool only_in_air):
	block_type_id(block_type_id), only_in_air(only_in_air)
{
	;
}

void PlaceBlock::perform(StructureGenerationContext& context) const
{
	if (context.bound_rect.contains(context.head) &&
		chunk_block_rect(context.target_b_field.chunk_coords).contains(context.head))
	{
		if ((this->only_in_air && context.target_b_field[context.head].is_air()) ||
			(not this->only_in_air))
		{
			context.target_b_field[context.head].type_id = this->block_type_id;
		}
	}
}

Repeat::Repeat(int inf, int sup, StructureGenerationProgram body):
	inf(inf), sup(sup), body(body)
{
	;
}

void Repeat::perform(StructureGenerationContext& context) const
{
	int number_of_iterations = this->inf + static_cast<int>(static_cast<float>(this->sup - this->inf) *
		context.nature.world_generator.noise_generator.base_noise(
			context.head.x, context.head.y, context.head.z, context.step_number));
	for (int i = 0; i < number_of_iterations; i++)
	{
		if (context.is_done)
		{
			return;
		}
		this->body.perform(context);
	}
}

RepeatFromSamePosition::RepeatFromSamePosition(int inf, int sup, StructureGenerationProgram body):
	inf(inf), sup(sup), body(body)
{
	;
}

void RepeatFromSamePosition::perform(StructureGenerationContext& context) const
{
	int number_of_iterations = this->inf + static_cast<int>(static_cast<float>(this->sup - this->inf) *
		context.nature.world_generator.noise_generator.base_noise(
			context.head.x, context.head.y, context.head.z, context.step_number));
	BlockCoords position = context.head;
	for (int i = 0; i < number_of_iterations; i++)
	{
		if (context.is_done)
		{
			return;
		}
		context.head = position;
		this->body.perform(context);
	}
}

} /* structure_generation_step */

StructureInstance::StructureInstance(StructureTypeId type_id, BlockCoords head_start, BlockRect bound_rect):
	type_id(type_id), head_start(head_start), bound_rect(bound_rect)
{
	;
}

void StructureInstance::generate(
	ChunkBField& target_b_field,
	ChunkCoords chunk_coords,
	ChunkNeighborhood<ChunkPttField> const& chunk_neighborhood_ptt_field,
	Nature const& nature)
{
	StructureGenerationContext context{
		false,
		0,
		this->head_start,
		this->bound_rect,
		nature,
		chunk_neighborhood_ptt_field,
		target_b_field};
	nature.structure_type_table[this->type_id].generation_program.perform(context);
}

} /* qwy2 */
