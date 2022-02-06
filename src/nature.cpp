
#include "nature.hpp"
#include "chunk.hpp"

namespace qwy2 {

BlockType::BlockType(
	AtlasRect fase_top_rect, AtlasRect fase_vertical_rect, AtlasRect fase_bottom_rect
):
	fase_top_rect(fase_top_rect),
	fase_vertical_rect(fase_vertical_rect),
	fase_bottom_rect(fase_bottom_rect)
{
	;
}

void Generator::generate_chunk_content(Nature const& nature, Chunk& chunk) const
{
	CoordsInt walker = chunk.rect.walker_start();
	do
	{
		Block& block = chunk.block(walker);
		block.type_index = 0;

		if ((walker.z == 5 && (walker.x + walker.y) % 6 == 0) || walker.z == -1)
		{
			block.is_air = false;
			block.type_index = 0;
		}
		else
		{
			block.is_air = true;
		}
	}
	while (chunk.rect.walker_iterate(walker));
}

} /* qwy2 */
