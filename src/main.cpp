
#include "window.hpp"
#include "shaders/shader.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include "shaders/line/line_rect.hpp"
#include "shaders/shadow/shadow.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "nature.hpp"
#include "noise.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <unordered_set>

int main(int argc, char** argv)
{
	using namespace qwy2;

	bool capture_cursor = true;
	bool quick_loading = false;
	for (unsigned int i = 1; i < static_cast<unsigned int>(argc); i++)
	{
		if (std::strcmp(argv[i], "--no-cursor-capture") == 0)
		{
			capture_cursor = false;
		}
		else if (std::strcmp(argv[i], "--quick-loading") == 0)
		{
			quick_loading = true;
		}
	}


	if (init_window_graphics() == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);


	NoiseGenerator::SeedType const seed = 8 + 1;

	Nature nature{seed};
	UniformValues uniform_values;

	nature.world_generator.surface_block_type =
		nature.nature_generator.generate_block_type(nature);
	nature.world_generator.primary_block_type =
		nature.nature_generator.generate_block_type(nature);

	uniform_values.atlas_texture_openglid = nature.atlas.texture_openglid;
	uniform_values.atlas_side = nature.atlas.side;


	glm::vec3 sun_position{100.0f, 500.0f, 1000.0f};
	Camera<OrthographicProjection> sun_camera{
		OrthographicProjection{300.0f, 300.0f},
		10.0f, 2000.0f};
	uniform_values.sun_camera_matrix = sun_camera.matrix;

	unsigned int shadow_framebuffer_openglid;
	glGenFramebuffers(1, &shadow_framebuffer_openglid);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_openglid);

	unsigned int shadow_framebuffer_side = 4096;
	GLint max_framebuffer_width, max_framebuffer_height;
	glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &max_framebuffer_width);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &max_framebuffer_height);
	shadow_framebuffer_side = std::min(shadow_framebuffer_side,
		static_cast<unsigned int>(max_framebuffer_width));
	shadow_framebuffer_side = std::min(shadow_framebuffer_side,
		static_cast<unsigned int>(max_framebuffer_height));
	unsigned int shadow_depth_texture_openglid;
	glGenTextures(1, &shadow_depth_texture_openglid);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_texture_openglid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
		shadow_framebuffer_side, shadow_framebuffer_side,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		shadow_depth_texture_openglid, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return EXIT_FAILURE;
		/* TODO: Get a true error message. */
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	uniform_values.shadow_depth_texture_openglid = shadow_depth_texture_openglid;

	ShaderProgramShadow shader_program_shadow;
	if (shader_program_shadow.init() == ErrorCode::ERROR)
	{
		std::cerr << "Error occured during shader compilation" << std::endl;
		return EXIT_FAILURE;
	}


	ShaderProgramLine shader_program_line;
	if (shader_program_line.init() == ErrorCode::ERROR)
	{
		std::cerr << "Error occured during shader compilation" << std::endl;
		return EXIT_FAILURE;
	}
	LineRectDrawer line_rect_drawer;


	ShaderProgramClassic shader_program_classic;
	if (shader_program_classic.init() == ErrorCode::ERROR)
	{
		std::cerr << "Error occured during shader compilation" << std::endl;
		return EXIT_FAILURE;
	}


	ChunkGrid chunk_grid{13};
	unsigned int const loading_radius = quick_loading ? 2 : 7;
	ChunkRect const loading_chunk_rect = ChunkRect{ChunkCoords{0, 0, 0}, loading_radius};
	for (ChunkCoords const& walker : loading_chunk_rect)
	{
		chunk_grid.generate_chunk(nature, walker);
	}


	auto const [width, height] = window_width_height();
	float const aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	Camera<PerspectiveProjection> player_camera{
		PerspectiveProjection{TAU / 8.0f, aspect_ratio},
		0.1f, 300.0f};

	glm::vec3 player_position{0.0f, 0.0f, 0.0f};
	float player_horizontal_angle = TAU / 2.0f;
	float player_vertical_angle = 0.0f;

	AlignedBox player_box{
		player_position + glm::vec3{0.0f, 0.0f, 1.0f},
		glm::vec3{0.8f, 0.8f, 1.8f}};
	AlignedBox player_box_blocks;

	SDL_Event event;

	bool moving_forward = false;
	bool moving_backward = false;
	bool moving_leftward = false;
	bool moving_rightward = false;
	float const floor_moving_factor_fast = 0.13f;
	float const floor_moving_factor_normal = 0.05f;
	float const falling_moving_factor = 0.005f;
	bool fast = false;

	glm::vec3 player_motion{0.0f, 0.0f, 0.0f};
	bool jump_boost = false;
	float const jump_boost_value = 0.2f;
	float const falling_factor = 0.012f;
	float const falling_friction_factor = 0.99f;
	float const floor_friction_factor = 0.4f;
	bool falling = true;
	bool allow_infinite_jumps = false;


	if (capture_cursor)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	float const moving_angle_factor = 0.005f;


	using clock = std::chrono::high_resolution_clock;
	auto const clock_time_beginning = clock::now();
	float time = 0.0f; /* Time in seconds. */
	
	float previous_time = -FLT_MAX;
	[[maybe_unused]] bool one_second_pulse = false;


	unsigned int const chunks_to_load_each_frame = 3;


	bool see_from_sun = false;
	bool see_through_walls = false;
	bool see_boxes = false;
	bool see_from_behind = false;


	bool running = true;
	while (running)
	{
		previous_time = time;
		time = std::chrono::duration<float>(clock::now() - clock_time_beginning).count();
		one_second_pulse = std::floor(previous_time) < std::floor(time);

		float horizontal_angle_motion = 0.0f;
		float vertical_angle_motion = 0.0f;
		jump_boost = false;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
				break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
					if (event.key.repeat != 0)
					{
						break;
					}
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							if (event.type == SDL_KEYDOWN)
							{
								running = false;
							}
						break;

						case SDLK_z:
							moving_forward = event.type == SDL_KEYDOWN;
						break;
						case SDLK_s:
							moving_backward = event.type == SDL_KEYDOWN;
						break;
						case SDLK_q:
							moving_leftward = event.type == SDL_KEYDOWN;
						break;
						case SDLK_d:
							moving_rightward = event.type == SDL_KEYDOWN;
						break;

						case SDLK_p:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[P] Recompute meshes" << std::endl;
								for (auto const& [chunk_coords, chunk] : chunk_grid.table)
								{
									chunk->recompute_mesh(nature);
								}
							}
						break;

						case SDLK_l:
							if (event.type == SDL_KEYDOWN)
							{
								if (SDL_GetRelativeMouseMode() == SDL_TRUE)
								{
									std::cout << "[L] Mouse cursor released" << std::endl;
									SDL_SetRelativeMouseMode(SDL_FALSE);
								}
								else
								{
									std::cout << "[L] Mouse cursor captured" << std::endl;
									SDL_SetRelativeMouseMode(SDL_TRUE);
								}
							}
						break;

						case SDLK_m:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[M] See from sun" << std::endl;
								see_from_sun = not see_from_sun;
							}
						break;

						case SDLK_i:
						case SDLK_k:
							if (event.type == SDL_KEYDOWN)
							{
								sun_position.x +=
									100.0f * (event.key.keysym.sym == SDLK_i ? 1.0f : -1.0f);
							}
						break;

						case SDLK_u:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[U] Player tp upward z+=20" << std::endl;
								player_position.z += 20.0f;
							}
						break;
						case SDLK_j:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[J] Player tp at z=3" << std::endl;
								player_position.z = 3.0f;
							}
						break;

						case SDLK_n:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[N] Unloading "
									<< chunk_grid.table.size() << " chunks" << std::endl;
								chunk_grid.table.clear();
							}
						break;

						case SDLK_h:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[H] See through walls" << std::endl;
								see_through_walls = not see_through_walls;
							}
						break;

						case SDLK_b:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[B] See boxes" << std::endl;
								see_boxes = not see_boxes;
							}
						break;

						case SDLK_v:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[V] See from behind" << std::endl;
								see_from_behind = not see_from_behind;
							}
						break;

						case SDLK_f:
							if (event.type == SDL_KEYDOWN)
							{
								std::cout << "[F] Allow infinite jumps "
									<< "and running fast like sonic" << std::endl;
								allow_infinite_jumps = not allow_infinite_jumps;
								fast = not fast;
								assert(allow_infinite_jumps == fast);
							}
						break;
					}
				break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
						if ((falling && allow_infinite_jumps) || (not falling))
						{
							jump_boost = event.type == SDL_MOUSEBUTTONDOWN;
						}
					}
				break;

				case SDL_MOUSEMOTION:
					horizontal_angle_motion += -event.motion.xrel * moving_angle_factor;
					vertical_angle_motion += -event.motion.yrel * moving_angle_factor;
				break;
			}
		}

		player_horizontal_angle += horizontal_angle_motion;
		player_vertical_angle += vertical_angle_motion;
		if (player_vertical_angle < -TAU / 4.0f + 0.0001f)
		{
			player_vertical_angle = -TAU / 4.0f + 0.0001f;
		}
		else if (player_vertical_angle > TAU / 4.0f - 0.0001f)
		{
			player_vertical_angle = TAU / 4.0f - 0.0001f;
		}

		glm::vec3 player_horizontal_direction{
			std::cos(player_horizontal_angle),
			std::sin(player_horizontal_angle),
			0.0f};
		glm::vec3 player_horizontal_right{
			std::cos(player_horizontal_angle - TAU / 4.0f),
			std::sin(player_horizontal_angle - TAU / 4.0f),
			0.0f};
		float current_moving_factor =
			falling ? falling_moving_factor :
			fast ? floor_moving_factor_fast :
			floor_moving_factor_normal;
		float forward_motion = current_moving_factor *
			((moving_forward ? 1.0f : 0.0f) - (moving_backward ? 1.0f : 0.0f));
		float rightward_motion = current_moving_factor *
			((moving_rightward ? 1.0f : 0.0f) - (moving_leftward ? 1.0f : 0.0f));


		BlockCoords player_coords_int{
			static_cast<int>(std::round(player_position.x)),
			static_cast<int>(std::round(player_position.y)),
			static_cast<int>(std::round(player_position.z))};
		ChunkCoords player_chunk_coords = chunk_grid.containing_chunk_coords(player_coords_int);
		Chunk* player_chunk = chunk_grid.containing_chunk(player_position);
		if (player_chunk == nullptr)
		{
			ChunkRect const around_chunk_rect = ChunkRect{player_chunk_coords, 2};
			for (ChunkCoords const& walker : around_chunk_rect)
			{
				if (chunk_grid.chunk(walker) == nullptr)
				{
					chunk_grid.generate_chunk(nature, walker);
				}
			}
			player_chunk = chunk_grid.containing_chunk(player_position);
		}
		else
		{
			unsigned int chunks_to_load = chunks_to_load_each_frame;
			ChunkRect const around_chunk_rect = ChunkRect{player_chunk_coords, 5};
			for (ChunkCoords const& walker : around_chunk_rect)
			{
				if (chunk_grid.chunk(walker) == nullptr)
				{
					chunk_grid.generate_chunk(nature, walker);
					chunks_to_load--;
					if (chunks_to_load == 0)
					{
						break;
					}
				}
			}
		}


		player_motion *= falling ? falling_friction_factor : floor_friction_factor;
		player_motion.z -= falling_factor;
		player_motion +=
			player_horizontal_direction * forward_motion +
			player_horizontal_right * rightward_motion;
		if (jump_boost)
		{
			player_motion.z = jump_boost_value;
		}

		player_position += player_motion;

		/* Handle collisions with blocks. */
		falling = true;
		player_box.center = player_position + glm::vec3{0.0f, 0.0f, 1.0f};
		BlockRect player_rect = player_box.containing_block_rect();
		BlockCoords player_center_coords_int{
			static_cast<int>(std::round(player_box.center.x)),
			static_cast<int>(std::round(player_box.center.y)),
			static_cast<int>(std::round(player_box.center.z))};
		std::unordered_set<BlockCoords, BlockCoords::Hash> collision_blacklist;
		constexpr unsigned int max_steps_collisions = 30;
		for (unsigned int collision_step = 0;
			collision_step < max_steps_collisions; collision_step++)
		{
			player_rect = player_box.containing_block_rect();

			/* Find the closest colliding block, and forget the others for now.
			 * If that is not enough then we will end up back here anyway. */
			std::vector<BlockCoords> collisions;
			for (BlockCoords coords : player_rect)
			{
				if ((not chunk_grid.block_is_air_or_not_generated(coords)) &&
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
				[player_box](auto const& left, auto const& right){
					float const left_dist =
						8; //glm::length(player_box.center - left->to_float_coords());
					float const right_dist =
						8; //glm::length(player_box.center - right->to_float_coords());
					return left_dist < right_dist;
				});
				/* Note:
				 * For SOME REASON taking the closest colliding block makes the game ignore
				 * some walls in some situations.. */

			/* Take into account multiple points in the player, not just its center. */
			/* TODO: Unstupidize or something. */
			std::array<glm::vec3, 27> const possible_player_points{
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, -1.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, -1.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, -1.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, 0.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, 0.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, 0.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, 1.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, 1.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{-1.0f, 1.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, -1.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, -1.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, -1.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, 0.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, 0.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, 0.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, 1.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, 1.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{0.0f, 1.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, -1.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, -1.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, -1.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, 0.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, 0.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, 0.0f, 1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, 1.0f, -1.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, 1.0f, 0.0f},
				player_box.center + (player_box.dimensions/2.0f) * glm::vec3{1.0f, 1.0f, 1.0f}};
			float min_dist = FLT_MAX;
			glm::vec3 player_point;
			for (glm::vec3 possible_player_point : possible_player_points)
			{
				float dist = glm::length(possible_player_point - collision.to_float_coords());
				if (dist < min_dist)
				{
					min_dist = dist;
					player_point = possible_player_point;
				}
			}

			glm::vec3 raw_force = player_point - collision.to_float_coords();

			/* Prevent untouchable block faces from being able to push in any way. */
			if (raw_force.x > 0)
			{
				BlockCoords neighboor = collision;
				neighboor.x++;
				if (not chunk_grid.block_is_air_or_not_generated(neighboor))
				{
					raw_force.x = 0.0f;
				}
			}
			else
			{
				BlockCoords neighboor = collision;
				neighboor.x--;
				if (not chunk_grid.block_is_air_or_not_generated(neighboor))
				{
					raw_force.x = 0.0f;
				}
			}
			if (raw_force.y > 0)
			{
				BlockCoords neighboor = collision;
				neighboor.y++;
				if (not chunk_grid.block_is_air_or_not_generated(neighboor))
				{
					raw_force.y = 0.0f;
				}
			}
			else
			{
				BlockCoords neighboor = collision;
				neighboor.y--;
				if (not chunk_grid.block_is_air_or_not_generated(neighboor))
				{
					raw_force.y = 0.0f;
				}
			}
			if (raw_force.z > 0)
			{
				BlockCoords neighboor = collision;
				neighboor.z++;
				if (not chunk_grid.block_is_air_or_not_generated(neighboor))
				{
					raw_force.z = 0.0f;
				}
			}
			else
			{
				BlockCoords neighboor = collision;
				neighboor.z--;
				if (not chunk_grid.block_is_air_or_not_generated(neighboor))
				{
					raw_force.z = 0.0f;
				}
			}

			/* Only keep the biggest axis-aligned component so that block faces push in
			 * an axis-aligned way (good) and not in some diagonalish directions (bad). */
			if (std::abs(raw_force.x) > std::abs(raw_force.y))
			{
				raw_force.y = 0.0f;
				if (std::abs(raw_force.x) > std::abs(raw_force.z))
				{
					raw_force.z = 0.0f;
				}
				else
				{
					raw_force.x = 0.0f;
				}
			}
			else
			{
				raw_force.x = 0.0f;
				if (std::abs(raw_force.y) > std::abs(raw_force.z))
				{
					raw_force.z = 0.0f;
				}
				else
				{
					raw_force.y = 0.0f;
				}
			}

			if (raw_force == glm::vec3{0.0f, 0.0f, 0.0f})
			{
				/* This collision doesn't seem to be of any use, all of its relevant faces
				 * are probably untouchable. Hopefully more useful collisions will follow. */
				collision_blacklist.insert(collision);
				continue;
			}
			raw_force /= glm::length(raw_force); /* Useless normalization xd. */

			std::cout << raw_force.x << "," << raw_force.y << "," << raw_force.z << std::endl;

			/* Push the player out of the colliding block,
			 * also stopping motion towards the colliding block as it bonked. */
			if (raw_force.x > 0.0f)
			{
				if (player_motion.x < 0.0f)
				{
					player_motion.x = 0.0f;
				}
				player_box.center.x =
					static_cast<float>(collision.x)
						+ (0.5f + player_box.dimensions.x / 2.0f + 0.0001f);
			}
			else if (raw_force.x < 0.0f)
			{
				if (player_motion.x > 0.0f)
				{
					player_motion.x = 0.0f;
				}
				player_box.center.x =
					static_cast<float>(collision.x)
						- (0.5f + player_box.dimensions.x / 2.0f + 0.0001f);
			}
			if (raw_force.y > 0.0f)
			{
				if (player_motion.y < 0.0f)
				{
					player_motion.y = 0.0f;
				}
				player_box.center.y =
					static_cast<float>(collision.y)
						+ (0.5f + player_box.dimensions.y / 2.0f + 0.0001f);
			}
			else if (raw_force.y < 0.0f)
			{
				if (player_motion.y > 0.0f)
				{
					player_motion.y = 0.0f;
				}
				player_box.center.y =
					static_cast<float>(collision.y)
						- (0.5f + player_box.dimensions.y / 2.0f + 0.0001f);
			}
			if (raw_force.z > 0.0f)
			{
				falling = false; /* The player is on the ground, stop falling. */
				if (player_motion.z < 0.0f)
				{
					player_motion.z = 0.0f;
				}
				player_box.center.z =
					static_cast<float>(collision.z)
						+ (0.5f + player_box.dimensions.z / 2.0f + 0.0001f);
			}
			else if (raw_force.z < 0.0f)
			{
				if (player_motion.z > 0.0f)
				{
					player_motion.z = 0.0f;
				}
				player_box.center.z =
					static_cast<float>(collision.z)
						- (0.5f + player_box.dimensions.z / 2.0f + 0.0001f);
			}
		}

		player_box_blocks.center = glm::vec3{
			static_cast<float>(player_rect.coords_min.x + player_rect.coords_max.x) / 2.0f,
			static_cast<float>(player_rect.coords_min.y + player_rect.coords_max.y) / 2.0f,
			static_cast<float>(player_rect.coords_min.z + player_rect.coords_max.z) / 2.0f};
		player_box_blocks.dimensions = glm::vec3{
			static_cast<float>(player_rect.coords_max.x - player_rect.coords_min.x) + 1.0f,
			static_cast<float>(player_rect.coords_max.y - player_rect.coords_min.y) + 1.0f,
			static_cast<float>(player_rect.coords_max.z - player_rect.coords_min.z) + 1.0f};

		player_position = player_box.center - glm::vec3{0.0f, 0.0f, 1.0f};


		glm::vec3 player_direction = glm::rotate(player_horizontal_direction,
			player_vertical_angle, player_horizontal_right);

		glm::vec3 player_camera_position = player_position + glm::vec3{0.0f, 0.0f, 1.5f};
		player_camera.set_position(player_camera_position);
		player_camera.set_direction(player_direction);
		if (see_from_behind)
		{
			player_camera_position -= 5.0f * player_direction;
			player_camera.set_position(player_camera_position);
		}

		sun_position.x = 500.0f * std::cos(time / 8.0f);
		sun_position.y = 500.0f * std::sin(time / 8.0f);
		sun_position.z = 300.0f;
		sun_camera.set_position(sun_position);
		sun_camera.set_target_position(glm::vec3{0.0f, 0.0f, 0.0f});
		if (see_from_sun)
		{
			uniform_values.user_camera_matrix = sun_camera.matrix;
			uniform_values.user_camera_direction = sun_camera.get_direction();
		}
		else
		{
			uniform_values.user_camera_matrix = player_camera.matrix;
			uniform_values.user_camera_direction = player_camera.get_direction();
		}
		uniform_values.sun_camera_matrix = sun_camera.matrix;
		uniform_values.sun_camera_direction = sun_camera.get_direction();


		/* Render the world from the sun camera to get its depth buffer for shadow rendering.
		 * Face culling is reversed here to make some shadowy artifacts appear in the shadows
		 * (instead of on the bright faces lit by sunlight) where they remain mostly unseen. */
		shader_program_shadow.update_uniforms(uniform_values);
		glViewport(0, 0, shadow_framebuffer_side, shadow_framebuffer_side);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_openglid);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);
		for (auto const& [chunk_coords, chunk] : chunk_grid.table)
		{
			if (not chunk->mesh.vertex_data.empty())
			{
				shader_program_shadow.draw(chunk->mesh);
			}
		}
		glCullFace(GL_FRONT);

		/* Render the world from the player camera. */
		shader_program_classic.update_uniforms(uniform_values);
		auto const [window_width, window_height] = window_width_height();
		glViewport(0, 0, window_width, window_height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.7f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (see_through_walls)
		{
			glCullFace(GL_BACK);
		}
		for (auto const& [chunk_coords, chunk] : chunk_grid.table)
		{
			if (not chunk->mesh.vertex_data.empty())
			{
				shader_program_classic.draw(chunk->mesh);
			}
		}
		glCullFace(GL_FRONT);

		
		if (see_boxes)
		{	
			shader_program_line.update_uniforms(uniform_values);
			glViewport(0, 0, window_width, window_height);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			std::array<std::pair<AlignedBox, glm::vec3>, 2> const box_array{
				std::make_pair(player_box, glm::vec3{0.0f, 0.0f, 0.7f}),
				std::make_pair(player_box_blocks, glm::vec3{0.7f, 0.0f, 0.0f})};
			
			for (auto const& [box, color] : box_array)
			{
				line_rect_drawer.color = color;
				line_rect_drawer.set_box(box);
				
				shader_program_line.draw(line_rect_drawer.mesh);
			}
		}


		SDL_GL_SwapWindow(g_window);
	}

	cleanup_window_graphics();

	return 0;
}
