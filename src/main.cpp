
#include "window.hpp"
#include "shaders/shader.hpp"
#include "shaders/classic/classic.hpp"
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

int main(int argc, char** argv)
{
	using namespace qwy2;

	bool capture_cursor = true;
	for (unsigned int i = 1; i < static_cast<unsigned int>(argc); i++)
	{
		if (std::strcmp(argv[i], "--no-cursor-capture") == 0)
		{
			capture_cursor = false;
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


	const NoiseGenerator::SeedType seed = 8 + 1;

	Nature nature(seed);
	UniformValues uniform_values;

	nature.world_generator.surface_block_type =
		nature.nature_generator.generate_block_type(nature);
	nature.world_generator.primary_block_type =
		nature.nature_generator.generate_block_type(nature);

	uniform_values.atlas_texture_openglid = nature.atlas.texture_openglid;
	uniform_values.atlas_side = nature.atlas.side;


	glm::vec3 sun_position{100.0f, 500.0f, 1000.0f};
	Camera<OrthographicProjection> sun_camera{
		sun_position,
		-sun_position,
		OrthographicProjection{300.0f, 300.0f},
		1.0f, 2000.0f};
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


	ShaderProgramClassic shader_program_classic;
	if (shader_program_classic.init() == ErrorCode::ERROR)
	{
		std::cerr << "Error occured during shader compilation" << std::endl;
		return EXIT_FAILURE;
	}


	ChunkGrid chunk_grid(13);
	const ChunkRect generated_chunk_rect = ChunkRect(ChunkCoords(0, 0, 0), 7);
	ChunkCoords walker = generated_chunk_rect.walker_start();
	do
	{
		chunk_grid.generate_chunk(nature, walker);
	}
	while (generated_chunk_rect.walker_iterate(walker));


	Camera<PerspectiveProjection> player_camera;

	glm::vec3 player_position{0.0f, 0.0f, 0.0f};
	float player_horizontal_angle = TAU / 2.0f;
	float player_vertical_angle = 0.0f;

	SDL_Event event;

	bool moving_forward = false;
	bool moving_backward = false;
	bool moving_leftward = false;
	bool moving_rightward = false;
	const float moving_factor = 0.1f;
	const float flying_moving_factor = 0.15f;

	glm::vec3 player_motion{0.0f, 0.0f, 0.0f};
	bool flying = false;
	bool flying_initial = false;
	const float flying_factor = 0.003f;
	const float flying_initial_value = 0.1f;
	const float falling_factor = 0.012f;
	const float friction_factor = 0.99f;
	const float floor_friction_factor = 0.95f;

	if (capture_cursor)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	const float moving_angle_factor = 0.005f;


	using clock = std::chrono::high_resolution_clock;
	const auto clock_time_beginning = clock::now();
	float time = 0.0f; /* Time in seconds. */
	
	float previous_time = -FLT_MAX;
	bool one_second_pulse = false;


	bool see_from_sun = false;
	bool see_through_walls = false;


	bool running = true;
	while (running)
	{
		previous_time = time;
		time = std::chrono::duration<float>(clock::now() - clock_time_beginning).count();
		one_second_pulse = std::floor(previous_time) < std::floor(time);
		(void)one_second_pulse;

		float horizontal_angle_motion = 0.0f;
		float vertical_angle_motion = 0.0f;
		flying_initial = false;

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
							for (auto const& [chunk_coords, chunk] : chunk_grid.table)
							{
								chunk->recompute_mesh(nature);
							}
						break;

						case SDLK_l:
							if (event.type == SDL_KEYDOWN)
							{
								SDL_SetRelativeMouseMode(SDL_FALSE);
							}
						break;

						case SDLK_m:
							if (event.type == SDL_KEYDOWN)
							{
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
								player_position.z += 20.0f;
							}
						break;
						case SDLK_j:
							if (event.type == SDL_KEYDOWN)
							{
								player_position.z = 3.0f;
							}
						break;

						case SDLK_n:
							if (event.type == SDL_KEYDOWN)
							{
								chunk_grid.table.clear();
							}
						break;

						case SDLK_h:
							if (event.type == SDL_KEYDOWN)
							{
								see_through_walls = not see_through_walls;
							}
						break;
					}
				break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
						flying = event.type == SDL_MOUSEBUTTONDOWN;
						flying_initial = event.type == SDL_MOUSEBUTTONDOWN;
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
			flying ? flying_moving_factor : moving_factor;
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
			const ChunkRect around_chunk_rect = ChunkRect(player_chunk_coords, 2);
			ChunkCoords walker = around_chunk_rect.walker_start();
			do
			{
				if (chunk_grid.chunk(walker) == nullptr)
				{
					chunk_grid.generate_chunk(nature, walker);
				}
			}
			while (around_chunk_rect.walker_iterate(walker));
			player_chunk = chunk_grid.containing_chunk(player_position);
		}
		else
		{
			const ChunkRect around_chunk_rect = ChunkRect(player_chunk_coords, 5);
			ChunkCoords walker = around_chunk_rect.walker_start();
			do
			{
				if (chunk_grid.chunk(walker) == nullptr)
				{
					chunk_grid.generate_chunk(nature, walker);
					break;
				}
			}
			while (around_chunk_rect.walker_iterate(walker));
		}
		bool is_in_block = not player_chunk->block(player_coords_int).is_air;


		if (flying)
		{
			if (flying_initial)
			{
				player_motion +=
					player_horizontal_direction * forward_motion +
					player_horizontal_right * rightward_motion;
				player_motion.z = flying_initial_value;
			}
			else
			{
				player_motion.z += flying_factor;
			}
			player_motion *= friction_factor;
			player_motion +=
				player_horizontal_direction * forward_motion * flying_factor +
				player_horizontal_right * rightward_motion * flying_factor;
			player_position += player_motion;
		}
		else if (not is_in_block)
		{
			player_motion.z -= falling_factor;
			player_motion *= friction_factor;
			player_motion +=
				player_horizontal_direction * forward_motion * flying_factor +
				player_horizontal_right * rightward_motion * flying_factor;
			player_position += player_motion;
		}
		else
		{
			player_motion.z = 0.0f;
			player_motion *= floor_friction_factor;
			player_position +=
				player_horizontal_direction * forward_motion +
				player_horizontal_right * rightward_motion +
				player_motion;
			player_position.z = std::round(player_position.z) + 0.5f - 0.001f;
		}

		glm::vec3 player_direction = glm::rotate(player_horizontal_direction,
			player_vertical_angle, player_horizontal_right);

		player_camera.set_position(player_position + glm::vec3(0.0f, 0.0f, 2.0f));
		player_camera.set_direction(player_direction);


		sun_position.x = 500.0f * std::cos(time / 8.0f);
		sun_position.y = 500.0f * std::sin(time / 8.0f);
		sun_position.z = 300.0f;
		sun_camera.set_position(sun_position);
		sun_camera.set_target_position(glm::vec3(0.0f, 0.0f, 0.0f));
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
		const auto [window_width, window_height] = window_width_height();
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

		SDL_GL_SwapWindow(g_window);
	}

	cleanup_window_graphics();

	return 0;
}
