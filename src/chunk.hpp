
#ifndef QWY2_HEADER_CHUNK_
#define QWY2_HEADER_CHUNK_

#include "opengl.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <iterator>

namespace qwy2 {

enum class Axis
{
	X = 0,
	Y = 1,
	Z = 2,
};

class CoordsInt
{
public:
	union
	{
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
		std::array<int, 3> arr;
	};

public:
	CoordsInt(int x, int y, int z);
	glm::vec3 to_float_coords() const;	
	bool operator==(CoordsInt const& other) const;
};

class RectInt
{
private:
	CoordsInt coords_min; /* Included. */
	CoordsInt coords_max; /* Included. */

public:
	RectInt(CoordsInt coords_min, CoordsInt coords_max);

	template<Axis A>
	unsigned int length() const;
	unsigned int volume() const;

	bool contains(CoordsInt const& coords) const;

	unsigned int to_index(CoordsInt const& coords) const;

	CoordsInt walker_start() const;
	bool walker_iterate(CoordsInt& walker) const;
};

class Block
{
public:
	bool is_air;
	glm::vec3 color;

public:
	Block();
	void generate_face(CoordsInt coords, Axis axis, bool negativeward,
		std::vector<float>& dst) const;
};

class Chunk
{
public:
	std::vector<float> mesh_data;
	GLuint mesh_openglid;
	RectInt rect;
private:
	std::vector<Block> block_grid;

public:
	Chunk(RectInt rect);
	Block& block(CoordsInt const& coords);
	void recompute_mesh();
	unsigned int mesh_vertex_count() const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_CHUNK_ */
