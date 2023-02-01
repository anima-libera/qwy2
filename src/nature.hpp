
#ifndef QWY2_HEADER_NATURE_
#define QWY2_HEADER_NATURE_

#include "opengl.hpp"
#include "noise.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

namespace qwy2
{

/* One RGBA pixel of a pixel grid. */
class PixelData
{
public:
	static constexpr GLenum OPENGL_FORMAT = GL_RGBA;
	static constexpr GLenum OPENGL_FORMAT_TYPE = GL_UNSIGNED_BYTE;

	/* In the texture atlas, unused pixels are of that color.
	 * Pixels of that color are considered free to be allocated to some new texture. */
	static PixelData const UNUSED;

	/* Different of the unused color, used to mark newly allocated pixels as used. */
	static PixelData const NOT_UNUSED;

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

/* Texture atlas, a big texture conaining a lot of small textures in it to allow using
 * numerous textures without having to switch the bound OpenGL textures. */
class Atlas
{
public:
	unsigned int side;
	PixelData* data;
	unsigned int texture_openglid;

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
class IsolatedChunk;
class Nature;

using BlockTypeId = std::uint16_t;

class WorldGenerator
{
public:
	NoiseGenerator noise_generator;
	NoiseGenerator noise_generator_2;
	BlockTypeId surface_block_type;
	BlockTypeId primary_block_type;
	BlockTypeId secondary_block_type;
	BlockTypeId test_block_type;
	bool flat;
	bool hills;
	bool homogenous;
	bool plane;
	bool planes;
	bool vertical_pillar;
	bool vertical_hole;
	bool horizontal_pillar;
	bool horizontal_hole;
	bool terrain_test_1;
	bool terrain_test_2;
	float noise_size;
	float density;
	float terrain_param_a;
	float terrain_param_b;
	float terrain_param_c;
	bool structures_enabled;
	bool stone_terrain;

public:
	WorldGenerator(NoiseGenerator::SeedType seed);
	#if 0
	void generate_chunk_content(Nature const& nature, IsolatedChunk& chunk) const;
	#endif
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
