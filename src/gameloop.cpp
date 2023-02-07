
#include "gameloop.hpp"
#include "config.hpp"
#include "window.hpp"
#include "shaders/table.hpp"
#include "nature.hpp"
#include "terrain_gen.hpp"
#include "noise.hpp"
#include "bitmap.hpp"
#include "embedded.hpp"
#include "opengl.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <optional>
#include <limits>
#include <fstream>
#include <filesystem>

namespace qwy2
{

using namespace std::literals::string_view_literals;

Game::Game()
{
	;
}

void Game::init(Config const& config)
{
	register_builtin_command_names();

	this->load_save_enabled = config.get<bool>("load_save"sv);
	this->save_name = config.get<std::string_view>("save_name"sv);

	this->save_directory += "saves/";
	this->save_directory += this->save_name;
	this->save_directory += "/";
	if (this->load_save_enabled)
	{
		std::filesystem::create_directories(this->save_directory);
		std::cout << "[Init] "
			<< "Save directory is \"" << save_directory << "\"."
			<< std::endl;
	}

	/* There are user-editable command files named `commands.qwy2`, one of which is
	 * to be run at the beginning of execution (now).
	 * There is one such file in the current directory (the common one), and one in
	 * the current save if load/save is enabled.
	 * If the one in the common one does not exist then we create it by copying the
	 * embedded `src/default_commands.qwy2`. If the one in the save does not exist
	 * and load/save is enabled then we create it by copying the one in the common one. */
	{
		/* The command file that will be run. */
		std::string command_file_path;
		if (this->load_save_enabled)
		{
			command_file_path += this->save_directory;
		}
		command_file_path += "commands.qwy2";
		
		/* Making sure that the command file that will be run exists, and if not
		 * then create and fill it with the default content (hardcoded for the . */
		std::ifstream command_file_check{"commands.qwy2"};
		if (not command_file_check.is_open())
		{
			std::ofstream command_file_check{"commands.qwy2"};
			command_file_check.write(g_default_commands, strlen(g_default_commands));
		}
		if (this->load_save_enabled)
		{
			std::ifstream command_file_check{command_file_path};
			if (not command_file_check.is_open())
			{
				std::filesystem::copy_file("commands.qwy2", command_file_path);
			}
		}

		/* Run the commands in the command file that is in the save,
		 * or in the current directory if load/save is disabled. */
		std::ifstream command_file{command_file_path};
		std::stringstream buffer;
		buffer << command_file.rdbuf();
		run_commands(buffer.str());
	}

	/* Initialize graphics. */
	if (init_window_graphics() == ErrorCode::ERROR)
	{
		std::exit(EXIT_FAILURE);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);

	std::cout << "[Init] " << "OpenGL used vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "[Init] " << "OpenGL used renderer: " << glGetString(GL_RENDERER) << std::endl;

	/* Compile shaders. */
	this->shader_table.init();

	/* Generate the laws of nature. */
	this->nature = new Nature{config.get<int>("seed"sv)};
	this->nature->world_generator.plain_terrain_generator =
		plain_terrain_generator_from_name(config.get<std::string_view>("terrain_generator"sv));
	this->nature->world_generator.terrain_param_a = config.get<float>("terrain_param_a"sv);
	this->nature->world_generator.terrain_param_b = config.get<float>("terrain_param_b"sv);
	this->nature->world_generator.terrain_param_c = config.get<float>("terrain_param_c"sv);
	this->nature->world_generator.noise_size = config.get<float>("noise_size"sv);
	this->nature->world_generator.density = config.get<float>("density"sv);
	this->nature->world_generator.structures_enabled = config.get<bool>("structures"sv);
	this->nature->world_generator.stone_terrain = config.get<bool>("stone_terrain"sv);
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
	/* Block type id 4 is a white test block. */
	this->nature->world_generator.test_block_type =
		this->nature->nature_generator.generate_block_type(*this->nature);
	
	/* Emit the texture atlas if requested. */
	if (config.get<bool>("emit_bitmap"sv)) {
		char const* bitmap_file_name = "atlas.bmp";
		emit_bitmap(this->nature->atlas.data,
			this->nature->atlas.side, this->nature->atlas.side, bitmap_file_name);
		std::cout << "[Init] "
			<< "Texture atlas emitted to the file \"" << bitmap_file_name << "\"." << std::endl;
	}

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
	this->shadow_framebuffer_side = config.get<int>("shadow_map_resolution"sv);
	GLint max_framebuffer_width, max_framebuffer_height;
	glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &max_framebuffer_width);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &max_framebuffer_height);
	this->shadow_framebuffer_side = std::min(this->shadow_framebuffer_side,
		static_cast<unsigned int>(max_framebuffer_width));
	this->shadow_framebuffer_side = std::min(this->shadow_framebuffer_side,
		static_cast<unsigned int>(max_framebuffer_height));
	std::cout << "[Init] "
		<< "shadow_framebuffer_side = " << this->shadow_framebuffer_side << std::endl;
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

	/* Initialize the thread pool. */
	unsigned int const loading_threads = config.get<int>("loading_threads"sv);
	this->thread_pool.set_thread_number(loading_threads);
	this->chunk_generation_manager.thread_pool = &this->thread_pool;
	/* Note: The `+ 2` here is arbitrary.
	 * TODO: Make this configurable or find a value that makes sense or both (even better). */
	this->chunk_generation_manager.generating_data_vector.resize(loading_threads + 2);

	/* Initialize the grid of chunks and related fields. */
	this->chunk_side = config.get<int>("chunk_side"sv);
	this->chunk_grid = new ChunkGrid{};
	this->chunk_generation_manager.chunk_grid = this->chunk_grid;
	this->loaded_radius = config.get<float>("loaded_radius"sv);
	this->unloaded_margin = config.get<float>("unloaded_margin"sv);
	this->chunk_generation_manager.generation_radius = this->loaded_radius;
	this->chunk_generation_manager.unloading_margin = this->unloaded_margin;

	this->chunk_generation_manager.load_save_enabled = config.get<bool>("load_save"sv);
	this->chunk_generation_manager.save_only_modified = config.get<bool>("save_only_modified"sv);
	if (this->chunk_generation_manager.load_save_enabled)
	{
		std::string chunk_save_directory{std::string(this->save_directory) + "chunks/"};
		std::filesystem::create_directories(chunk_save_directory);
	}

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

	/* Make the pointer plus-shape mesh. */
	VertexDataLineUi ui_vertex;
	ui_vertex.color = glm::vec3{1.0f, 1.0f, 1.0f};
	float const pointer_size = 0.015f;
	ui_vertex.coords = glm::vec2{0.0f - pointer_size, 0.0f};
	this->pointer_cross_mesh.vertex_data.push_back(ui_vertex);
	ui_vertex.coords = glm::vec2{0.0f + pointer_size, 0.0f};
	this->pointer_cross_mesh.vertex_data.push_back(ui_vertex);
	ui_vertex.coords = glm::vec2{0.0f, 0.0f - aspect_ratio * pointer_size};
	this->pointer_cross_mesh.vertex_data.push_back(ui_vertex);
	ui_vertex.coords = glm::vec2{0.0f, 0.0f + aspect_ratio * pointer_size};
	this->pointer_cross_mesh.vertex_data.push_back(ui_vertex);
	this->pointer_cross_mesh.update_opengl_data();

	/* Handle the mesurment of time. */
	using clock = std::chrono::high_resolution_clock;
	this->clock_time_beginning = clock::now();
	this->time = 0.0f; /* Time in seconds since game startup. */
	this->previous_time = std::numeric_limits<float>::lowest();

	/* Set the defualt value of some settings. */
	this->see_from_sun = false;
	this->see_through_walls = false;
	this->see_boxes = false;
	this->see_chunk_borders = false;
	this->see_from_behind = false;
	this->render_shadows = true;
	this->auto_close = config.get<bool>("close"sv);

	/* Temporary note. */
	std::cout << "[Init] "
		<< "Game loop ready." << std::endl;
	std::cout
		<< "\x1b[33m" /* Yellow. */
		<< "Please wait a bit, rendering the first chunk requires a bit of generation first.\n"
		<< "You can read or modify `commands.qwy2` to learn or customize controls.\n"
		<< "You can read `src/command.cpp` to get a list of commands.\n"
		<< "You can read `src/keycode.cpp` to get the name of keyboard keys."
		<< "\x1b[39m" << std::endl;
}

void Game::loop()
{
	/* GLOP is the (cheap) profiler of Qwy2 and is only enabled when build with `--glop`. */
	#ifdef GLOP_ENABLED
		/* Frame number incremented at each frame. */
		unsigned int frame = 0;
		GlopColumnId glop_frame = this->glop.add_column("Frame");
		/* Some counts of chunk components. */
		GlopColumnId glop_chunk_ptg_count = this->glop.add_column("PTG count");
		GlopColumnId glop_chunk_ptt_count = this->glop.add_column("PTT count");
		GlopColumnId glop_chunk_b_count = this->glop.add_column("B count");
		GlopColumnId glop_chunk_mesh_count = this->glop.add_column("Chunk mesh count");
		/* Timers of different sections of the game loop. */
		GlopColumnId glop_time_all = this->glop.add_column("Timer: All");
		GlopColumnId glop_time_event_handling = this->glop.add_column("Timer: Event handling");
		GlopColumnId glop_time_chunk_manager = this->glop.add_column("Timer: Chunk manager");
		GlopColumnId glop_time_chunk_mesh_sync = this->glop.add_column("Timer: Chunk mesh sync");
		GlopColumnId glop_time_sun_shadows = this->glop.add_column("Timer: Sun shadows");
		GlopColumnId glop_time_chunk_mesh_render = this->glop.add_column("Timer: Chunk mesh render");
		GlopColumnId glop_time_chunk_box_render = this->glop.add_column("Timer: Chunk box render");
		GlopColumnId glop_time_sdl_swap_window = this->glop.add_column("Timer: SDL_GL_SwapWindow");
		this->glop.open_output_stream();
		#define TIME_BLOCK(column_id_) GlopTimer timer_{this->glop, column_id_}
	#else
		#define TIME_BLOCK(column_id_) do {} while (false)
	#endif

	/* The main game loop is here! */
	this->loop_running = true;
	while (this->loop_running)
	{
		/* Mesure time at the beginnig of the current iteration. */
		using clock = std::chrono::high_resolution_clock;
		auto const clock_time_before_iteration = clock::now();
		this->previous_time = this->time;
		this->time = std::chrono::duration<float>(
			clock::now() - this->clock_time_beginning).count();
		auto const delta_time = this->time - this->previous_time;

		/* Handle input events. */
		{
			TIME_BLOCK(glop_time_event_handling);
			this->input_event_handler.handle_events();
		}

		/* Apply controls, motion and collisions to the player. */
		this->player.apply_motion(*this->chunk_grid, this->player_controls, delta_time);
		this->chunk_generation_manager.generation_center = this->player.box.center;

		/* Generate chunks around the player. */
		{
			TIME_BLOCK(glop_time_chunk_manager);
			this->chunk_generation_manager.manage(*this->nature);
		}

		/* Handle the player's camera. */
		glm::vec3 player_camera_position = this->player.camera_position();
		glm::vec3 const player_direction = this->player.direction();
		this->shader_table.update_uniform(Uniform::USER_COORDS, player_camera_position);
		this->player_camera.set_position(player_camera_position);
		this->player_camera.set_direction(player_direction);
		if (this->see_from_behind)
		{
			player_camera_position -= 5.0f * player_direction;
			this->player_camera.set_position(player_camera_position);
		}

		/* Handle the face pointed by the player. */
		this->pointed_face_opt = this->player.pointed_face(*this->chunk_grid);

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
		{
			TIME_BLOCK(glop_time_chunk_mesh_sync);
			for (auto& [chunk_coords, mesh] : this->chunk_grid->mesh)
			{
				if (mesh.needs_update_opengl_data)
				{
					mesh.update_opengl_data();
				}
			}
		}

		/* Render the world from the sun camera to get its depth buffer for shadow rendering.
		 * Face culling is reversed here to make some shadowy artifacts appear in the shadows
		 * (instead of on the bright faces lit by sunlight) where they remain mostly unseen. */
		glEnable(GL_DEPTH_TEST);
		if (this->render_shadows)
		{
			TIME_BLOCK(glop_time_sun_shadows);

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
		
		/* Render the world from the player camera. */
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
		{
			TIME_BLOCK(glop_time_chunk_mesh_render);
			for (auto const& [chunk_coords, mesh] : this->chunk_grid->mesh)
			{
				if (mesh.openglid != 0)
				{
					this->shader_table.classic().draw(mesh);
				}
			}
		}
		glCullFace(GL_FRONT);

		/* Render the pointed face. */
		if (this->pointed_face_opt.has_value())
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

			AlignedBox const box{this->pointed_face_opt->internal_coords,
				glm::vec3{1.0f, 1.0f, 1.0f} * 1.001f};
			this->line_rect_drawer.color = glm::vec3{1.0f, 1.0f, 1.0f};
			this->line_rect_drawer.set_box(box);
			this->shader_table.line().draw(this->line_rect_drawer.mesh);
		}

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

		/* Render chunk borders if enabled. */
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

			{
				TIME_BLOCK(glop_time_chunk_box_render);

				this->line_rect_drawer.color = glm::vec3{0.0f, 0.4f, 0.8f};
				for (auto const& [chunk_coords, chunk_ptg_field] : this->chunk_grid->ptg_field)
				{
					BlockRect const rect = chunk_block_rect(chunk_coords);
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
		}

		/* Render the pointer cross. */
		glDisable(GL_DEPTH_TEST);
		if (not this->see_from_sun)
		{
			glViewport(0, 0, window_width, window_height);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			this->shader_table.line_ui().draw(this->pointer_cross_mesh);
		}

		/* Apply what was drawn to what is displayed on the screen. */
		{
			TIME_BLOCK(glop_time_sdl_swap_window);
			SDL_GL_SwapWindow(g_window);
		}

		/* Mesure time at the end of the current iteration. */
		auto const clock_time_after_iteration = clock::now();
		[[maybe_unused]] float const duration_iteration =
			std::chrono::duration<float>(
				clock_time_after_iteration - clock_time_before_iteration).count();

		#ifdef GLOP_ENABLED
			this->glop.set_column_value(glop_time_all,
				duration_iteration);
			this->glop.set_column_value(glop_frame,
				frame++);
			this->glop.set_column_value(glop_chunk_ptg_count,
				this->chunk_grid->ptg_field.size());
			this->glop.set_column_value(glop_chunk_ptt_count,
				this->chunk_grid->ptt_field.size());
			this->glop.set_column_value(glop_chunk_b_count,
				this->chunk_grid->b_field.size());
			this->glop.set_column_value(glop_chunk_mesh_count,
				this->chunk_grid->mesh.size());
			this->glop.emit_row();
		#endif

		#if 0
		std::cout
			//<< "Chunk mesh count: " << this->chunk_grid->mesh.size() << "  \t"
			//<< "Chunk PTG field count: " << this->chunk_grid->ptg_field.size() << "  \t"
			<< "FPS: " << (1.0f / duration_iteration) << std::endl;
		#endif

		if (this->auto_close)
		{
			std::cout << "[Gameloop] "
				<< "Closing as requested by the configuration." << std::endl;
			break;
		}
	}

	#ifdef GLOP_ENABLED
		this->glop.close_output_stream();
	#endif

	/* TODO: Make this better. */
	if (this->chunk_generation_manager.load_save_enabled)
	{
		std::string chunk_save_directory{std::string(this->save_directory) + "chunks/"};
		std::cout << "[Cleanup] "
			<< "Saved chunk B fields to \"" << chunk_save_directory << "\"." << std::endl;
		this->chunk_grid->save_all_that_is_necessary();
	}

	cleanup_window_graphics();
}

Game* g_game = nullptr;

} /* qwy2 */
