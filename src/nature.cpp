
#include "nature.hpp"
#include "chunk.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <limits>

namespace qwy2
{

PixelData const PixelData::UNUSED{255, 0, 0, 0};
PixelData const PixelData::NOT_UNUSED{0, 0, 0, 0};

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
	atlas{atlas}, x{x}, y{y}, w{w}, h{h}
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
	rect.atlas_coords_min = glm::vec2{
		static_cast<float>(this->x) / static_cast<float>(this->atlas.side),
		static_cast<float>(this->y) / static_cast<float>(this->atlas.side)};
	rect.atlas_coords_max = glm::vec2{
		static_cast<float>(this->x + this->w) / static_cast<float>(this->atlas.side),
		static_cast<float>(this->y + this->h) / static_cast<float>(this->atlas.side)};
	return rect;
}

Atlas::Atlas(unsigned int side):
	side{side}, data{new PixelData[side * side]}
{
	std::fill(this->data, this->data + this->side * this->side, PixelData::UNUSED);

	GLint max_atlas_side;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_atlas_side);
	if (static_cast<unsigned int>(max_atlas_side) < this->side)
	{
		std::cerr << "GL_MAX_TEXTURE_SIZE is " << max_atlas_side << " which is considered as "
			<< "too small for an atlas of requested side " << this->side << std::endl;
	}
	glGenTextures(1, &this->texture_openglid);
	glBindTexture(GL_TEXTURE_2D, this->texture_openglid);
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
			if (this->data[x + this->side * y] != PixelData::UNUSED)
			{
				goto next_rect;
			}
		}

		/* Reserve the rect by marking it as used on every pixel. */
		for (unsigned int x = rect_view.x; x < rect_view.x + rect_view.w; x++)
		for (unsigned int y = rect_view.y; y < rect_view.y + rect_view.h; y++)
		{
			this->data[x + this->side * y] = PixelData::NOT_UNUSED;
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
	glBindTexture(GL_TEXTURE_2D, this->texture_openglid);
	/* TODO: Optimize ^^. */
	glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0, this->side, this->side,
		PixelData::OPENGL_FORMAT, PixelData::OPENGL_FORMAT_TYPE,
		this->data);
}

BlockType::BlockType(
	AtlasRect fase_top_rect, AtlasRect fase_vertical_rect, AtlasRect fase_bottom_rect
):
	fase_top_rect{fase_top_rect},
	fase_vertical_rect{fase_vertical_rect},
	fase_bottom_rect{fase_bottom_rect}
{
	;
}


WorldGenerator::WorldGenerator(NoiseGenerator::SeedType seed):
	noise_generator{seed}, noise_generator_2{seed+1}
{
	;
}

namespace
{

[[maybe_unused]]
float generator_value(NoiseGenerator const& noise_generator, BlockCoords coords)
{
	if (std::abs(coords.x) == 10 && std::abs(coords.y) == 10 && coords.z == 2)
	{
		return -1.0f;
	}

	float zoom_x = static_cast<float>(coords.x) / 20.0f;
	float zoom_y = static_cast<float>(coords.y) / 20.0f;
	float zoom_z = static_cast<float>(coords.z) / 20.0f;

	float value = noise_generator.base_noise(zoom_x, zoom_y, zoom_z) * 19.0f;

	float const hole_x = -40.0f, hole_y = 55.0f;
	float hole_dist = std::sqrt(
		(coords.x - hole_x) * (coords.x - hole_x) +
		(coords.y - hole_y) * (coords.y - hole_y));

	if (15.0f < hole_dist && (hole_dist < 150.0f || coords.x < 0) && coords.z <= 0.0f)
	{
		return std::numeric_limits<float>::lowest();
	}
	else if (hole_dist <= 15.0f && coords.z <= 0.0f)
	{
		return (value / 19.0f) * (15.0f - hole_dist + 1.0f) - 1.5f;
	}

	float const pillar_x = 40.0f, pillar_y = 0.0f;
	float pillar_dist = std::sqrt(
		(coords.x - pillar_x) * (coords.x - pillar_x) +
		(coords.y - pillar_y) * (coords.y - pillar_y));
	pillar_dist = std::min(pillar_dist, 40.0f);

	value += std::min(zoom_z, 20.0f / 20.0f) * pillar_dist * 0.1f;
	value *= pillar_dist;

	value -= 200.0f;

	return value;
}

} /* Anonymous namespace. */


NatureGenerator::NatureGenerator(NoiseGenerator::SeedType seed):
	noise_generator{seed}
{
	;
}

namespace
{

void paint_grass_pixel(NoiseGenerator& noise_generator, int x, int y, PixelData& pixel)
{
	pixel.r = (noise_generator.base_noise(x, y, 0) * 0.1f + 0.3f) * 255.0f;
	pixel.g = (noise_generator.base_noise(x, y, 1) * 0.2f + 0.8f) * 255.0f;
	pixel.b = (noise_generator.base_noise(x, y, 2) * 0.1f + 0.1f) * 255.0f;
	pixel.a = 255;
}

void paint_dirt_pixel(NoiseGenerator& noise_generator, int x, int y, PixelData& pixel)
{
	pixel.r = (noise_generator.base_noise(x, y, 0) * 0.15f + 0.4f) * 255.0f;
	pixel.g = (noise_generator.base_noise(x, y, 1) * 0.15f + 0.3f) * 255.0f;
	pixel.b = (noise_generator.base_noise(x, y, 2) * 0.05f + 0.0f) * 255.0f;
	pixel.a = 255;
}

void paint_rock_pixel(NoiseGenerator& noise_generator, int x, int y, PixelData& pixel)
{
	float const grey_value = noise_generator.base_noise(x, y, 0);
	pixel.r = (grey_value * 0.15f + 0.7f) * 255.0f;
	pixel.g = (grey_value * 0.15f + 0.7f) * 255.0f;
	pixel.b = (grey_value * 0.15f + 0.7f) * 255.0f;
	pixel.a = 255;
}

void paint_white_pixel(NoiseGenerator& noise_generator, int x, int y, PixelData& pixel)
{
	float const grey_value = noise_generator.base_noise(x, y, 0);
	pixel.r = (grey_value * 0.05f + 0.95f) * 255.0f;
	pixel.g = (grey_value * 0.05f + 0.95f) * 255.0f;
	pixel.b = (grey_value * 0.05f + 0.95f) * 255.0f;
	pixel.a = 255;
}

void paint_grass_top(NoiseGenerator& noise_generator, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);
		paint_grass_pixel(noise_generator, x, y, pixel);
	}
}

void paint_grass_vertical(NoiseGenerator& noise_generator, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);

		int grass_length = static_cast<int>(noise_generator.base_noise(x) * 2.0f + 6.5f);
		if (y <= grass_length)
		{
			paint_grass_pixel(noise_generator, x, y, pixel);
		}
		else
		{
			paint_dirt_pixel(noise_generator, x, y, pixel);
		}
	}
}

void paint_dirt(NoiseGenerator& noise_generator, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);
		paint_dirt_pixel(noise_generator, x, y, pixel);
	}
}

void paint_rock(NoiseGenerator& noise_generator, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);
		paint_rock_pixel(noise_generator, x, y, pixel);
	}
}

void paint_white(NoiseGenerator& noise_generator, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);
		paint_white_pixel(noise_generator, x, y, pixel);
	}
}

float cap(float x, float inf, float sup)
{
	if (x < inf)
	{
		return inf;
	}
	else if (sup < x)
	{
		return sup;
	}
	else
	{
		return x;
	}
}

void paint_colored(NoiseGenerator& noise_generator, PixelData color, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);
		pixel = color;

		pixel.r = cap(pixel.r + (noise_generator.base_noise(x, y, 0) - 0.5f) * 30.0f, 0.0f, 255.0f);
		pixel.g = cap(pixel.g + (noise_generator.base_noise(x, y, 1) - 0.5f) * 30.0f, 0.0f, 255.0f);
		pixel.b = cap(pixel.b + (noise_generator.base_noise(x, y, 2) - 0.5f) * 30.0f, 0.0f, 255.0f);
		pixel.a = 255;
	}
}

float interpolate_ratio_smoothstep(float x)
{
	return 3.0f * x * x - 2.0f * x * x * x;
}
float interpolate(float x, float inf, float sup)
{
	float const ratio = interpolate_ratio_smoothstep(x);
	return inf * (1.0f - ratio) + sup * ratio;
}

/* TODO: Make a more proper function out of this. */
float octaved_noise(NoiseGenerator& noise_generator, float noise_size, float x, float y, float z, float w)
{
	constexpr unsigned int octave_number = 4;
	float value_sum = 0.0f, coef_sum = 0.0f;
	for (int i = 0; i < octave_number; i++)
	{
		float value = noise_generator.base_noise(
			#define H(param_) (param_) / (noise_size / static_cast<float>(1 << i))
				H(x), H(y), H(z), H(w)
			#undef H
			);
		float coef = 1.0f / static_cast<float>(1 << i);
		value_sum += value * coef;
		coef_sum += coef;
	}
	float const value = value_sum / coef_sum;
	return value;
};

void paint_two_colored(NoiseGenerator& noise_generator, PixelData color_a, PixelData color_b, int block_type_index, PixelRect& pixel_rect)
{
	for (int y = 0; y < static_cast<int>(pixel_rect.h); y++)
	for (int x = 0; x < static_cast<int>(pixel_rect.w); x++)
	{
		PixelData& pixel = pixel_rect.pixel(x, y);
		pixel = color_a;

		float fx = x;
		float fy = y;
		float ax = std::cos(fx * TAU / static_cast<float>(pixel_rect.w));
		float ay = std::sin(fx * TAU / static_cast<float>(pixel_rect.w));
		float bx = std::cos(fy * TAU / static_cast<float>(pixel_rect.w));
		float by = std::sin(fy * TAU / static_cast<float>(pixel_rect.w));
		bx += static_cast<float>(block_type_index) * 40.0f;
		float more_a_than_b = octaved_noise(noise_generator, 1.0f, ax, ay, bx, by);
		//float more_a_than_b = octaved_noise(noise_generator, 0.5f, ax, ay, bx, by);
		//float more_a_than_b = noise_generator.base_noise(ax, ay, bx, by);
		//float more_a_than_b = noise_generator.base_noise(ax*0.7f, ay*0.7f, bx*0.7f, by*0.7f);
		pixel.r = interpolate(more_a_than_b,
			static_cast<float>(color_a.r), static_cast<float>(color_b.r));
		pixel.g = interpolate(more_a_than_b,
			static_cast<float>(color_a.g), static_cast<float>(color_b.g));
		pixel.b = interpolate(more_a_than_b,
			static_cast<float>(color_a.b), static_cast<float>(color_b.b));
		pixel.a = 255;
	}
}

} /* Anonymous namespace. */

BlockTypeId NatureGenerator::generate_block_type(Nature& nature)
{
	unsigned int block_type_index = nature.block_type_table.size();

	int side = 16;

	PixelRect pixel_rect_top = nature.atlas.allocate_rect(side, side);
	PixelRect pixel_rect_vertical = nature.atlas.allocate_rect(side, side);
	PixelRect pixel_rect_bottom = nature.atlas.allocate_rect(side, side);

	if (block_type_index == 1)
	{
		paint_grass_top(this->noise_generator, pixel_rect_top);
		paint_grass_vertical(this->noise_generator, pixel_rect_vertical);
		paint_dirt(this->noise_generator, pixel_rect_bottom);
	}
	else if (block_type_index == 2)
	{
		paint_dirt(this->noise_generator, pixel_rect_top);
		paint_dirt(this->noise_generator, pixel_rect_vertical);
		paint_dirt(this->noise_generator, pixel_rect_bottom);
	}
	else if (block_type_index == 3)
	{
		paint_rock(this->noise_generator, pixel_rect_top);
		paint_rock(this->noise_generator, pixel_rect_vertical);
		paint_rock(this->noise_generator, pixel_rect_bottom);
	}
	else if (block_type_index == 4)
	{
		paint_white(this->noise_generator, pixel_rect_top);
		paint_white(this->noise_generator, pixel_rect_vertical);
		paint_white(this->noise_generator, pixel_rect_bottom);
	}
	else
	{
		if (this->noise_generator.base_noise(static_cast<int>(block_type_index + 12)) < 0.6f)
		{
			PixelData color{
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 44)),
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 45)),
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 46)),
				255};
			paint_colored(this->noise_generator, color, pixel_rect_top);
			paint_colored(this->noise_generator, color, pixel_rect_vertical);
			paint_colored(this->noise_generator, color, pixel_rect_bottom);
		}
		else
		{
			PixelData color_a{
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 41)),
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 42)),
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 43)),
				255};
			PixelData color_b{
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 44)),
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 45)),
				255.0f * this->noise_generator.base_noise(static_cast<int>(block_type_index * 17 + 46)),
				255};
			paint_two_colored(this->noise_generator, color_a, color_b, block_type_index, pixel_rect_top);
			paint_two_colored(this->noise_generator, color_a, color_b, block_type_index, pixel_rect_vertical);
			paint_two_colored(this->noise_generator, color_a, color_b, block_type_index, pixel_rect_bottom);
		}
	}

	nature.atlas.update_opengl_data();

	nature.block_type_table.push_back(BlockType{
		pixel_rect_top.atlas_rect(),
		pixel_rect_vertical.atlas_rect(),
		pixel_rect_bottom.atlas_rect()});

	return block_type_index;
}

Nature::Nature(NoiseGenerator::SeedType seed):
	atlas{1024}, world_generator{seed}, nature_generator{seed}
{
	;
}

} /* qwy2 */
