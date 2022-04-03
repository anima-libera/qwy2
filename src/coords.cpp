
#include "coords.hpp"
#include <cmath>

namespace qwy2
{


template<CoordsLevel L>
CoordsInt<L>::CoordsInt():
	x{0}, y{0}, z{0}
{
	;
}

template<CoordsLevel L>
CoordsInt<L>::CoordsInt(int x, int y, int z):
	x{x}, y{y}, z{z}
{
	;
}

template<CoordsLevel L>
int& CoordsInt<L>::operator[](int index)
{
	return (&x)[index];
}

template<CoordsLevel L>
int const& CoordsInt<L>::operator[](int index) const
{
	return (&x)[index];
}

template<CoordsLevel L>
CoordsInt<L>::operator glm::vec3() const
{
	return glm::vec3{this->x, this->y, this->z};
}

template<CoordsLevel L>
bool CoordsInt<L>::operator==(CoordsInt const& other) const
{
	/* TODO: Find some C++17-idiomatic way to remove this kind of code duplication. */
	return this->x == other.x && this->y == other.y && this->z == other.z;
}

template<CoordsLevel L>
bool CoordsInt<L>::operator!=(CoordsInt const& other) const
{
	return not (*this == other);
}

template<CoordsLevel L>
std::size_t CoordsInt<L>::Hash::operator()(CoordsInt const& coords) const noexcept
{
	return
		std::hash<int>()(coords.x ^ 10000) ^
		std::hash<int>()(coords.y ^ 54678) ^
		std::hash<int>()(coords.z ^ 22263);
}

template class CoordsInt<CoordsLevel::BLOCK>;
template class CoordsInt<CoordsLevel::CHUNK>;

template<CoordsLevel L>
std::ostream& operator<<(std::ostream& out_stream, CoordsInt<L> const& coords)
{
	out_stream << "(" << coords.x << ", " << coords.y << ", " << coords.z << ")";
	return out_stream;
}

template std::ostream& operator<<(std::ostream& out_stream,
	CoordsInt<CoordsLevel::BLOCK> const& coords);
template std::ostream& operator<<(std::ostream& out_stream,
	CoordsInt<CoordsLevel::CHUNK> const& coords);

template<CoordsLevel L>
RectInt<L>::RectInt():
	coords_min{0, 0, 0}, coords_max{0, 0, 0}
{
	;
}

template<CoordsLevel L>
RectInt<L>::RectInt(CoordsInt<L> coords_min, CoordsInt<L> coords_max):
	coords_min{coords_min}, coords_max{coords_max}
{
	assert(coords_min.x <= coords_max.x);
	assert(coords_min.y <= coords_max.y);
	assert(coords_min.z <= coords_max.z);
}

template<CoordsLevel L>
RectInt<L>::RectInt(CoordsInt<L> coords_center, unsigned int radius):
	coords_min{
		coords_center.x - static_cast<int>(radius - 1),
		coords_center.y - static_cast<int>(radius - 1),
		coords_center.z - static_cast<int>(radius - 1)},
	coords_max{
		coords_center.x + static_cast<int>(radius - 1),
		coords_center.y + static_cast<int>(radius - 1),
		coords_center.z + static_cast<int>(radius - 1)}
{
	;
}

template<CoordsLevel L>
RectInt<L>::RectInt(CoordsInt<L> coords_center,
	unsigned int radius_x, unsigned int radius_y, unsigned int radius_z
):
	coords_min{
		coords_center.x - static_cast<int>(radius_x - 1),
		coords_center.y - static_cast<int>(radius_y - 1),
		coords_center.z - static_cast<int>(radius_z - 1)},
	coords_max{
		coords_center.x + static_cast<int>(radius_x - 1),
		coords_center.y + static_cast<int>(radius_y - 1),
		coords_center.z + static_cast<int>(radius_z - 1)}
{
	;
}

template<CoordsLevel L>
template<Axis A>
unsigned int RectInt<L>::length() const
{
	int const axis = static_cast<int>(A);
	return this->coords_max[axis] - this->coords_min[axis] + 1;
}

template<CoordsLevel L>
unsigned int RectInt<L>::volume() const
{
	return
		this->length<Axis::X>() *
		this->length<Axis::Y>() *
		this->length<Axis::Z>();
}

template<CoordsLevel L>
bool RectInt<L>::contains(CoordsInt<L> const& coords) const
{
	return
		this->coords_min.x <= coords.x && coords.x <= this->coords_max.x &&
		this->coords_min.y <= coords.y && coords.y <= this->coords_max.y &&
		this->coords_min.z <= coords.z && coords.z <= this->coords_max.z;
}

template<CoordsLevel L>
unsigned int RectInt<L>::to_index(CoordsInt<L> const& coords) const
{
	assert(this->contains(coords));
	return
		(coords.x - this->coords_min.x) +
		(coords.y - this->coords_min.y) * this->length<Axis::X>() +
		(coords.z - this->coords_min.z) * this->length<Axis::X>() * this->length<Axis::Y>();
}

template<CoordsLevel L>
bool RectInt<L>::operator==(RectInt const& right) const
{
	return this->coords_min == right.coords_min && this->coords_max == right.coords_max;
}

template<CoordsLevel L>
bool RectInt<L>::operator!=(RectInt const& right) const
{
	return not (*this == right);
}

template<CoordsLevel L>
RectInt<L>::Iterator::Iterator(CoordsInt<L> starting_coords, RectInt const* containing_rect):
	current_coords{starting_coords}, containing_rect{containing_rect}
{
	;
}

template<CoordsLevel L>
CoordsInt<L> const& RectInt<L>::Iterator::operator*() const
{
	return this->current_coords;
}

template<CoordsLevel L>
typename RectInt<L>::Iterator& RectInt<L>::Iterator::operator++()
{
	int axis = static_cast<int>(Axis::X);
	while (axis <= static_cast<int>(Axis::Z))
	{
		this->current_coords[axis]++;
		if (this->current_coords[axis] > this->containing_rect->coords_max[axis])
		{
			this->current_coords[axis] = this->containing_rect->coords_min[axis];
			axis++;
		}
		else
		{
			return *this;
		}
	}
	/* Going past-the-end. */
	*this = this->containing_rect->end();
	return *this;
}

template<CoordsLevel L>
typename RectInt<L>::Iterator RectInt<L>::Iterator::operator++(int)
{
	RectInt<L>::Iterator copy = *this;
	++(*this);
	return copy;
}

template<CoordsLevel L>
bool RectInt<L>::Iterator::operator==(typename RectInt<L>::Iterator const& right) const
{
	assert(this->containing_rect == right.containing_rect);
	return this->current_coords == right.current_coords;
}

template<CoordsLevel L>
bool RectInt<L>::Iterator::operator!=(typename RectInt<L>::Iterator const& right) const
{
	assert(this->containing_rect == right.containing_rect);
	return this->current_coords != right.current_coords;
}

template<CoordsLevel L>
typename RectInt<L>::Iterator RectInt<L>::begin() const
{
	return RectInt<L>::Iterator{this->coords_min, this};
}

template<CoordsLevel L>
typename RectInt<L>::Iterator RectInt<L>::end() const
{
	CoordsInt<L> past_the_end = this->coords_max;
	past_the_end.x++;
	return RectInt<L>::Iterator{past_the_end, this};
}

template class RectInt<CoordsLevel::BLOCK>;
template class RectInt<CoordsLevel::CHUNK>;

template<CoordsLevel L>
FaceInt<L>::FaceInt(CoordsInt<L> internal_coords, Axis axis, bool negativeward):
	internal_coords{internal_coords}, axis{axis}, negativeward{negativeward}
{
	;
}

template<CoordsLevel L>
CoordsInt<L> FaceInt<L>::external_coords() const
{
	unsigned int index_axis = static_cast<int>(this->axis);
	CoordsInt<L> coords = this->internal_coords;
	coords[index_axis] += this->negativeward ? -1 : 1;
	return coords;
}

template<CoordsLevel L>
std::tuple<unsigned int, unsigned int, unsigned int> FaceInt<L>::indices_axis_a_b() const
{
	unsigned int const index_axis = static_cast<int>(this->axis);
	unsigned int const index_a = index_axis == 0 ? 1 : 0;
	unsigned int const index_b = index_axis == 2 ? 1 : 2;
	return std::make_tuple(index_axis, index_a, index_b);
}

template class FaceInt<CoordsLevel::BLOCK>;
template class FaceInt<CoordsLevel::CHUNK>;

AlignedBox::AlignedBox()
{
	;
}

AlignedBox::AlignedBox(glm::vec3 center, glm::vec3 dimensions):
	center{center}, dimensions{dimensions}
{
	;
}

BlockRect AlignedBox::containing_block_rect() const
{
	return BlockRect{
		BlockCoords{
			static_cast<int>(std::round(this->center.x - this->dimensions.x / 2.0f)),
			static_cast<int>(std::round(this->center.y - this->dimensions.y / 2.0f)),
			static_cast<int>(std::round(this->center.z - this->dimensions.z / 2.0f))},
		BlockCoords{
			static_cast<int>(std::round(this->center.x + this->dimensions.x / 2.0f)),
			static_cast<int>(std::round(this->center.y + this->dimensions.y / 2.0f)),
			static_cast<int>(std::round(this->center.z + this->dimensions.z / 2.0f))}};
}

std::pair<glm::vec3, glm::vec3> AlignedBox::coords_min_max() const
{
	return std::make_pair(
		glm::vec3{
			this->center.x - this->dimensions.x / 2.0f,
			this->center.y - this->dimensions.y / 2.0f,
			this->center.z - this->dimensions.z / 2.0f},
		glm::vec3{
			this->center.x + this->dimensions.x / 2.0f,
			this->center.y + this->dimensions.y / 2.0f,
			this->center.z + this->dimensions.z / 2.0f});
}

} /* qwy2 */
