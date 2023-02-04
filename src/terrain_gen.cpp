
#include "terrain_gen.hpp"
#include "nature.hpp"

namespace qwy2
{

ChunkPtgField PlainTerrainGeneratorFlat::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		ptg_field[coords] = (coords.z <= 0) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorClassic::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		float const dist =
			glm::distance(glm::vec2(coords.x, coords.y), glm::vec2(0.0f, 0.0f));
		float const crazy =
			dist < 20.0f ? 2.0f :
			(dist - 20.0f + 2.0f) * 3.0f;
		ptg_field[coords] =
			((value - (1.0f - nature.world_generator.density)) * crazy > coords.z) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorHills::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size);
		ptg_field[coords] = (coords.z <= -value * 5.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorHomogenous::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		ptg_field[coords] = (value - nature.world_generator.density < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorPlane::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		float const value_plane =
			value + static_cast<float>(std::abs(coords.z - (-20))) / 30.0f;
		ptg_field[coords] = (value_plane - nature.world_generator.density < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorPlanes::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		int nearest_plane_z = coords.z - cool_mod(coords.z, 60) + 30;
		float const value_plane =
			value + static_cast<float>(std::abs(coords.z - nearest_plane_z)) / 30.0f;
		ptg_field[coords] = (value_plane - nature.world_generator.density < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorVerticalPillar::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		float const dist =
			glm::distance(glm::vec2(coords.x, coords.y), glm::vec2(0.0f, 0.0f));
		ptg_field[coords] = (-(value - dist * 0.01f) < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorVerticalHole::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		float const dist =
			glm::distance(glm::vec2(coords.x, coords.y), glm::vec2(0.0f, 0.0f));
		ptg_field[coords] = (value - dist * 0.01f < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorHorizontalPillar::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		float const dist =
			glm::distance(glm::vec2(coords.y, coords.z), glm::vec2(0.0f, 0.0f));
		ptg_field[coords] = (-(value - dist * 0.01f) < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorHorizontalHole::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(coords.x) / nature.world_generator.noise_size,
			static_cast<float>(coords.y) / nature.world_generator.noise_size,
			static_cast<float>(coords.z) / nature.world_generator.noise_size);
		float const dist =
			glm::distance(glm::vec2(coords.y, coords.z), glm::vec2(0.0f, 0.0f));
		ptg_field[coords] = (value - dist * 0.01f < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorFunky1::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = nature.world_generator.noise_generator.base_noise(
				static_cast<float>(coords.x) / nature.world_generator.noise_size,
				static_cast<float>(coords.y) / nature.world_generator.noise_size,
				static_cast<float>(coords.z) / nature.world_generator.noise_size,
				42 + i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 10.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 20.0f * nature.world_generator.terrain_param_b;
		glm::ivec3 ci{
			static_cast<int>(c.x) - (static_cast<int>(c.x) % h),
			static_cast<int>(c.y) - (static_cast<int>(c.y) % h),
			static_cast<int>(c.z) - (static_cast<int>(c.z) % h)};
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		ptg_field[coords] = (value - nature.world_generator.density < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorFunky2::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = nature.world_generator.noise_generator.base_noise(
				static_cast<float>(coords.x) / nature.world_generator.noise_size,
				static_cast<float>(coords.y) / nature.world_generator.noise_size,
				static_cast<float>(coords.z) / nature.world_generator.noise_size,
				42 + i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 10.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 20.0f * nature.world_generator.terrain_param_b;
		glm::ivec3 ci{
			static_cast<int>(c.x) - (static_cast<int>(c.x) % h),
			static_cast<int>(c.y) - (static_cast<int>(c.y) % h),
			static_cast<int>(c.z) - (static_cast<int>(c.z) % h)};
		float const fard = 8.0f * nature.world_generator.terrain_param_c;
		float const value = nature.world_generator.noise_generator.base_noise(
			static_cast<float>(ci.x) / (static_cast<float>(h) * fard),
			static_cast<float>(ci.y) / (static_cast<float>(h) * fard),
			static_cast<float>(ci.z) / (static_cast<float>(h) * fard),
			69);
		ptg_field[coords] = (value - nature.world_generator.density < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

PlainTerrainGenerator* plain_terrain_generator_from_name(std::string_view name)
{
	using namespace std::literals::string_view_literals;
	if (name == "flat"sv)              return new PlainTerrainGeneratorFlat{};
	if (name == "classic"sv)           return new PlainTerrainGeneratorClassic{};
	if (name == "hills"sv)             return new PlainTerrainGeneratorHills{};
	if (name == "homogenous"sv)        return new PlainTerrainGeneratorHomogenous{};
	if (name == "plane"sv)             return new PlainTerrainGeneratorPlane{};
	if (name == "planes"sv)            return new PlainTerrainGeneratorPlanes{};
	if (name == "vertical_pillar"sv)   return new PlainTerrainGeneratorVerticalPillar{};
	if (name == "vertical_hole"sv)     return new PlainTerrainGeneratorVerticalHole{};
	if (name == "horizontal_pillar"sv) return new PlainTerrainGeneratorHorizontalPillar{};
	if (name == "horizontal_hole"sv)   return new PlainTerrainGeneratorHorizontalHole{};
	if (name == "funky_1"sv)           return new PlainTerrainGeneratorFunky1{};
	if (name == "funky_2"sv)           return new PlainTerrainGeneratorFunky2{};
	assert(false /* Unknown plain terrain generator name. */);
	std::exit(EXIT_FAILURE);
}

} /* qwy2 */
