
#ifndef QWY2_HEADER_CHUNK_
#define QWY2_HEADER_CHUNK_

#include "opengl.hpp"
#include "nature.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <ostream>

namespace qwy2
{

/* There are two natural length units here: the length of the edge of a block
 * and the length of the edge of a chunk. The latter should remain specific to
 * chunk handling, and thus the coordinate system used to map the 3D space of
 * the world considers that the edge of a block is of length 1.
 * Here, the vertical axis is Z (not Y), integer coordinates correspond to points
 * at the center of blocks (not at the corner of blocks), chunks are of odd side
 * length so that they have a center block, and the center block of the chunk at
 * the (0, 0, 0) chunk coords is the block at the (0, 0, 0) block coords. */

/* Used to represent one non-oriented axis. The vertical axis is Z.
 * If needed, an orientation is often given as a boolean negativewardness. */
enum class Axis
{
	X = 0,
	Y = 1,
	Z = 2,
};

enum class CoordsLevel
{
	/* The associated coords types describe points in the world,
	* with the edge of a block being of length 1. */
	BLOCK,

	/* The associated coords types describe chunks in the chunk grid,
	* with the edge of a chunk being of length 1 (for what this is worth). */
	CHUNK,
};

/* Integer 3D coords. */
template<CoordsLevel L>
class CoordsInt
{
public:
	int x, y, z;

public:
	CoordsInt();
	CoordsInt(int x, int y, int z);
	int& operator[](int index);
	int const& operator[](int index) const;
	glm::vec3 to_float_coords() const;
	bool operator==(CoordsInt const& other) const;
	bool operator!=(CoordsInt const& other) const;

public:
	/* Callable hash that allows CoordsInt values to be used as keys
	 * in std::unordered_map (that is implemented as a hash map). */
	class Hash
	{
	public:
		std::size_t operator()(CoordsInt const& coords) const noexcept;
	};
};

template<CoordsLevel L>
std::ostream& operator<<(std::ostream& out_stream, CoordsInt<L> const& coords);

/* A 3D axis-aligned rectangle area of integer coords,
 * such as a recangle of blocks or a ractangle of chunks. */
template<CoordsLevel L>
class RectInt
{
public:
	CoordsInt<L> coords_min; /* Included. */
	CoordsInt<L> coords_max; /* Included. */

public:
	RectInt();
	RectInt(CoordsInt<L> coords_min, CoordsInt<L> coords_max);
	RectInt(CoordsInt<L> coords_center, unsigned int radius);
	RectInt(CoordsInt<L> coords_center,
		unsigned int radius_x, unsigned int radius_y, unsigned int radius_z);

	template<Axis A>
	unsigned int length() const;

	unsigned int volume() const;

	bool contains(CoordsInt<L> const& coords) const;

	unsigned int to_index(CoordsInt<L> const& coords) const;

	bool operator==(RectInt const& right) const;
	bool operator!=(RectInt const& right) const;

	class Iterator
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using difference_type = int;
		using value_type = CoordsInt<L>;
		using pointer = value_type*;
		using reference = value_type&;

	public:
		CoordsInt<L> current_coords;
		RectInt const* containing_rect;
	
	public:
		Iterator(CoordsInt<L> starting_coords, RectInt const* containing_rect);
		CoordsInt<L> const& operator*() const;
		Iterator& operator++();
		Iterator operator++(int);
		bool operator==(Iterator const& right) const;
		bool operator!=(Iterator const& right) const;
	};

	Iterator begin() const;
	Iterator end() const;
};

/* Represents an oriented face, the "interior" of which is at the internal_coords,
 * and the "exterior" of which is at the external_coords that are the internal_coords but
 * moved (of one length unit) along the given axis in the given negativewardness. */
template<CoordsLevel L>
class FaceInt
{
public:
	CoordsInt<L> internal_coords;
	Axis axis;
	bool negativeward;

public:
	FaceInt(CoordsInt<L> internal_coords, Axis axis, bool negativeward);
	CoordsInt<L> external_coords() const;
};

using BlockCoords = CoordsInt<CoordsLevel::BLOCK>;
using BlockRect = RectInt<CoordsLevel::BLOCK>;
using BlockFace = FaceInt<CoordsLevel::BLOCK>;

class VertexDataClassic;

/* Describes the state of one voxel in a grid of voxels. */
class Block
{
public:
	bool is_air;
	unsigned int type_index;

public:
	Block();
	void generate_face(Nature const& nature, BlockFace const& face,
		std::vector<VertexDataClassic>& dst) const;
};

template<typename VertexDataType>
class Mesh
{
public:
	std::vector<VertexDataType> vertex_data;
	GLenum opengl_buffer_usage;
	GLuint openglid;
	bool needs_update_opengl_data;

public:
	Mesh();
	Mesh(GLenum opengl_buffer_usage);
	~Mesh();

	void update_opengl_data();
};

class ChunkGrid;
using ChunkCoords = CoordsInt<CoordsLevel::CHUNK>;
using ChunkRect = RectInt<CoordsLevel::CHUNK>;
using ChunkFace = FaceInt<CoordsLevel::CHUNK>;

class GeneratingChunk
{
public:
	ChunkCoords chunk_coords;
	BlockRect rect;
	std::vector<Block> block_grid;
	bool is_all_air;
	std::vector<VertexDataClassic> vertex_data;
};

GeneratingChunk* generate_chunk(ChunkCoords chunk_coords, BlockRect rect, Nature const& nature);

class Chunk
{
public:
	Mesh<VertexDataClassic> mesh;
	BlockRect rect;
	bool is_all_air;
	bool is_generated;
	bool is_just_generated;
private:
	std::vector<Block> block_grid;

public:
	Chunk(BlockRect rect);
	Block& block(BlockCoords const& coords);
	void generate(Nature& nature);
	void recompute_mesh(Nature const& nature);
	void add_common_faces_to_mesh(Nature const& nature,
		ChunkFace chunk_face, Chunk& touching_chunk);

	//friend class Generator;
	friend class ChunkGrid;
};

class ChunkGrid
{
public:
	int chunk_side; /* Should stay signed, and must be odd. */
	std::unordered_map<ChunkCoords, Chunk*, ChunkCoords::Hash> table; /* Center to chunk. */

public:
	ChunkGrid(int chunk_side);

	ChunkCoords containing_chunk_coords(BlockCoords coords) const;
	BlockRect chunk_rect(ChunkCoords chunk_coords) const;

	BlockCoords containing_chunk_center_coords(BlockCoords coords) const;
	BlockRect containing_chunk_rect(BlockCoords coords) const;
	BlockRect containing_chunk_rect(glm::vec3 coords) const;

	Chunk* chunk(ChunkCoords chunk_coords);
	Chunk* containing_chunk(BlockCoords coords);
	Chunk* containing_chunk(glm::vec3 coords);

	Chunk* generate_chunk(Nature& nature, ChunkCoords chunk_coords);
	Chunk* add_generated_chunk(GeneratingChunk* generating_chunk, ChunkCoords chunk_coords,
		Nature const& nature);

	bool block_is_air_or_not_generated(BlockCoords coords);
};

} /* qwy2 */

#endif /* QWY2_HEADER_CHUNK_ */
