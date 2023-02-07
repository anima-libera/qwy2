
#ifndef QWY2_HEADER_TERRAIN_GEN_
#define QWY2_HEADER_TERRAIN_GEN_

#include "coords.hpp"
#include "chunk.hpp"

namespace qwy2
{

class PlainTerrainGenerator
{
public:
	virtual ChunkPtgField generate_chunk_ptg_field(
		ChunkCoords chunk_coords, Nature const& nature) = 0;
};

#define SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(class_name_) \
	class class_name_: public PlainTerrainGenerator \
	{ \
	public: \
		virtual ChunkPtgField generate_chunk_ptg_field( \
			ChunkCoords chunk_coords, Nature const& nature) override; \
	}

SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorFlat);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorClassic);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorHills);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorHomogenous);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorPlane);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorPlanes);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorVerticalPillar);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorVerticalHole);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorHorizontalPillar);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorHorizontalHole);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorFunky1);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorFunky2);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorOctaves1);
SIMPLE_PLAIN_TERRAIN_GENERATOR_SUBCLASS(PlainTerrainGeneratorOctaves2);

PlainTerrainGenerator* plain_terrain_generator_from_name(std::string_view name);

} /* qwy2 */

#endif /* QWY2_HEADER_TERRAIN_GEN_ */