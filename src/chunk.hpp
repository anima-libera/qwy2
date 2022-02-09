
#ifndef QWY2_HEADER_CHUNK_
#define QWY2_HEADER_CHUNK_

#include "opengl.hpp"
#include "nature.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <ostream>

namespace qwy2 {

enum class Axis
{
	X = 0,
	Y = 1,
	Z = 2,
};

class BlockCoordsLevel {};
class ChunkCoordsLevel {};

template<typename T>
inline constexpr bool is_coords_level =
	std::is_same_v<T, BlockCoordsLevel> ||
	std::is_same_v<T, ChunkCoordsLevel>;

template<typename L>
class CoordsInt
{
	static_assert(is_coords_level<L>);

public:
	union
	{
		std::array<int, 3> arr;
		
		/* Here are some compiler-specific (ugh) pragmas to allow
		 * for an anonymous struct to exist, for aesthetic purposes.
		 * Inspired from the GLM source code. */
		#if defined(__clang__)
			#pragma clang diagnostic push
			#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
			#pragma clang diagnostic ignored "-Wnested-anon-types"		
		#elif defined(__GNUG__)
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpedantic"
		#endif
		struct { int x, y, z; };
		#if defined(__clang__)
			#pragma clang diagnostic pop
		#elif defined(__GNUG__)
			#pragma GCC diagnostic pop
		#endif
		/* TODO: Try to do proper C++ here, such as by overloading the [] operator
		 * and defining it with some inline (this+n) dereferancing if some static assertions
		 * hold, and define it case by case otherwise. */
	};

public:
	CoordsInt(int x, int y, int z);
	glm::vec3 to_float_coords() const;	
	bool operator==(CoordsInt const& other) const;

public:
	class Hash
	{
	public:
		std::size_t operator()(CoordsInt const& coords) const noexcept;
	};
};

template<typename L>
std::ostream& operator<<(std::ostream& out_stream, CoordsInt<L> const& coords);

template<typename L>
class RectInt
{
	static_assert(is_coords_level<L>);

public:
	CoordsInt<L> coords_min; /* Included. */
	CoordsInt<L> coords_max; /* Included. */

public:
	RectInt(CoordsInt<L> coords_min, CoordsInt<L> coords_max);
	RectInt(CoordsInt<L> coords_center, unsigned int radius);

	template<Axis A>
	unsigned int length() const;

	unsigned int volume() const;

	bool contains(CoordsInt<L> const& coords) const;

	unsigned int to_index(CoordsInt<L> const& coords) const;

	CoordsInt<L> walker_start() const;
	bool walker_iterate(CoordsInt<L>& walker) const;
};

/* Represents an oriented face, the "interior" of which is at the internal_coords,
 * and the "exterior" of which is at the external_coords that are the internal_coords but
 * moved (of one length unit) along the given axis in the given negativewardness. */
template<typename L>
class FaceInt
{
	static_assert(is_coords_level<L>);

public:
	CoordsInt<L> internal_coords;
	Axis axis;
	bool negativeward;

public:
	FaceInt(CoordsInt<L> internal_coords, Axis axis, bool negativeward);
	CoordsInt<L> external_coords() const;
};

using BlockCoords = CoordsInt<BlockCoordsLevel>;
using BlockRect = RectInt<BlockCoordsLevel>;
using BlockFace = FaceInt<BlockCoordsLevel>;

class BlockVertexData;

class Block
{
public:
	bool is_air;
	unsigned int type_index;

public:
	Block();
	void generate_face(Nature const& nature, BlockFace const& face,
		std::vector<BlockVertexData>& dst) const;
};

template<typename VertexDataType>
class Mesh
{
public:
	std::vector<VertexDataType> vertex_data;
	GLenum opengl_buffer_usage;
	GLuint openglid;

public:
	Mesh();
	Mesh(GLenum opengl_buffer_usage);
	~Mesh();

	void update_opengl_data();
};

class ChunkGrid;
using ChunkCoords = CoordsInt<ChunkCoordsLevel>;
using ChunkRect = RectInt<ChunkCoordsLevel>;
using ChunkFace = FaceInt<ChunkCoordsLevel>;

class Chunk
{
public:
	Mesh<BlockVertexData> mesh;
	BlockRect rect;
private:
	std::vector<Block> block_grid;

public:
	Chunk(Nature& nature, BlockRect rect);
	Block& block(BlockCoords const& coords);
	void recompute_mesh(Nature const& nature);
	void add_common_faces_to_mesh(Nature const& nature, ChunkGrid& chunk_grid,
		ChunkFace chunk_face, Chunk& touching_chunk);

	//friend class Generator;
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
};

} /* qwy2 */

#endif /* QWY2_HEADER_CHUNK_ */
