
#include "nature.hpp"
#include "chunk.hpp"
#include <iostream>
#include <algorithm>

namespace qwy2 {

const PixelData PixelData::unused{255, 0, 0, 0};
const PixelData PixelData::not_unused{0, 0, 0, 0};

bool PixelData::operator==(PixelData const& other) const
{
	return
		this->r == other.r &&
		this->g == other.g &&
		this->b == other.b &&
		this->a == other.a;
}

bool PixelData::operator!=(PixelData const& other) const
{
	return not (*this == other);
}

PixelRect::PixelRect(Atlas& atlas,
	unsigned int x, unsigned int y, unsigned int w, unsigned int h
):
	atlas(atlas), x(x), y(y), w(w), h(h)
{
	;
}

PixelData& PixelRect::pixel(unsigned int interior_x, unsigned int interior_y)
{
	assert(interior_x < this->w);
	assert(interior_y < this->h);

	return this->atlas.data[(this->y + interior_y) * this->atlas.side + (this->x + interior_x)];
}

AtlasRect PixelRect::atlas_rect() const
{
	AtlasRect rect;
	rect.atlas_coords_min = glm::vec2(
		static_cast<float>(this->x) / static_cast<float>(this->atlas.side),
		static_cast<float>(this->y) / static_cast<float>(this->atlas.side));
	rect.atlas_coords_max = glm::vec2(
		static_cast<float>(this->x + this->w) / static_cast<float>(this->atlas.side),
		static_cast<float>(this->y + this->h) / static_cast<float>(this->atlas.side));
	return rect;
}

Atlas::Atlas(unsigned int side):
	side(side), data(new PixelData[side * side])
{
	std::fill(this->data, this->data + this->side * this->side, PixelData::unused);

	GLint max_atlas_side;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_atlas_side);
	if (static_cast<unsigned int>(max_atlas_side) < this->side)
	{
		std::cerr << "GL_MAX_TEXTURE_SIZE is " << max_atlas_side << " which is considered as "
			<< "too small for an atlas of requested side " << this->side << std::endl;
	}
	glGenTextures(1, &openglid);
	glBindTexture(GL_TEXTURE_2D, openglid);
	this->opengltextureid = 0;
	glActiveTexture(GL_TEXTURE0 + opengltextureid);
	glTexImage2D(GL_TEXTURE_2D,
		0, GL_RGBA, this->side, this->side, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, this->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Atlas::~Atlas()
{
	delete[] this->data;
}

PixelRect Atlas::allocate_rect(unsigned int w, unsigned int h)
{
	PixelRect rect_view{*this, 0, 0, w, h};
	while (1)
	{
		/* If the current rect has a used reigion, then try another one. */
		for (unsigned int x = rect_view.x; x < rect_view.x + rect_view.w; x++)
		for (unsigned int y = rect_view.y; y < rect_view.y + rect_view.h; y++)
		{
			if (this->data[x + this->side * y] != PixelData::unused)
			{
				goto next_rect;
			}
		}

		/* Reserve the rect by marking it as used on every pixel. */
		for (unsigned int x = rect_view.x; x < rect_view.x + rect_view.w; x++)
		for (unsigned int y = rect_view.y; y < rect_view.y + rect_view.h; y++)
		{
			this->data[x + this->side * y] = PixelData::not_unused;
		}
		return rect_view;

		next_rect:
		rect_view.x += 16;
		if (rect_view.x + rect_view.w >= this->side)
		{
			rect_view.x = 0;
			rect_view.y += 16;
			if (rect_view.y + rect_view.h >= this->side)
			{
				std::cerr << "Atlas out-of-surface" << std::endl;
				std::abort();
			}
		}
	}
}

void Atlas::update_opengl_data()
{
	glBindTexture(GL_TEXTURE_2D, openglid);
	glActiveTexture(GL_TEXTURE0 + opengltextureid);
	/* TODO: Optimize ^^. */
	glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0, this->side, this->side,
		PixelData::opengl_format, PixelData::opengl_format_type,
		this->data);
}

BlockType::BlockType(
	AtlasRect fase_top_rect, AtlasRect fase_vertical_rect, AtlasRect fase_bottom_rect
):
	fase_top_rect(fase_top_rect),
	fase_vertical_rect(fase_vertical_rect),
	fase_bottom_rect(fase_bottom_rect)
{
	;
}

WorldGenerator::WorldGenerator(NoiseGenerator::SeedType seed):
	noise_generator(seed)
{
	;
}

void WorldGenerator::generate_chunk_content(Nature const& nature, Chunk& chunk)
{
	(void)nature;

	BlockCoords walker = chunk.rect.walker_start();
	do
	{
		Block& block = chunk.block(walker);
		block.type_index = 0;

		float zoom_x = static_cast<float>(walker.x) / 20.0f;
		float zoom_y = static_cast<float>(walker.y) / 20.0f;
		float zoom_z = static_cast<float>(walker.z) / 20.0f;

		float value = this->noise_generator.base_noise(zoom_x, zoom_y, zoom_z) * 19.0f;
		value += zoom_z * 3.0f;
		if (value < 0.0f)
		{
			block.is_air = false;
		}
	}
	while (chunk.rect.walker_iterate(walker));
}

NatureGenerator::NatureGenerator(NoiseGenerator::SeedType seed):
	noise_generator(seed)
{
	;
}

static void paint_grass_top(NoiseGenerator& noise_generator, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);

		pixel.r = (noise_generator.base_noise(x, y, 0) * 0.1f + 0.3f) * 255.0f;
		pixel.g = (noise_generator.base_noise(x, y, 1) * 0.2f + 0.8f) * 255.0f;
		pixel.b = (noise_generator.base_noise(x, y, 2) * 0.1f + 0.1f) * 255.0f;
		pixel.a = 255;
	}
}

BlockTypeId NatureGenerator::generate_block_type(Nature& nature)
{
	PixelRect pixel_rect_top = nature.atlas.allocate_rect(16, 16);
	PixelRect pixel_rect_vertical = nature.atlas.allocate_rect(16, 16);
	PixelRect pixel_rect_bottom = nature.atlas.allocate_rect(16, 16);

	paint_grass_top(this->noise_generator, pixel_rect_top);

	for (unsigned int y = 0; y < pixel_rect_vertical.h; y++)
	for (unsigned int x = 0; x < pixel_rect_vertical.w; x++)
	{
		PixelData& pixel = pixel_rect_vertical.pixel(x, y);

		pixel.r = (std::cos(static_cast<float>(x) * 0.1f) + 1.0f) / 2.0f * 255.0f;
		pixel.g = (std::cos(static_cast<float>(x + y) * 0.27f) + 1.0f) / 2.0f * 255.0f;
		pixel.b = (std::cos(static_cast<float>(y) * 0.31f) + 1.0f) / 2.0f * 255.0f;
		pixel.a = 255;
	}

	for (unsigned int y = 0; y < pixel_rect_bottom.h; y++)
	for (unsigned int x = 0; x < pixel_rect_bottom.w; x++)
	{
		PixelData& pixel = pixel_rect_bottom.pixel(x, y);

		pixel.r = (std::cos(static_cast<float>(x) * 0.1f) + 1.0f) / 2.0f * 255.0f;
		pixel.g = (std::cos(static_cast<float>(x + y) * 0.27f) + 1.0f) / 2.0f * 255.0f;
		pixel.b = (std::cos(static_cast<float>(y) * 0.31f) + 1.0f) / 2.0f * 255.0f;
		pixel.a = 255;
	}

	nature.atlas.update_opengl_data();

	nature.block_type_table.push_back(BlockType(
		pixel_rect_top.atlas_rect(),
		pixel_rect_vertical.atlas_rect(),
		pixel_rect_bottom.atlas_rect()));
	unsigned int block_type_index = nature.block_type_table.size() - 1;
	return block_type_index;
}

Nature::Nature(NoiseGenerator::SeedType seed):
	atlas(1024), world_generator(seed), nature_generator(seed)
{
	;
}

} /* qwy2 */
