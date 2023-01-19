
#include "line_rect.hpp"
#include "mesh.hpp"
#include "coords.hpp"
#include <utility>
#include <array>

namespace qwy2
{

LineRectDrawer::LineRectDrawer()
{
	;
}

void LineRectDrawer::set_box(AlignedBox const& aligned_box)
{
	/* TODO: Optimize this mesh use. */
	this->mesh.vertex_data.clear();
	this->mesh.vertex_data.reserve(24);

	VertexDataLine vertex;
	vertex.color = this->color;

	auto [coords_min, coords_max] = aligned_box.coords_min_max();
	
	for (Axis axis : {Axis::X, Axis::Y, Axis::Z})
	{
		unsigned int const index_axis = static_cast<int>(axis);
		unsigned int const index_a = index_axis == 0 ? 1 : 0;
		unsigned int const index_b = index_axis == 2 ? 1 : 2;

		std::array<std::pair<bool, bool>, 4> const possibilities_ab{
			std::make_pair(false, false),
			std::make_pair(true, false),
			std::make_pair(true, true),
			std::make_pair(false, true)};
		
		for (auto [negativeward_a, negativeward_b] : possibilities_ab)
		{
			vertex.coords[index_a] =
				negativeward_a ? coords_min[index_a] : coords_max[index_a];
			vertex.coords[index_b] =
				negativeward_b ? coords_min[index_b] : coords_max[index_b];

			for (bool negativeward : {false, true})
			{
				vertex.coords[index_axis] =
					negativeward ? coords_min[index_axis] : coords_max[index_axis];
				
				this->mesh.vertex_data.push_back(vertex);
			}
		}
	}

	this->mesh.update_opengl_data();
}

} /* qwy2 */
