
#include "chunk.hpp"
#include "nature.hpp"
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

void Block::generate_face(Nature const& nature,
	CoordsInt coords, Axis axis, bool negativeward,
	std::vector<float>& dst) const
{
	int index_axis = static_cast<int>(axis);
	int index_a = index_axis == 0 ? 1 : 0;
	int index_b = index_axis == 2 ? 1 : 2;

	BlockType const& type = nature.block_type_table[this->type_index];
	AtlasRect atlas_rect = axis == Axis::Z ?
		(negativeward ? type.fase_bottom_rect : type.fase_top_rect) :
		type.fase_vertical_rect;
	if ((axis == Axis::Y && negativeward) || (axis == Axis::X && not negativeward))
	{
		std::swap(atlas_rect.atlas_coords_min.x, atlas_rect.atlas_coords_max.x);
	}

	struct VertexData
	{
		glm::vec3 coords;
		glm::vec2 atlas_coords;
	};

	glm::vec3 coords_nn = coords.to_float_coords() - glm::vec3(0.5f, 0.5f, 0.5f);
	coords_nn[index_axis] += negativeward ? 0.0f : 1.0f;

	VertexData nn;
	nn.coords = coords_nn;
	nn.coords[index_a] += 0.0f;
	nn.coords[index_b] += 0.0f;
	nn.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	nn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	VertexData np;
	np.coords = coords_nn;
	np.coords[index_a] += 0.0f;
	np.coords[index_b] += 1.0f;
	np.atlas_coords.x = atlas_rect.atlas_coords_min.x;
	np.atlas_coords.y = atlas_rect.atlas_coords_max.y;
	VertexData pn;
	pn.coords = coords_nn;
	pn.coords[index_a] += 1.0f;
	pn.coords[index_b] += 0.0f;
	pn.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pn.atlas_coords.y = atlas_rect.atlas_coords_min.y;
	VertexData pp;
	pp.coords = coords_nn;
	pp.coords[index_a] += 1.0f;
	pp.coords[index_b] += 1.0f;
	pp.atlas_coords.x = atlas_rect.atlas_coords_max.x;
	pp.atlas_coords.y = atlas_rect.atlas_coords_max.y;

	std::array<VertexData, 6> vertex_data_sequence{nn, pn, pp, nn, pp, np};

	dst.reserve(dst.size() + vertex_data_sequence.size() * 5);
	for (VertexData& vertex_data : vertex_data_sequence)
	{
		dst.push_back(vertex_data.coords.x);
		dst.push_back(vertex_data.coords.y);
		dst.push_back(vertex_data.coords.z);
		dst.push_back(vertex_data.atlas_coords.x);
		dst.push_back(vertex_data.atlas_coords.y);
		//std::cout << "vertex coords " <<
		//	dst[dst.size()-5+0] << ", " << dst[dst.size()-5+1] << ", " << dst[dst.size()-5+2] <<
		//	" atlas coords " <<
		//	dst[dst.size()-5+3] << ", " << dst[dst.size()-5+4] << std::endl;
	}
}

Chunk::Chunk(Nature const& nature, RectInt rect):
	rect(rect)
{
	this->block_grid.resize(rect.volume());

	glGenBuffers(1, &this->mesh_openglid);
	this->recompute_mesh(nature);
}

Block& Chunk::block(CoordsInt const& coords)
{
	return this->block_grid[this->rect.to_index(coords)];
}

void Chunk::recompute_mesh(Nature const& nature)
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
					//std::cout << "generate face " <<
					//	walker.x << ", " << walker.y << ", " << walker.z << " -> " <<
					//	neighbor.x << ", " << neighbor.y << ", " << neighbor.z << std::endl;
					this->block(walker).generate_face(nature,
						walker, axis, negativeward, mesh_data);
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
	return mesh_data.size() / 5;
}

} /* Qwy2 */
