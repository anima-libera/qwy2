
#ifndef QWY2_HEADER_NATURE_
#define QWY2_HEADER_NATURE_

#include "opengl.hpp"
#include "noise.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace qwy2 {

class PixelData
{
public:
	static constexpr GLenum opengl_format = GL_RGBA;
	static constexpr GLenum opengl_format_type =  GL_UNSIGNED_BYTE;

	/* In the texture atlas, unused pixels are of that color. */
	static const PixelData unused;

	static const PixelData not_unused;

public:
	std::uint8_t r, g, b, a;

public:
	bool operator==(PixelData const& other) const;
	bool operator!=(PixelData const& other) const;
};

/* A texture rect that can be used by shaders, with coords between 0.0f and 1.0f. */
class AtlasRect
{
public:
	glm::vec2 atlas_coords_min;
	glm::vec2 atlas_coords_max;
};

class Atlas;

/* Rect on the pixel grid. */
class PixelRect
{
public:
	Atlas& atlas;
	unsigned int x, y, w, h;

public:
	PixelRect(Atlas& atlas,
		unsigned int x, unsigned int y, unsigned int w, unsigned int h);

	PixelData& pixel(unsigned int interior_x, unsigned int interior_y);
	AtlasRect atlas_rect() const;
};

class Atlas
{
public:
	unsigned int side;
	PixelData* data;
	GLuint openglid;
	unsigned int opengltextureid;

public:
	Atlas(unsigned int side);
	~Atlas();

	PixelRect allocate_rect(unsigned int w, unsigned int h);

	void update_opengl_data();
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

using BlockTypeId = unsigned int;

class WorldGenerator
{
public:
	NoiseGenerator noise_generator;
	BlockTypeId primary_block_type;

public:
	WorldGenerator(NoiseGenerator::SeedType seed);
	void generate_chunk_content(Nature const& nature, Chunk& chunk);
};

class NatureGenerator
{
public:
	NoiseGenerator noise_generator;

public:
	NatureGenerator(NoiseGenerator::SeedType seed);
	BlockTypeId generate_block_type(Nature& nature);
};

/* Describes the laws of physics of a game of Qwy2. */
class Nature
{
public:
	Atlas atlas;
	std::vector<BlockType> block_type_table;
	WorldGenerator world_generator;
	NatureGenerator nature_generator;

public:
	Nature(NoiseGenerator::SeedType seed);
};

} /* qwy2 */

#endif /* QWY2_HEADER_NATURE_ */
