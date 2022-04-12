
#include "gameloop.hpp"
#include "window.hpp"
#include "shaders/shader.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include "shaders/line/line_rect.hpp"
#include "shaders/shadow/shadow.hpp"
#include "shaders/table.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "coords.hpp"
#include "nature.hpp"
#include "noise.hpp"
#include "bitmap.hpp"
#include "player.hpp"
#include "input_events.hpp"
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
#include <sstream>
#include <thread>
#include <future>
#include <optional>
#include <limits>

namespace qwy2
{

using namespace std::literals::string_view_literals;

Game::Game(Config const& config)
{
	/* Initialize graphics. */
	if (init_window_graphics() == ErrorCode::ERROR)
	{
		std::exit(EXIT_FAILURE);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);

	/* Compile shaders. */
	this->shader_table.init();

	/* Generate the laws of nature. */
	this->nature = new Nature{config.get<int>("seed"sv)};
	/* Block type id 0 is air. */
	this->nature->nature_generator.generate_block_type(*this->nature);
	/* Block type id 1 is dirt covered with grass. */
	this->nature->world_generator.surface_block_type =
		this->nature->nature_generator.generate_block_type(*this->nature);
	/* Block type id 2 is plain dirt. */
	this->nature->world_generator.primary_block_type =
		this->nature->nature_generator.generate_block_type(*this->nature);
	/* Block type id 3 is plain rock. */
	this->nature->world_generator.secondary_block_type =
		this->nature->nature_generator.generate_block_type(*this->nature);
	#if 0
	emit_bitmap(this->nature->atlas.data,
		this->nature->atlas.side, this->nature->atlas.side, "atlas.bmp");
	#endif

	/* The texture image unit of id 0 is used as the default active unit so that binding
	 * textures around does not disturb the texture bound in non-zero units that are
	 * given to shaders as uniforms. */
	unsigned int next_texture_image_unit_openglid = 1;

	/* Give a texture image unit id to the atlas's texture. */
	unsigned int const atlas_texture_image_unit_openglid = next_texture_image_unit_openglid++;
	glActiveTexture(GL_TEXTURE0 + atlas_texture_image_unit_openglid);
	glBindTexture(GL_TEXTURE_2D, nature->atlas.texture_openglid);
	this->shader_table.update_uniform(Uniform::ATLAS_TEXTURE_IMAGE_UNIT_OPENGLID,
		atlas_texture_image_unit_openglid);
	this->shader_table.update_uniform(Uniform::ATLAS_SIDE, static_cast<float>(nature->atlas.side));
	glActiveTexture(GL_TEXTURE0 + 0);

	/* Define the sun's camera that will cast shadows on the world. */
	//this->sun_position = glm::vec3{100.0f, 500.0f, 1000.0f};
	this->sun_camera = Camera<OrthographicProjection>{
		OrthographicProjection{300.0f, 300.0f},
		10.0f, 2000.0f};

	/* Create the framebuffer and associated texture that will receive the depth component
	 * of the world's rendering from the sun's point of view, which will then be used to
	 * cast shadows on the user's rendering of the world. */
	glGenFramebuffers(1, &this->shadow_framebuffer_openglid);
	glBindFramebuffer(GL_FRAMEBUFFER, this->shadow_framebuffer_openglid);
	this->shadow_framebuffer_side = 4096 * 8; /* TODO: Make this configurable. */
	GLint max_framebuffer_width, max_framebuffer_height;
	glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &max_framebuffer_width);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &max_framebuffer_height);
	this->shadow_framebuffer_side = std::min(this->shadow_framebuffer_side,
		static_cast<unsigned int>(max_framebuffer_width));
	this->shadow_framebuffer_side = std::min(this->shadow_framebuffer_side,
		static_cast<unsigned int>(max_framebuffer_height));
	std::cout << "shadow_framebuffer_side = " << this->shadow_framebuffer_side << std::endl;
	unsigned int shadow_depth_texture_openglid;
	glGenTextures(1, &shadow_depth_texture_openglid);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_texture_openglid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
		this->shadow_framebuffer_side, this->shadow_framebuffer_side,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		shadow_depth_texture_openglid, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::exit(EXIT_FAILURE);
		/* TODO: Get a true error message. */
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* Give a texture image unit id to the shadow depth texture. */
	unsigned int const shadow_depth_texture_image_unit_openglid =
		next_texture_image_unit_openglid++;
	glActiveTexture(GL_TEXTURE0 + shadow_depth_texture_image_unit_openglid);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_texture_openglid);
	this->shader_table.update_uniform(Uniform::SHADOW_DEPTH_TEXTURE_IMAGE_UNIT_OPENGLID,
		shadow_depth_texture_image_unit_openglid);
	glActiveTexture(GL_TEXTURE0 + 0);

	/* Initialize the grid of chunks and related fields. */
	g_chunk_side = config.get<int>("chunk_side"sv);
	std::cout << "Chunk side: " << g_chunk_side << std::endl;
	this->chunk_grid = new ChunkGrid{};
	this->chunk_generation_manager.chunk_grid = this->chunk_grid;
	this->keep_generating_chunks = true;
	this->loaded_radius = config.get<float>("loaded_radius"sv);
	this->chunk_generation_manager.generation_radius = this->loaded_radius;

	/* Define the sky color and make the fog correspond to it. */
	this->sky_color = glm::vec3{0.0f, 0.7f, 0.9f};
	this->shader_table.update_uniform(Uniform::FOG_COLOR, this->sky_color);
	this->shader_table.update_uniform(Uniform::FOG_DISTANCE_INF, this->loaded_radius * 0.5f);
	this->shader_table.update_uniform(Uniform::FOG_DISTANCE_SUP, this->loaded_radius * 0.96f);

	/* Define the player's camera. */
	auto const [width, height] = window_width_height();
	float const aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	this->player_camera = Camera<PerspectiveProjection>{
		PerspectiveProjection{TAU / 8.0f, aspect_ratio},
		0.1f, this->loaded_radius * 2.5f};

	/* Handle the cursor's capture and sensibility. */
	this->cursor_is_captured = config.get<bool>("cursor_capture"sv);
	if (this->cursor_is_captured)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	assert(this->player.moving_angle_factor == 0.005f);

	/* Handle the mesurment of time. */
	using clock = std::chrono::high_resolution_clock;
	this->clock_time_beginning = clock::now();
	this->time = 0.0f; /* Time in seconds since game startup. */
	this->previous_time = std::numeric_limits<float>::lowest();

	/* Set the defualt value of some settings. */
	this->see_from_sun = false;
	this->see_through_walls = false;
	this->see_boxes = false;
	this->see_chunk_borders = true;
	this->see_from_behind = false;
	this->render_shadows = true;
	this->loop_running = true;

	/* Temporary note. */
	std::cout << "Game loop ready.\n"
		<< "Please wait a bit, rendering the first chunk requires a bit of generation first.\n"
		<< "Press [G] to stop displaying the chunk boarders.\n"
		<< "Press [M] to toggle the view from the sun's point of view.\n"
		<< "Use [ZQSD] to walk around and [right-click] to jump.\n"
		<< "Other controls are available, try mashing the keyboard." << std::endl;
}

void Game::loop()
{
	this->loop_running = true;
	while (this->loop_running)
	{
		/* Mesure time at the beginnig of the current iteration. */
		using clock = std::chrono::high_resolution_clock;
		auto const clock_time_before_iteration = clock::now();
		this->previous_time = this->time;
		this->time = std::chrono::duration<float>(
			clock::now() - this->clock_time_beginning).count();

		/* Handle input events. */
		this->input_event_handler.handle_events(*this);

		/* Apply controls, motion and collisions to the player. */
		this->player.apply_motion(*this->chunk_grid, this->player_controls);
		this->chunk_generation_manager.generation_center = this->player.box.center;

		/* Generate chunks around the player. */
		this->chunk_generation_manager.manage(*this->nature);

		/* Handle the player's camera. */
		glm::vec3 player_camera_position =
			this->player.box.center + glm::vec3{0.0f, 0.0f, 0.6f};
		glm::vec3 const player_direction = this->player.direction();
		this->shader_table.update_uniform(Uniform::USER_COORDS, player_camera_position);
		this->player_camera.set_position(player_camera_position);
		this->player_camera.set_direction(player_direction);
		if (this->see_from_behind)
		{
			player_camera_position -= 5.0f * player_direction;
			this->player_camera.set_position(player_camera_position);
		}

		/* Handle the sun's camera. */
		this->sun_position.x = 500.0f * std::cos(this->time / 40.0f);
		this->sun_position.y = 500.0f * std::sin(this->time / 40.0f);
		this->sun_position.z = 300.0f;
		this->sun_position += this->player.box.center;
		this->sun_camera.set_position(this->sun_position);
		this->sun_camera.set_target_position(this->player.box.center);
		if (this->see_from_sun)
		{
			this->shader_table.update_uniform(Uniform::USER_CAMERA_MATRIX,
				this->sun_camera.matrix);
			this->shader_table.update_uniform(Uniform::USER_CAMERA_DIRECTION,
				this->sun_camera.get_direction());
		}
		else
		{
			this->shader_table.update_uniform(Uniform::USER_CAMERA_MATRIX,
				this->player_camera.matrix);
			this->shader_table.update_uniform(Uniform::USER_CAMERA_DIRECTION,
				this->player_camera.get_direction());
		}
		this->shader_table.update_uniform(Uniform::SUN_CAMERA_MATRIX,
			this->sun_camera.matrix);
		this->shader_table.update_uniform(Uniform::SUN_CAMERA_DIRECTION,
			this->sun_camera.get_direction());

		/* Make sure that all chunk's mesh data are synchronized on the GPU's side. */
		for (auto& [chunk_coords, mesh] : this->chunk_grid->mesh)
		{
			if (mesh.needs_update_opengl_data)
			{
				mesh.update_opengl_data();
			}
		}

		/* Render the world from the sun camera to get its depth buffer for shadow rendering.
		 * Face culling is reversed here to make some shadowy artifacts appear in the shadows
		 * (instead of on the bright faces lit by sunlight) where they remain mostly unseen. */
		auto const clock_time_before_sun_shadows = clock::now();
		if (this->render_shadows)
		{
			glViewport(0, 0, this->shadow_framebuffer_side, this->shadow_framebuffer_side);
			glBindFramebuffer(GL_FRAMEBUFFER, this->shadow_framebuffer_openglid);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_BACK);
			for (auto const& [chunk_coords, mesh] : this->chunk_grid->mesh)
			{
				if (mesh.openglid != 0)
				{
					this->shader_table.shadow().draw(mesh);
				}
			}
		}
		glCullFace(GL_FRONT);
		auto const clock_time_after_sun_shadows = clock::now();
		[[maybe_unused]] auto const duration_sun_shadows =
			(clock_time_after_sun_shadows - clock_time_before_sun_shadows).count();

		/* Render the world from the player camera. */
		auto const clock_time_before_user_rendering = clock::now();
		auto const [window_width, window_height] = window_width_height();
		if (this->see_from_sun)
		{
			glViewport(0, 0, window_height, window_height);
		}
		else
		{
			glViewport(0, 0, window_width, window_height);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(this->sky_color.x, this->sky_color.y, this->sky_color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (this->see_through_walls)
		{
			glCullFace(GL_BACK);
		}
		for (auto const& [chunk_coords, mesh] : this->chunk_grid->mesh)
		{
			if (mesh.openglid != 0)
			{
				this->shader_table.classic().draw(mesh);
			}
		}
		glCullFace(GL_FRONT);
		auto const clock_time_after_user_rendering = clock::now();
		[[maybe_unused]] auto const duration_user_rendering =
			(clock_time_after_user_rendering - clock_time_before_user_rendering).count();

		/* Render boxes (currently only the player hitbox) is enabled. */
		if (this->see_boxes)
		{	
			if (this->see_from_sun)
			{
				glViewport(0, 0, window_height, window_height);
			}
			else
			{
				glViewport(0, 0, window_width, window_height);
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			std::array<std::pair<AlignedBox, glm::vec3>, 2> const box_array{
				std::make_pair(this->player.box, glm::vec3{0.0f, 0.0f, 0.7f})};
			for (auto const& [box, color] : box_array)
			{
				this->line_rect_drawer.color = color;
				this->line_rect_drawer.set_box(box);
				this->shader_table.line().draw(this->line_rect_drawer.mesh);
			}
		}

		/* Render chunk boarders if enabled. */
		if (this->see_chunk_borders)
		{
			if (this->see_from_sun)
			{
				glViewport(0, 0, window_height, window_height);
			}
			else
			{
				glViewport(0, 0, window_width, window_height);
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			this->line_rect_drawer.color = glm::vec3{0.0f, 0.4f, 0.8f};
			for (auto const& [chunk_coords, chunk_ptg_field] : this->chunk_grid->ptg_field)
			{
				BlockRect const rect = chunk_rect(chunk_coords);
				glm::vec3 const coords_min =
					static_cast<glm::vec3>(rect.coords_min) - glm::vec3{0.5f, 0.5f, 0.5f};
				glm::vec3 const coords_max =
					static_cast<glm::vec3>(rect.coords_max) + glm::vec3{0.5f, 0.5f, 0.5f};
				this->line_rect_drawer.set_box(AlignedBox{
					(coords_min + coords_max) / 2.0f, coords_max - coords_min});
				this->shader_table.line().draw(this->line_rect_drawer.mesh);
			}

			#if 0
			this->line_rect_drawer.color = glm::vec3{1.0f, 0.0f, 0.0f};
			for (std::optional<GeneratingChunkWrapper> const& wrapper_opt :
				this->generating_chunk_table)
			{
				if (wrapper_opt.has_value())
				{
					BlockRect const chunk_rect = chunk_grid->chunk_rect(
						wrapper_opt.value().chunk_coords);
					glm::vec3 const coords_min =
						static_cast<glm::vec3>(chunk_rect.coords_min)
							- glm::vec3{0.5f, 0.5f, 0.5f};
					glm::vec3 const coords_max =
						static_cast<glm::vec3>(chunk_rect.coords_max)
							+ glm::vec3{0.5f, 0.5f, 0.5f};
					this->line_rect_drawer.set_box(AlignedBox{
						(coords_min + coords_max) / 2.0f, coords_max - coords_min});
					this->shader_table.line().draw(this->line_rect_drawer.mesh);
				}
			}
			#endif
		}

		/* Apply what was drawn to what is displayed on the screen. */
		auto const clock_time_before_swapping_buffers = clock::now();
		SDL_GL_SwapWindow(g_window);
		auto const clock_time_after_swapping_buffers = clock::now();
		[[maybe_unused]] auto const duration_swapping_buffers =
			(clock_time_after_swapping_buffers - clock_time_before_swapping_buffers).count();

		/* Mesure time at the end of the current iteration. */
		auto const clock_time_after_iteration = clock::now();
		[[maybe_unused]] float const duration_iteration =
			std::chrono::duration<float>(
				clock_time_after_iteration - clock_time_before_iteration).count();

		#if 0
		std::cout
			<< "Chunk count: " << this->chunk_grid->table.size() << "\t"
			<< "FPS: " << (1.0f / duration_iteration) << std::endl;
		#endif
	}

	cleanup_window_graphics();
}

} /* qwy2 */
