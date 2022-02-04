
#include "chunk.hpp"
#include <iostream>

namespace qwy2 {

CoordsInt::CoordsInt(int x, int y, int z):
	x(x), y(y), z(z)
{
	;
}

glm::vec3 CoordsInt::to_float_coords() const
{
	return glm::vec3(this->x, this->y, this->z);
}

bool CoordsInt::operator==(CoordsInt const& other) const
{
	return this->arr == other.arr;
}

RectInt::RectInt(CoordsInt coords_min, CoordsInt coords_max):
	coords_min(coords_min), coords_max(coords_max)
{
	;
}

template<Axis A>
unsigned int RectInt::length() const
{
	const int axis = static_cast<int>(A);
	return this->coords_max.arr[axis] - this->coords_min.arr[axis] + 1;
}

unsigned int RectInt::volume() const
{
	return
		this->length<Axis::X>() *
		this->length<Axis::Y>() *
		this->length<Axis::Z>();
}

bool RectInt::contains(CoordsInt const& coords) const
{
	return
		this->coords_min.x <= coords.x && coords.x <= this->coords_max.x &&
		this->coords_min.y <= coords.y && coords.y <= this->coords_max.y &&
		this->coords_min.z <= coords.z && coords.z <= this->coords_max.z;
}

unsigned int RectInt::to_index(CoordsInt const& coords) const
{
	return
		(coords.x - this->coords_min.x) +
		(coords.y - this->coords_min.y) * this->length<Axis::X>() +
		(coords.z - this->coords_min.z) * this->length<Axis::X>() * this->length<Axis::Y>();
}

CoordsInt RectInt::walker_start() const
{
	return this->coords_min;
}

bool RectInt::walker_iterate(CoordsInt& walker) const
{
	int axis = static_cast<int>(Axis::X);
	while (axis <= static_cast<int>(Axis::Z))
	{
		walker.arr[axis]++;
		if (walker.arr[axis] > this->coords_max.arr[axis])
		{
			walker.arr[axis] = this->coords_min.arr[axis];
			axis++;
		}
		else
		{
			return true;
		}
	}
	return false;
}

Block::Block():
	is_air(true)
{
	;
}

void Block::generate_face(CoordsInt coords, Axis axis, bool negativeward,
	std::vector<float>& dst) const
{
	int index_axis = static_cast<int>(axis);
	int index_a = index_axis == 0 ? 1 : 0;
	int index_b = index_axis == 2 ? 1 : 2;

	glm::vec3 coords_nn = coords.to_float_coords() - glm::vec3(0.5f, 0.5f, 0.5f);
	coords_nn[index_axis] += negativeward ? 0.0f : 1.0f;
	glm::vec3 coords_np = coords_nn;
	coords_np[index_b] += 1.0f;
	glm::vec3 coords_pn = coords_nn;
	coords_pn[index_a] += 1.0f;
	glm::vec3 coords_pp = coords_nn;
	coords_pp[index_a] += 1.0f;
	coords_pp[index_b] += 1.0f;

	std::array<glm::vec3, 6> coords_sequence{
		coords_nn, coords_pp, coords_np, coords_nn, coords_pp, coords_pn};

	dst.reserve(dst.size() + 6 * 6);
	for (unsigned int i = 0; i < 6; i++)
	{
		glm::vec3& coords = coords_sequence[i];
		dst.push_back(coords.x);
		dst.push_back(coords.y);
		dst.push_back(coords.z);
		dst.push_back(this->color.r);
		dst.push_back(this->color.g);
		dst.push_back(this->color.b);
	}
}

Chunk::Chunk(RectInt rect):
	rect(rect)
{
	this->block_grid.resize(rect.volume());

	glGenBuffers(1, &this->mesh_openglid);
	this->recompute_mesh();
}

Block& Chunk::block(CoordsInt const& coords)
{
	return this->block_grid[this->rect.to_index(coords)];
}

void Chunk::recompute_mesh()
{
	mesh_data.clear();

	CoordsInt walker = this->rect.walker_start();
	do
	{
		//std::cout << "block " <<
		//	walker.x << ", " << walker.y << ", " << walker.z << std::endl;
		if (not this->block(walker).is_air)
		{
			//std::cout << "not air at " <<
			//	walker.x << ", " << walker.y << ", " << walker.z << std::endl;
			for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
			for (bool negativeward : {false, true})
			{
				int index_axis = static_cast<int>(axis);
				CoordsInt neighbor = walker;
				neighbor.arr[index_axis] += negativeward ? -1 : 1;
				if (not this->rect.contains(neighbor) ||
					this->block(neighbor).is_air)
				{
					this->block(walker).generate_face(walker, axis, negativeward, mesh_data);
					//std::cout << "generate face " <<
					//	walker.x << ", " << walker.y << ", " << walker.z << " -> " <<
					//	neighbor.x << ", " << neighbor.y << ", " << neighbor.z << std::endl;
				}
			}
		}
	}
	while (this->rect.walker_iterate(walker));

	glBindBuffer(GL_ARRAY_BUFFER, this->mesh_openglid);
	glBufferData(GL_ARRAY_BUFFER, this->mesh_data.size() * sizeof (float),
		this->mesh_data.data(), GL_DYNAMIC_DRAW);
}

unsigned int Chunk::mesh_vertex_count() const
{
	return mesh_data.size() / 6;
}

} /* Qwy2 */
