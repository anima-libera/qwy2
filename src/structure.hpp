
#ifndef QWY2_HEADER_STRUCTURE_
#define QWY2_HEADER_STRUCTURE_

#include "coords.hpp"
#include "chunk.hpp"

namespace qwy2
{

class StructureType
{
public:
};

class StructureInstance
{
public:
	BlockCoords head_start;
	BlockRect bound_rect;

public:
	StructureInstance(BlockCoords head_start, BlockRect bound_rect);

	void generate(
		ChunkBField& target_b_field,
		ChunkCoords chunk_coords,
		ChunkNeighborhood<ChunkPttField> const& chunk_neighborhood_ptt_field,
		Nature const& nature);
};

} /* qwy2 */

#endif /* QWY2_HEADER_STRUCTURE_ */
