
#ifndef QWY2_HEADER_COORDS_
#define QWY2_HEADER_COORDS_

#include <iosfwd>
#include <tuple>
#include <glm/vec3.hpp>

namespace qwy2
{

/* There are two natural length units here: the length of the edge of a block
 * and the length of the edge of a chunk. The latter should remain specific to
 * chunk handling, and thus the coordinate system used to map the 3D space of
 * the world considers that the edge of a block is of length 1.
 * Here, the vertical axis is Z (not Y), integer coordinates correspond to points
 * at the center of blocks (not at the corner of blocks), chunks are of odd side
 * length so that they have a center block, and the center block of the chunk at
 * the (0, 0, 0) chunk coords is the block at the (0, 0, 0) block coords.
 * Integer block coords refer to points that are at the center of blocks. */

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
	operator glm::vec3() const;
	bool operator==(CoordsInt const& other) const;
	bool operator!=(CoordsInt const& other) const;
	CoordsInt operator+(CoordsInt other) const;
	CoordsInt operator-(CoordsInt other) const;

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

	/* Considering a 3D array having a cell per integer coords contained in this rect,
	 * then this method returns the index in this array corresponding to the given coords. */
	unsigned int to_index(CoordsInt<L> const& coords) const;

	bool operator==(RectInt const& right) const;
	bool operator!=(RectInt const& right) const;

	/* This Iterator class, along with the associated begin and end methods, allow for
	 * range-based for loops to be used to iterate over the integer coords contained
	 * in the rect. */
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
 * and the "exterior" of which is at the external coords that are the internal_coords but
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
	
	/* The first index returned corresponds to the axis of this face.
	 * The other two are the other two indices in {0, 1, 2}. */
	std::tuple<unsigned int, unsigned int, unsigned int> indices_axis_a_b() const;
};

using BlockCoords = CoordsInt<CoordsLevel::BLOCK>;
using BlockRect = RectInt<CoordsLevel::BLOCK>;
using BlockFace = FaceInt<CoordsLevel::BLOCK>;

using ChunkCoords = CoordsInt<CoordsLevel::CHUNK>;
using ChunkRect = RectInt<CoordsLevel::CHUNK>;
using ChunkFace = FaceInt<CoordsLevel::CHUNK>;

/* A 3D float rect aligned with the blocks, using block level coords. */
class AlignedBox
{
public:
	glm::vec3 center;
	glm::vec3 dimensions;

public:
	AlignedBox();
	AlignedBox(glm::vec3 center, glm::vec3 dimensions);

	/* Returns the smallest block rect that contains the aligned box. */
	BlockRect containing_block_rect() const;
	
	std::pair<glm::vec3, glm::vec3> coords_min_max() const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_COORDS_ */
