
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

ChunkPtgField PlainTerrainGeneratorOctaves1::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(nature.world_generator.noise_size * 2.5f / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		ptg_field[coords] = (value + static_cast<float>(coords.z) * 0.01f < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorOctaves2::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const noise_size = nature.world_generator.noise_size * 2.5f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		ptg_field[coords] =
			(value + static_cast<float>(coords.z) * (0.2f / noise_size) < 0.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorNoiseTest1::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value = octaved_noise(coords, 1);
		ptg_field[coords] = (0.48f <= value && value <= 0.52f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorNoiseTest2::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value_a = octaved_noise(coords, 1);
		float const value_b = octaved_noise(coords, 2);
		float const inf = 0.5f - 0.02f * nature.world_generator.terrain_param_a;
		float const sup = 0.5f + 0.02f * nature.world_generator.terrain_param_a;
		ptg_field[coords] =
			(inf <= value_a && value_a <= sup) &&
			(inf <= value_b && value_b <= sup) ?
				1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorNoiseTest3::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel, float noise_size_factor)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f * noise_size_factor;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		float const value_a = octaved_noise(coords, 1, 1.0f);
		float const value_b = octaved_noise(coords, 2, 1.0f);
		float const value_c = octaved_noise(coords, 3, 10.0f) < 0.5f ? 0.0f : 0.02f;
		float const inf = 0.5f - 0.5f * value_c;
		float const sup = 0.5f + 0.5f * value_c;
		ptg_field[coords] =
			(inf <= value_a && value_a <= sup) &&
			(inf <= value_b && value_b <= sup) ?
				1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorNoiseTest4::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel, float noise_size_factor)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f * noise_size_factor;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		BlockCoords coords_but_flatter = coords;
		coords_but_flatter.z *= 10.0f;
		float const value_a = octaved_noise(coords_but_flatter, 1, 3.0f);
		float const value_b = octaved_noise(coords, 2, 1.0f);
		float const inf_a = 0.5f - 0.06f * nature.world_generator.terrain_param_a;
		float const sup_a = 0.5f + 0.06f * nature.world_generator.terrain_param_a;
		float const inf_b = 0.5f - 0.02f * nature.world_generator.terrain_param_a;
		float const sup_b = 0.5f + 0.02f * nature.world_generator.terrain_param_a;
		ptg_field[coords] =
			(inf_a <= value_a && value_a <= sup_a) &&
			(inf_b <= value_b && value_b <= sup_b) ?
				1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorCaves1::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		if (coords.z > 0)
		{
			ptg_field[coords] = 0;
		}
		else
		{
			float const value_a = octaved_noise(coords, 1);
			float const value_b = octaved_noise(coords, 2);
			float const inf = 0.5f - 0.04f * nature.world_generator.terrain_param_a;
			float const sup = 0.5f + 0.04f * nature.world_generator.terrain_param_a;
			ptg_field[coords] =
				(inf <= value_a && value_a <= sup) &&
				(inf <= value_b && value_b <= sup) ?
					0 : 1;
		}
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes1::generate_chunk_ptg_field(
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
		int const h = 70.0f * nature.world_generator.terrain_param_b;
		glm::ivec3 ci{
			static_cast<int>(c.x) - (static_cast<int>(c.x) % h),
			static_cast<int>(c.y) - (static_cast<int>(c.y) % h),
			static_cast<int>(c.z) - (static_cast<int>(c.z) % 200)};
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		ptg_field[coords] = (coords.z <= -value * 100.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes2::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = octaved_noise(coords, i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 100.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 70.0f * nature.world_generator.terrain_param_b;
		auto iize = [](int a, int b)
		{
			return (a - cool_mod(a, b)) / b;
		};
		glm::ivec3 ci{
			iize(c.x, h),
			iize(c.y, h),
			iize(c.z, 200)};
		/* Trying to get some ci blocks to map to neighbors to make same "biome" block zones. */
		glm::ivec3 initial_ci{ci};
		int asking_neighbor_count = 0;
		while (asking_neighbor_count <= 30)
		{
			asking_neighbor_count++;
			/* Try to limit neighborhood to horizontal plane neighbors for simplicity here. */
			glm::ivec3 dci[9] = {
				{-1, -1, 0}, {-1, 0, 0}, {-1, 1, 0},
				{0, -1, 0}, {0, 0, 0}, {0, 1, 0},
				{1, -1, 0}, {1, 0, 0}, {1, 1, 0}};
			float max_value = -1.0f;
			glm::ivec3 max_dci{0, 0, 0};
			for (int i = 0; i < 9; i++)
			{
				float value = nature.world_generator.noise_generator.base_noise(
					ci.x + dci[i].x, ci.y + dci[i].y, ci.z + dci[i].z, 13);
				if (max_value < value)
				{
					max_value = value;
					max_dci = dci[i];
				}
			}
			if (max_dci == glm::ivec3{0, 0, 0})
			{
				break;
			}
			ci += max_dci;
		}
		/* Zones of blocks are mapped to a "ci" block, and "ci" blocks are mapped to other
		 * "ci" blocks so that the shape of the final mapping does not look too much like a grid.
		 * Each "ci" block at the end of the mapping chain represents a biome type,
		 * which is represented as altitude for now. */
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		ptg_field[coords] = (coords.z <= -value * 100.0f) ? 1 : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes3::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = octaved_noise(coords, i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 100.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 70.0f * nature.world_generator.terrain_param_b;
		auto iize = [](int a, int b)
		{
			return (a - cool_mod(a, b)) / b;
		};
		glm::ivec3 ci{
			iize(c.x, h),
			iize(c.y, h),
			iize(c.z, 200)};
		/* Trying to get some ci blocks to map to neighbors to make same "biome" block zones. */
		glm::ivec3 initial_ci{ci};
		int asking_neighbor_count = 0;
		while (asking_neighbor_count <= 30)
		{
			asking_neighbor_count++;
			/* Try to limit neighborhood to horizontal plane neighbors for simplicity here. */
			glm::ivec3 dci[9] = {
				{-1, -1, 0}, {-1, 0, 0}, {-1, 1, 0},
				{0, -1, 0}, {0, 0, 0}, {0, 1, 0},
				{1, -1, 0}, {1, 0, 0}, {1, 1, 0}};
			float max_value = -1.0f;
			glm::ivec3 max_dci{0, 0, 0};
			for (int i = 0; i < 9; i++)
			{
				float value = nature.world_generator.noise_generator.base_noise(
					ci.x + dci[i].x, ci.y + dci[i].y, ci.z + dci[i].z, 13);
				if (max_value < value)
				{
					max_value = value;
					max_dci = dci[i];
				}
			}
			if (max_dci == glm::ivec3{0, 0, 0})
			{
				break;
			}
			ci += max_dci;
		}
		/* Zones of blocks are mapped to a "ci" block, and "ci" blocks are mapped to other
		 * "ci" blocks so that the shape of the final mapping does not look too much like a grid.
		 * Each "ci" block at the end of the mapping chain represents a biome type,
		 * which is represented as altitude for now. */
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		float const kind_float = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 420);
		int const kind = 2 + static_cast<int>(kind_float * 20.0f);
		ptg_field[coords] = (coords.z <= -value * 100.0f) ? kind : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes4::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = octaved_noise(coords, i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 100.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 70.0f * nature.world_generator.terrain_param_b;
		auto iize = [](int a, int b)
		{
			return (a - cool_mod(a, b)) / b;
		};
		glm::ivec3 ci{
			iize(c.x, h),
			iize(c.y, h),
			iize(c.z, 200)};
		/* Trying to get some ci blocks to map to neighbors to make same "biome" block zones. */
		glm::ivec3 initial_ci{ci};
		int asking_neighbor_count = 0;
		while (asking_neighbor_count <= 30)
		{
			asking_neighbor_count++;
			/* Try to limit neighborhood to horizontal plane neighbors for simplicity here. */
			glm::ivec3 dci[9] = {
				{-1, -1, 0}, {-1, 0, 0}, {-1, 1, 0},
				{0, -1, 0}, {0, 0, 0}, {0, 1, 0},
				{1, -1, 0}, {1, 0, 0}, {1, 1, 0}};
			float max_value = -1.0f;
			glm::ivec3 max_dci{0, 0, 0};
			for (int i = 0; i < 9; i++)
			{
				float value = nature.world_generator.noise_generator.base_noise(
					ci.x + dci[i].x, ci.y + dci[i].y, ci.z + dci[i].z, 13);
				if (max_value < value)
				{
					max_value = value;
					max_dci = dci[i];
				}
			}
			if (max_dci == glm::ivec3{0, 0, 0})
			{
				break;
			}
			ci += max_dci;
		}
		/* Zones of blocks are mapped to a "ci" block, and "ci" blocks are mapped to other
		 * "ci" blocks so that the shape of the final mapping does not look too much like a grid.
		 * Each "ci" block at the end of the mapping chain represents a biome type,
		 * which is represented as altitude for now. */
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		float const kind_float = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 420);
		bool const full = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) > 0.7f;
		int const kind = 2 + static_cast<int>(kind_float * 20.0f);
		ptg_field[coords] = (full || coords.z <= -value * 100.0f) ? kind : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes5::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = octaved_noise(coords, i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 100.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 120.0f * nature.world_generator.terrain_param_b;
		auto iize = [](int a, int b)
		{
			return (a - cool_mod(a, b)) / b;
		};
		glm::ivec3 ci{
			iize(c.x, h),
			iize(c.y, h),
			iize(c.z, h)};
		/* Trying to get some ci blocks to map to neighbors to make same "biome" block zones. */
		glm::ivec3 initial_ci{ci};
		int asking_neighbor_count = 0;
		while (asking_neighbor_count <= 10)
		{
			asking_neighbor_count++;
			/* Try to limit neighborhood to horizontal plane neighbors for simplicity here. */
			glm::ivec3 dci[9] = {
				{-1, -1, 0}, {-1, 0, 0}, {-1, 1, 0},
				{0, -1, 0}, {0, 0, 0}, {0, 1, 0},
				{1, -1, 0}, {1, 0, 0}, {1, 1, 0}};
			float max_value = -1.0f;
			glm::ivec3 max_dci{0, 0, 0};
			for (int i = 0; i < 9; i++)
			{
				float value = nature.world_generator.noise_generator.base_noise(
					ci.x + dci[i].x, ci.y + dci[i].y, ci.z + dci[i].z, 13);
				if (max_value < value)
				{
					max_value = value;
					max_dci = dci[i];
				}
			}
			if (max_dci == glm::ivec3{0, 0, 0})
			{
				break;
			}
			ci += max_dci;
		}
		/* Zones of blocks are mapped to a "ci" block, and "ci" blocks are mapped to other
		 * "ci" blocks so that the shape of the final mapping does not look too much like a grid.
		 * Each "ci" block at the end of the mapping chain represents a biome type,
		 * which is represented as altitude for now. */
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		float const kind_float = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 420);
		bool const full = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) > 0.7f;
		int const kind = 2 + static_cast<int>(kind_float * 20.0f);
		float const altitude = static_cast<float>(ci.z) + static_cast<float>(h) * value;
		ptg_field[coords] = (full || coords.z <= altitude) ? kind : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes6::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = octaved_noise(coords, i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 100.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 180.0f * nature.world_generator.terrain_param_b;
		auto iize = [](int a, int b)
		{
			return (a - cool_mod(a, b)) / b;
		};
		glm::ivec3 ci{
			iize(c.x, h),
			iize(c.y, h),
			iize(c.z, h)};
		/* Trying to get some ci blocks to map to neighbors to make same "biome" block zones. */
		glm::ivec3 initial_ci{ci};
		int asking_neighbor_count = 0;
		while (asking_neighbor_count <= 10)
		{
			asking_neighbor_count++;
			/* Try to limit neighborhood to horizontal plane neighbors for simplicity here. */
			glm::ivec3 dci[9] = {
				{-1, -1, 0}, {-1, 0, 0}, {-1, 1, 0},
				{0, -1, 0}, {0, 0, 0}, {0, 1, 0},
				{1, -1, 0}, {1, 0, 0}, {1, 1, 0}};
			float max_value = -1.0f;
			glm::ivec3 max_dci{0, 0, 0};
			for (int i = 0; i < 9; i++)
			{
				float value = nature.world_generator.noise_generator.base_noise(
					ci.x + dci[i].x, ci.y + dci[i].y, ci.z + dci[i].z, 13);
				if (max_value < value)
				{
					max_value = value;
					max_dci = dci[i];
				}
			}
			if (max_dci == glm::ivec3{0, 0, 0})
			{
				break;
			}
			ci += max_dci;
		}
		/* Zones of blocks are mapped to a "ci" block, and "ci" blocks are mapped to other
		 * "ci" blocks so that the shape of the final mapping does not look too much like a grid.
		 * Each "ci" block at the end of the mapping chain represents a biome type,
		 * which is represented as altitude for now. */
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		float const kind_float = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 420);
		bool const full = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) > 0.85f;
		bool const empty = (not full) && nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) > 0.40f;
		int const kind = 2 + static_cast<int>(kind_float * 20.0f);
		float const altitude = (static_cast<float>(ci.z) + value) * static_cast<float>(h);
		ptg_field[coords] = empty ? 0 : (full || coords.z <= altitude) ? kind : 0;
	}
	return ptg_field;
}

ChunkPtgField PlainTerrainGeneratorLameBiomes7::generate_chunk_ptg_field(
	ChunkCoords chunk_coords, Nature const& nature)
{
	/* TODO: Make a real function out of this. */
	auto octaved_noise = [&](BlockCoords coords, int channel)
	{
		float const noise_size = nature.world_generator.noise_size * 5.0f;
		constexpr unsigned int octave_number = 4;
		float value_sum = 0.0f, coef_sum = 0.0f;
		for (int i = 0; i < octave_number; i++)
		{
			float value = nature.world_generator.noise_generator.base_noise(
				#define H(axis_) \
					static_cast<float>(coords.axis_) / \
						(noise_size / static_cast<float>(1 << i))
					H(x), H(y), H(z),
				#undef H
				2 + i + 17 * channel);
			float coef = 1.0f / static_cast<float>(1 << i);
			value_sum += value * coef;
			coef_sum += coef;
		}
		float const value = value_sum / coef_sum;
		return value;
	};

	ChunkPtgField ptg_field{chunk_coords};
	for (BlockCoords coords : chunk_block_rect(chunk_coords))
	{
		glm::vec3 d;
		for (int i = 0; i < 3; i++)
		{
			d[i] = octaved_noise(coords, i) * 2.0f - 1.0f;
		}
		glm::vec3 c = coords;
		d *= 100.0f * nature.world_generator.terrain_param_a;
		c += d;
		int const h = 180.0f * nature.world_generator.terrain_param_b;
		auto iize = [](int a, int b)
		{
			return (a - cool_mod(a, b)) / b;
		};
		glm::ivec3 ci{
			iize(c.x, h),
			iize(c.y, h),
			iize(c.z, h)};
		/* Trying to get some ci blocks to map to neighbors to make same "biome" block zones. */
		glm::ivec3 initial_ci{ci};
		int asking_neighbor_count = 0;
		while (asking_neighbor_count <= 10)
		{
			asking_neighbor_count++;
			/* Try to limit neighborhood to horizontal plane neighbors for simplicity here. */
			glm::ivec3 dci[9] = {
				{-1, -1, 0}, {-1, 0, 0}, {-1, 1, 0},
				{0, -1, 0}, {0, 0, 0}, {0, 1, 0},
				{1, -1, 0}, {1, 0, 0}, {1, 1, 0}};
			float max_value = -1.0f;
			glm::ivec3 max_dci{0, 0, 0};
			for (int i = 0; i < 9; i++)
			{
				float value = nature.world_generator.noise_generator.base_noise(
					ci.x + dci[i].x, ci.y + dci[i].y, ci.z + dci[i].z, 13);
				if (max_value < value)
				{
					max_value = value;
					max_dci = dci[i];
				}
			}
			if (max_dci == glm::ivec3{0, 0, 0})
			{
				break;
			}
			ci += max_dci;
		}
		/* Zones of blocks are mapped to a "ci" block, and "ci" blocks are mapped to other
		 * "ci" blocks so that the shape of the final mapping does not look too much like a grid.
		 * Each "ci" block at the end of the mapping chain represents a biome type,
		 * which is represented as altitude for now. */
		float const value = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 69);
		float const kind_float = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 420);
		bool const full = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) > 0.85f;
		bool const empty = (not full) && nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) > 0.40f;
		int const kind = 2 + static_cast<int>(kind_float * 20.0f);
		float const altitude = (static_cast<float>(ci.z) + value) * static_cast<float>(h);
		bool const web = nature.world_generator.noise_generator.base_noise(
			ci.x, ci.y, ci.z, 1234) < 0.1f;
		if (web)
		{
			float const value_a = octaved_noise(coords, 1);
			float const value_b = octaved_noise(coords, 2);
			float const inf = 0.5f - 0.02f * 1.0f;
			float const sup = 0.5f + 0.02f * 1.0f;
			ptg_field[coords] =
				(inf <= value_a && value_a <= sup) &&
				(inf <= value_b && value_b <= sup) ?
					kind : 0;
		}
		else
		{
			ptg_field[coords] = empty ? 0 : (full || coords.z <= altitude) ? kind : 0;
		}
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
	if (name == "octaves_1"sv)         return new PlainTerrainGeneratorOctaves1{};
	if (name == "octaves_2"sv)         return new PlainTerrainGeneratorOctaves2{};
	if (name == "noise_test_1"sv)      return new PlainTerrainGeneratorNoiseTest1{};
	if (name == "noise_test_2"sv)      return new PlainTerrainGeneratorNoiseTest2{};
	if (name == "noise_test_3"sv)      return new PlainTerrainGeneratorNoiseTest3{};
	if (name == "noise_test_4"sv)      return new PlainTerrainGeneratorNoiseTest4{};
	if (name == "caves_1"sv)           return new PlainTerrainGeneratorCaves1{};
	if (name == "lame_biomes_1"sv)     return new PlainTerrainGeneratorLameBiomes1{};
	if (name == "lame_biomes_2"sv)     return new PlainTerrainGeneratorLameBiomes2{};
	if (name == "lame_biomes_3"sv)     return new PlainTerrainGeneratorLameBiomes3{};
	if (name == "lame_biomes_4"sv)     return new PlainTerrainGeneratorLameBiomes4{};
	if (name == "lame_biomes_5"sv)     return new PlainTerrainGeneratorLameBiomes5{};
	if (name == "lame_biomes_6"sv)     return new PlainTerrainGeneratorLameBiomes6{};
	if (name == "lame_biomes_7"sv)     return new PlainTerrainGeneratorLameBiomes7{};
	assert(false /* Unknown plain terrain generator name. */);
	std::exit(EXIT_FAILURE);
}

} /* qwy2 */
