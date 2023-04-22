
#include "entity.hpp"
#include "gameloop.hpp"
#include <unordered_set>
#include <cassert>
#include <cmath>

namespace qwy2
{

EntityPhysics::EntityPhysics(glm::vec3 box_dimensions):
	box_dimensions(box_dimensions), is_falling(true), motion(0.0f, 0.0f, 0.0f)
{
	;
}

EntityRendering::EntityRendering():
	mesh()
{
	;
}

void EntityRendering::draw(glm::vec3 coords)
{
	/* TODO: OPTIMIZE omg this is so unoptimized!
	 * What about instanced rendering or something!? */

	float const t = g_game->time * 10.0f;

	glm::vec3 vert_coords[6] = {
		coords + glm::vec3{ 0.0f,  0.0f, +1.0f} * 0.5f,
		coords + glm::vec3{std::cos(t+TAU*0.0f/4.0f), std::sin(t+TAU*0.0f/4.0f),  0.0f} * 0.5f,
		coords + glm::vec3{std::cos(t+TAU*1.0f/4.0f), std::sin(t+TAU*1.0f/4.0f),  0.0f} * 0.5f,
		coords + glm::vec3{std::cos(t+TAU*2.0f/4.0f), std::sin(t+TAU*2.0f/4.0f),  0.0f} * 0.5f,
		coords + glm::vec3{std::cos(t+TAU*3.0f/4.0f), std::sin(t+TAU*3.0f/4.0f),  0.0f} * 0.5f,
		coords + glm::vec3{ 0.0f,  0.0f, -1.0f} * 0.5f,
	};
	glm::vec3 vert_colors[6] = {
		glm::vec3{1.0f, 0.0f, 0.0f},
		glm::vec3{0.0f, 1.0f, 0.0f},
		glm::vec3{0.0f, 0.0f, 1.0f},
		glm::vec3{1.0f, 1.0f, 0.0f},
		glm::vec3{1.0f, 1.0f, 1.0f},
		glm::vec3{1.0f, 0.0f, 1.0f}};

	mesh.vertex_data.clear();

	auto add_face = [&](int vi_0, int vi_1, int vi_2)
	{
		VertexDataSimple a{};
		VertexDataSimple b{};
		VertexDataSimple c{};
		a.coords = vert_coords[vi_0];
		a.color = vert_colors[vi_0];
		b.coords = vert_coords[vi_1];
		b.color = vert_colors[vi_1];
		c.coords = vert_coords[vi_2];
		c.color = vert_colors[vi_2];

		glm::vec3 const normal = glm::normalize(
			glm::cross(
				b.coords - a.coords,
				c.coords - a.coords));
		a.normal = normal;
		b.normal = normal;
		c.normal = normal;

		mesh.vertex_data.push_back(a);
		mesh.vertex_data.push_back(b);
		mesh.vertex_data.push_back(c);
	};

	add_face(0, 1, 2);
	add_face(0, 2, 3);
	add_face(0, 3, 4);
	add_face(0, 4, 1);
	add_face(1, 5, 2);
	add_face(2, 5, 3);
	add_face(3, 5, 4);
	add_face(4, 5, 1);

	this->mesh.update_opengl_data();

	g_game->shader_table.simple().draw(this->mesh);
}

Entity::Entity(glm::vec3 coords):
	coords(coords)
{
	;
}

Entity::Entity(glm::vec3 coords, EntityPhysics physics):
	coords(coords), physics(physics), rendering(EntityRendering{})
{
	;
}

#warning TODO: Refactor out all the cringe!
/* TODO: This was just copied from `Player::apply_motion` and tinkedred with a bit to make
 * it work, but it requires more work and to be factorized with code form `Player::apply_motion`
 * please. */
void Entity::apply_motion(float delta_time)
{
	constexpr float floor_moving_factor_fast = 0.13f;
	constexpr float floor_moving_factor_normal = 0.05f;
	constexpr float falling_moving_factor = 0.005f;
	constexpr float jump_boost_value = 0.2f;
	constexpr float falling_factor = 0.012f;
	constexpr float falling_friction_factor = 0.99f;
	constexpr float floor_friction_factor = 0.4f;

	ChunkGrid& chunk_grid = *g_game->chunk_grid;

	#if 0
	this->horizontal_angle += controls.horizontal_angle_motion * this->moving_angle_factor;
	this->vertical_angle += controls.vertical_angle_motion * this->moving_angle_factor;
	if (this->vertical_angle < -TAU / 4.0f + 0.0001f)
	{
		this->vertical_angle = -TAU / 4.0f + 0.0001f;
	}
	else if (this->vertical_angle > TAU / 4.0f - 0.0001f)
	{
		this->vertical_angle = TAU / 4.0f - 0.0001f;
	}

	glm::vec3 horizontal_direction{
		std::cos(this->horizontal_angle),
		std::sin(this->horizontal_angle),
		0.0f};
	glm::vec3 horizontal_right_direction{
		std::cos(this->horizontal_angle - TAU / 4.0f),
		std::sin(this->horizontal_angle - TAU / 4.0f),
		0.0f};

	glm::vec3 walking_motion_unnormalized =
		horizontal_direction * static_cast<float>(controls.walking_forward) +
		horizontal_right_direction * static_cast<float>(controls.walking_rightward);
	glm::vec3 walking_motion =
		glm::length(walking_motion_unnormalized) == 0.0f ? glm::vec3{0.0f, 0.0f, 0.0f} :
		glm::normalize(walking_motion_unnormalized) * (
			this->is_falling ? falling_moving_factor :
			this->allowed_fast_and_infinite_jumps ? floor_moving_factor_fast :
			floor_moving_factor_normal);
	#endif

	float delta_time_in_60_fps_frames = delta_time / 0.016f;
	if (delta_time_in_60_fps_frames >= 2.0f)
	{
		delta_time_in_60_fps_frames = 2.0f;
		/* We cannot allow for a step to apply a too big motion that did not pass through enough
		 * friction smoothing steps or something. */
	}

	assert(this->physics.has_value());
	EntityPhysics& physics = *this->physics;

	float const friction_factor = physics.is_falling ? falling_friction_factor : floor_friction_factor;
	glm::vec3 const friction = physics.motion * (1.0f - friction_factor);
	physics.motion -= friction * delta_time_in_60_fps_frames;
	physics.motion.z -= falling_factor * delta_time_in_60_fps_frames;
	#if 0
	physics.motion += walking_motion * delta_time_in_60_fps_frames;
	if (controls.will_jump_if_allowed)
	{
		bool const jump_is_allowed =
			(physics.is_falling && physics.allowed_fast_and_infinite_jumps) || (not physics.is_falling);
		if (jump_is_allowed)
		{
			physics.motion.z = jump_boost_value;
		}
	}
	#endif

	glm::vec3 motion_remaining = physics.motion * delta_time_in_60_fps_frames;
	if (not chunk_grid.has_complete_mesh(containing_chunk_coords(this->coords)))
	{
		/* Prevents falling through unloaded floor. */
		motion_remaining.z = 0.0f;
	}
	float step_max_length = 0.05f;
	glm::vec3 motion_nonfinal_step = glm::normalize(physics.motion) * step_max_length;
	BlockRect rect = AlignedBox{this->coords, physics.box_dimensions}.containing_block_rect();
	while (glm::dot(motion_remaining, motion_nonfinal_step) > 0.0f)
	{
		glm::vec3 motion_step =
			glm::length(motion_remaining) > glm::length(motion_nonfinal_step) ?
			motion_nonfinal_step : motion_remaining;
		motion_remaining -= motion_nonfinal_step;

		/* Handle collisions with blocks.
		 * TODO: Remake this from scratch and with some elegant ideas instead of patching
		 * crappy stuff until it somehow works. */
		physics.is_falling = true;
		this->coords += motion_step;
		rect = AlignedBox{this->coords, physics.box_dimensions}.containing_block_rect();
		BlockCoords center_coords_int{
			static_cast<int>(std::round(this->coords.x)),
			static_cast<int>(std::round(this->coords.y)),
			static_cast<int>(std::round(this->coords.z))};
		std::unordered_set<BlockCoords, BlockCoords::Hash> collision_blacklist;
		constexpr unsigned int max_steps_collisions = 30;
		for (unsigned int collision_step = 0;
			collision_step < max_steps_collisions; collision_step++)
		{
			rect = AlignedBox{this->coords, physics.box_dimensions}.containing_block_rect();

			/* Find the closest colliding block, and forget the others for now.
			 * If that is not enough then we will end up back here anyway. */
			std::vector<BlockCoords> collisions;
			for (BlockCoords coords : rect)
			{
				if ((not chunk_grid.block_is_air_or_unloaded(coords)) &&
					(collision_blacklist.find(coords) == collision_blacklist.end()))
				{
					collisions.push_back(coords);
				}
			}
			if (collisions.empty())
			{
				break;
			}
			BlockCoords const& collision = *std::min(collisions.begin(), collisions.end(),
				[/*this->box*/](
					[[maybe_unused]] auto const& left, [[maybe_unused]] auto const& right
				){
					float const left_dist =
						8; //glm::length(this->box.center - left->to_float_coords());
					float const right_dist =
						8; //glm::length(this->box.center - right->to_float_coords());
					return left_dist < right_dist;
				});
				/* Note:
				 * For SOME REASON taking the closest colliding block makes the game ignore
				 * some walls in some situations.. */

			/* Take into account multiple points in the player, not just its center. */
			/* TODO: Unstupidize or something. */
			#define P(x_, y_, z_) \
				this->coords + (physics.box_dimensions/2.0f) * glm::vec3{x_, y_, z_}
			std::array const possible_points{
				P(-1.0f, -1.0f, -1.0f),
				P(-1.0f, -1.0f, 0.0f),
				P(-1.0f, -1.0f, 1.0f),
				P(-1.0f, 0.0f, -1.0f),
				P(-1.0f, 0.0f, 0.0f),
				P(-1.0f, 0.0f, 1.0f),
				P(-1.0f, 1.0f, -1.0f),
				P(-1.0f, 1.0f, 0.0f),
				P(-1.0f, 1.0f, 1.0f),
				P(0.0f, -1.0f, -1.0f),
				P(0.0f, -1.0f, 0.0f),
				P(0.0f, -1.0f, 1.0f),
				P(0.0f, 0.0f, -1.0f),
				P(0.0f, 0.0f, 0.0f),
				P(0.0f, 0.0f, 1.0f),
				P(0.0f, 1.0f, -1.0f),
				P(0.0f, 1.0f, 0.0f),
				P(0.0f, 1.0f, 1.0f),
				P(1.0f, -1.0f, -1.0f),
				P(1.0f, -1.0f, 0.0f),
				P(1.0f, -1.0f, 1.0f),
				P(1.0f, 0.0f, -1.0f),
				P(1.0f, 0.0f, 0.0f),
				P(1.0f, 0.0f, 1.0f),
				P(1.0f, 1.0f, -1.0f),
				P(1.0f, 1.0f, 0.0f),
				P(1.0f, 1.0f, 1.0f),
				/* Test. */
				P(1.0f, 0.0f, -0.5f),
				P(-1.0f, 0.0f, -0.5f),
				P(0.0f, 1.0f, -0.5f),
				P(0.0f, -1.0f, -0.5f),
				/* More test ><. */
				P(1.0f, 0.0f, +0.5f),
				P(-1.0f, 0.0f, +0.5f),
				P(0.0f, 1.0f, +0.5f),
				P(0.0f, -1.0f, +0.5f)};
			#undef P
			float min_dist = std::numeric_limits<float>::max();
			glm::vec3 points{};
			for (glm::vec3 possible_points : possible_points)
			{
				float dist =
					glm::length(possible_points - static_cast<glm::vec3>(collision));
				if (dist < min_dist)
				{
					min_dist = dist;
					points = possible_points;
				}
			}

			glm::vec3 raw_force = points - static_cast<glm::vec3>(collision);

			/* Prevent untouchable block faces from being able to push in any way. */
			if (raw_force.x > 0)
			{
				BlockCoords neighboor = collision;
				neighboor.x++;
				if (not chunk_grid.block_is_air_or_unloaded(neighboor))
				{
					raw_force.x = 0.0f;
				}
			}
			else
			{
				BlockCoords neighboor = collision;
				neighboor.x--;
				if (not chunk_grid.block_is_air_or_unloaded(neighboor))
				{
					raw_force.x = 0.0f;
				}
			}
			if (raw_force.y > 0)
			{
				BlockCoords neighboor = collision;
				neighboor.y++;
				if (not chunk_grid.block_is_air_or_unloaded(neighboor))
				{
					raw_force.y = 0.0f;
				}
			}
			else
			{
				BlockCoords neighboor = collision;
				neighboor.y--;
				if (not chunk_grid.block_is_air_or_unloaded(neighboor))
				{
					raw_force.y = 0.0f;
				}
			}
			if (raw_force.z > 0)
			{
				BlockCoords neighboor = collision;
				neighboor.z++;
				if (not chunk_grid.block_is_air_or_unloaded(neighboor))
				{
					raw_force.z = 0.0f;
				}
			}
			else
			{
				BlockCoords neighboor = collision;
				neighboor.z--;
				if (not chunk_grid.block_is_air_or_unloaded(neighboor))
				{
					raw_force.z = 0.0f;
				}
			}


			for (unsigned int h = 0; h < 3; h++)
			{
				glm::vec3 raw_force_try = raw_force;

				/* Only keep the biggest axis-aligned component so that block faces push in
				* an axis-aligned way (good) and not in some diagonalish directions (bad). */
				if (std::abs(raw_force_try.x) > std::abs(raw_force_try.y))
				{
					raw_force_try.y = 0.0f;
					if (std::abs(raw_force_try.x) > std::abs(raw_force_try.z))
					{
						raw_force_try.z = 0.0f;
					}
					else
					{
						raw_force_try.x = 0.0f;
					}
				}
				else
				{
					raw_force_try.x = 0.0f;
					if (std::abs(raw_force_try.y) > std::abs(raw_force_try.z))
					{
						raw_force_try.z = 0.0f;
					}
					else
					{
						raw_force_try.y = 0.0f;
					}
				}

				if (raw_force_try == glm::vec3{0.0f, 0.0f, 0.0f})
				{
					/* This collision doesn't seem to be of any use, all of its relevant faces
					 * are probably untouchable.
					 * Hopefully more useful collisions will follow. */
					collision_blacklist.insert(collision);
					goto continue_collisions;
				}
				raw_force_try /= glm::length(raw_force_try); /* Useless normalization xd. */


				glm::vec3 new_center{this->coords};

				/* Push the player out of the colliding block,
				 * also stopping motion towards the colliding block as it bonked. */
				unsigned int axis_index;
				if (raw_force_try.x > 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::X);
					if (motion_step.x < 0.0f)
					{
						motion_step.x = 0.0f;
					}
					new_center.x =
						static_cast<float>(collision.x)
							+ (0.5f + physics.box_dimensions.x / 2.0f + 0.0001f);
				}
				else if (raw_force_try.x < 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::X);
					if (motion_step.x > 0.0f)
					{
						motion_step.x = 0.0f;
					}
					new_center.x =
						static_cast<float>(collision.x)
							- (0.5f + physics.box_dimensions.x / 2.0f + 0.0001f);
				}
				if (raw_force_try.y > 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::Y);
					if (motion_step.y < 0.0f)
					{
						motion_step.y = 0.0f;
					}
					new_center.y =
						static_cast<float>(collision.y)
							+ (0.5f + physics.box_dimensions.y / 2.0f + 0.0001f);
				}
				else if (raw_force_try.y < 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::Y);
					if (motion_step.y > 0.0f)
					{
						motion_step.y = 0.0f;
					}
					new_center.y =
						static_cast<float>(collision.y)
							- (0.5f + physics.box_dimensions.y / 2.0f + 0.0001f);
				}
				if (raw_force_try.z > 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::Z);
					physics.is_falling = false; /* The player is on the ground, stop this->is_falling. */
					if (motion_step.z < 0.0f)
					{
						motion_step.z = 0.0f;
					}
					new_center.z =
						static_cast<float>(collision.z)
							+ (0.5f + physics.box_dimensions.z / 2.0f + 0.0001f);
				}
				else if (raw_force_try.z < 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::Z);
					if (motion_step.z > 0.0f)
					{
						motion_step.z = 0.0f;
					}
					new_center.z =
						static_cast<float>(collision.z)
							- (0.5f + physics.box_dimensions.z / 2.0f + 0.0001f);
				}

				/* Dirty fix to forbid wierd displacements that happen in some cases. */
				if (glm::distance(new_center, this->coords) > 0.3f)
				{
					//std::cout << "Faulty displacement along " << axis_index << std::endl;
					//std::cout << this->box.center[axis_index] << " -> "
					//	<< new_center[axis_index] << std::endl;
					raw_force[axis_index] = 0.0f;
				}
				else
				{
					this->coords = new_center;
					break;
				}
			}

			continue_collisions:;
		}
	}
}

void Entity::draw() {
	if (this->rendering.has_value()) {
		this->rendering.value().draw(this->coords);
	}
}

} /* qwy2 */
