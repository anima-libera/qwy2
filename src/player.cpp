
#include "player.hpp"
#include "chunk.hpp"
#include "input_events.hpp"
#include "utils.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
#include <unordered_set>
#include <limits>

#include <iostream>

namespace qwy2
{

Player::Player():
	box{glm::vec3{0.0f, 0.0f, 8.0f}, glm::vec3{0.8f, 0.8f, 1.8f}},
	horizontal_angle{TAU / 2.0f},
	vertical_angle{0.0f},
	is_falling{true},
	motion{0.0f, 0.0f, 0.0f},
	moving_angle_factor{0.005f},
	allowed_fast_and_infinite_jumps{false}
{
	;
}

void Player::apply_motion(ChunkGrid const& chunk_grid, PlayerControls const& controls)
{
	constexpr float floor_moving_factor_fast = 0.13f;
	constexpr float floor_moving_factor_normal = 0.05f;
	constexpr float falling_moving_factor = 0.005f;
	constexpr float jump_boost_value = 0.2f;
	constexpr float falling_factor = 0.012f;
	constexpr float falling_friction_factor = 0.99f;
	constexpr float floor_friction_factor = 0.4f;

	this->horizontal_angle += controls.horizontal_angle_motion;
	this->vertical_angle += controls.vertical_angle_motion;
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


	this->motion *= this->is_falling ? falling_friction_factor : floor_friction_factor;
	this->motion.z -= falling_factor;
	this->motion += walking_motion;
	if (controls.will_jump)
	{
		this->motion.z = jump_boost_value;
	}

	glm::vec3 motion_remaining = this->motion;
	if (not chunk_grid.has_complete_mesh(containing_chunk_coords(this->box.center)))
	{
		/* Prevents falling through unloaded floor. */
		motion_remaining.z = 0.0f;
	}
	float step_max_length = 0.05f;
	glm::vec3 motion_nonfinal_step = glm::normalize(this->motion) * step_max_length;
	BlockRect rect = this->box.containing_block_rect();
	while (glm::dot(motion_remaining, motion_nonfinal_step) > 0.0f)
	{
		glm::vec3 motion_step =
			glm::length(motion_remaining) > glm::length(motion_nonfinal_step) ?
			motion_nonfinal_step : motion_remaining;
		motion_remaining -= motion_nonfinal_step;

		/* Handle collisions with blocks.
		 * TODO: Remake this from scratch and with some elegant ideas instead of patching
		 * crappy stuff until it somehow works. */
		this->is_falling = true;
		this->box.center += motion_step;
		rect = this->box.containing_block_rect();
		BlockCoords center_coords_int{
			static_cast<int>(std::round(this->box.center.x)),
			static_cast<int>(std::round(this->box.center.y)),
			static_cast<int>(std::round(this->box.center.z))};
		std::unordered_set<BlockCoords, BlockCoords::Hash> collision_blacklist;
		constexpr unsigned int max_steps_collisions = 30;
		for (unsigned int collision_step = 0;
			collision_step < max_steps_collisions; collision_step++)
		{
			rect = this->box.containing_block_rect();

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
				this->box.center + (this->box.dimensions/2.0f) * glm::vec3{x_, y_, z_}
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


				glm::vec3 new_center{this->box.center};

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
							+ (0.5f + this->box.dimensions.x / 2.0f + 0.0001f);
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
							- (0.5f + this->box.dimensions.x / 2.0f + 0.0001f);
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
							+ (0.5f + this->box.dimensions.y / 2.0f + 0.0001f);
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
							- (0.5f + this->box.dimensions.y / 2.0f + 0.0001f);
				}
				if (raw_force_try.z > 0.0f)
				{
					axis_index = static_cast<unsigned int>(Axis::Z);
					this->is_falling = false; /* The player is on the ground, stop this->is_falling. */
					if (motion_step.z < 0.0f)
					{
						motion_step.z = 0.0f;
					}
					new_center.z =
						static_cast<float>(collision.z)
							+ (0.5f + this->box.dimensions.z / 2.0f + 0.0001f);
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
							- (0.5f + this->box.dimensions.z / 2.0f + 0.0001f);
				}

				/* Dirty fix to forbid wierd displacements that happen in some cases. */
				if (glm::distance(new_center, this->box.center) > 0.3f)
				{
					//std::cout << "Faulty displacement along " << axis_index << std::endl;
					//std::cout << this->box.center[axis_index] << " -> "
					//	<< new_center[axis_index] << std::endl;
					raw_force[axis_index] = 0.0f;
				}
				else
				{
					this->box.center = new_center;
					break;
				}
			}

			continue_collisions:;
		}
	}
}

std::optional<BlockFace> Player::pointed_face(ChunkGrid const& chunk_grid) const
{
	glm::vec3 const start_point_coords = this->camera_position();
	glm::vec3 point_coords = start_point_coords;
	BlockCoords previous_block = point_coords;
	BlockCoords current_block = point_coords;
	glm::vec3 const direction = glm::normalize(this->direction());
	glm::vec3 const step_max = direction * 4.0f;

	while (true)
	{
		/* We basically do a binary search on the distance to move to get to the next block
		 * without skipping any (which means that we should get to a common face neighbor). */
		previous_block = point_coords;
		glm::vec3 const max_next_point_coords = point_coords + step_max;
		glm::vec3 next_point_coords = max_next_point_coords;
		float step_ratio_min = 0.0f;
		float step_ratio_max = 1.0f;
		float step_ratio = (step_ratio_min + step_ratio_max) / 2.0f;
		current_block = next_point_coords;
		while (not previous_block.is_common_face_neighbor(current_block))
		{
			float const previous_step_ratio = step_ratio;
			if (previous_block == current_block)
			{
				step_ratio_min = step_ratio;
			}
			else
			{
				step_ratio_max = step_ratio;
			}
			step_ratio = (step_ratio_min + step_ratio_max) / 2.0f;
			if (step_ratio == previous_step_ratio)
			{
				next_point_coords = max_next_point_coords;
				current_block = next_point_coords;
				break;
			}

			next_point_coords =
				point_coords * (1.0f - step_ratio) + max_next_point_coords * step_ratio;
			current_block = next_point_coords;
		}

		if (not chunk_grid.block_is_air_or_unloaded(current_block))
		{
			return BlockFace{current_block, previous_block};
		}
		else if (glm::distance(start_point_coords, next_point_coords) > 5.0f)
		{
			return std::nullopt;
		}
		point_coords = next_point_coords;
	}
}

glm::vec3 Player::direction() const
{
	glm::vec3 horizontal_direction{
		std::cos(this->horizontal_angle),
		std::sin(this->horizontal_angle),
		0.0f};
	glm::vec3 horizontal_right_direction{
		std::cos(this->horizontal_angle - TAU / 4.0f),
		std::sin(this->horizontal_angle - TAU / 4.0f),
		0.0f};
	return glm::rotate(horizontal_direction,
		this->vertical_angle, horizontal_right_direction);
}

glm::vec3 Player::camera_position() const
{
	return this->box.center + glm::vec3{0.0f, 0.0f, 0.6f};
}

} /* qwy2 */
