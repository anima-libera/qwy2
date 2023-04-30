
#ifndef QWY2_HEADER_STRUCTURE_
#define QWY2_HEADER_STRUCTURE_

#include "coords.hpp"
#include "chunk.hpp"
#include <vector>

namespace qwy2
{

class StructureGenerationContext
{
public:
	bool is_done;
	int step_number;
	BlockCoords head;
	BlockRect bound_rect;
	Nature const& nature;
	ChunkNeighborhood<ChunkPttField> const& chunk_neighborhood_ptt_field;
	ChunkBField& target_b_field;
};

class StructureGenerationStep
{
public:
	virtual void perform(StructureGenerationContext& context) const = 0;
};

class StructureGenerationProgram
{
public:
	std::vector<StructureGenerationStep*> steps;
	void perform(StructureGenerationContext& context) const;
};

namespace structure_generation_step
{

class SearchGround: public StructureGenerationStep
{
public:
	virtual void perform(StructureGenerationContext& context) const override final;
};

class MoveAtRandom: public StructureGenerationStep
{
public:
	virtual void perform(StructureGenerationContext& context) const override final;
};

class MoveUpwards: public StructureGenerationStep
{
public:
	virtual void perform(StructureGenerationContext& context) const override final;
};

class PlaceBlock: public StructureGenerationStep
{
public:
	BlockTypeId block_type_id;
	PlaceBlock(BlockTypeId block_type_id);
	virtual void perform(StructureGenerationContext& context) const override final;
};

class Repeat: public StructureGenerationStep
{
public:
	int inf, sup;
	StructureGenerationProgram body;
	Repeat(int inf, int sup, StructureGenerationProgram body);
	virtual void perform(StructureGenerationContext& context) const override final;
};

class RepeatFromSamePosition: public StructureGenerationStep
{
public:
	int inf, sup;
	StructureGenerationProgram body;
	RepeatFromSamePosition(int inf, int sup, StructureGenerationProgram body);
	virtual void perform(StructureGenerationContext& context) const override final;
};

} /* structure_generation_step */

class StructureType
{
public:
	StructureGenerationProgram generation_program;
};

using StructureTypeId = unsigned int;

class StructureInstance
{
public:
	StructureTypeId type_id;
	BlockCoords head_start;
	BlockRect bound_rect;

public:
	StructureInstance(StructureTypeId type_id, BlockCoords head_start, BlockRect bound_rect);

	void generate(
		ChunkBField& target_b_field,
		ChunkCoords chunk_coords,
		ChunkNeighborhood<ChunkPttField> const& chunk_neighborhood_ptt_field,
		Nature const& nature);
};

} /* qwy2 */

#endif /* QWY2_HEADER_STRUCTURE_ */
