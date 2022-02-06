
#ifndef QWY2_HEADER_NATURE_
#define QWY2_HEADER_NATURE_

#include <glm/glm.hpp>
#include <vector>

namespace qwy2 {

class AtlasRect
{
public:
	glm::vec2 atlas_coords_min;
	glm::vec2 atlas_coords_max;
};

class BlockType
{
public:
	AtlasRect fase_top_rect;
	AtlasRect fase_vertical_rect;
	AtlasRect fase_bottom_rect;

public:
	BlockType(
		AtlasRect fase_top_rect, AtlasRect fase_vertical_rect, AtlasRect fase_bottom_rect);
};

class Chunk;
class Nature;

class Generator
{
public:
	void generate_chunk_content(Nature const& nature, Chunk& chunk) const;
};

/* Describes the laws of physics of a game of Qwy2. */
class Nature
{
public:
	std::vector<BlockType> block_type_table;
	Generator generator;
};

} /* qwy2 */

#endif /* QWY2_HEADER_NATURE_ */
