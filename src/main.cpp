
#include "window.hpp"
#include "shaders/shader.hpp"
#include "shaders/blocks/blocks.hpp"
#include "camera.hpp"
#include "chunk.hpp"
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

int main()
{
	using namespace qwy2;

	if (init_window_graphics() == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}

	glEnable(GL_DEPTH_TEST);


	unsigned int atlas_side = 1024;
	std::uint8_t* atlas_data = new std::uint8_t[atlas_side * atlas_side * 4];
	for (unsigned int y = 0; y < atlas_side; y++)
	for (unsigned int x = 0; x < atlas_side; x++)
	{
		std::uint8_t& r = atlas_data[y * atlas_side * 4 + x * 4 + 0];
		std::uint8_t& g = atlas_data[y * atlas_side * 4 + x * 4 + 1];
		std::uint8_t& b = atlas_data[y * atlas_side * 4 + x * 4 + 2];
		std::uint8_t& a = atlas_data[y * atlas_side * 4 + x * 4 + 3];

		r = (std::cos(static_cast<float>(x) * 0.1f) + 1.0f) / 2.0f * 255.0f;
		g = (std::cos(static_cast<float>(x + y) * 0.27f) + 1.0f) / 2.0f * 255.0f;
		b = (std::cos(static_cast<float>(y) * 0.31f) + 1.0f) / 2.0f * 255.0f;
		a = 255;
	}

	AtlasRect rect_a;
	rect_a.atlas_coords_min = glm::vec2(
		static_cast<float>(0) / static_cast<float>(atlas_side),
		static_cast<float>(0) / static_cast<float>(atlas_side));
	rect_a.atlas_coords_max = glm::vec2(
		static_cast<float>(0 + 16) / static_cast<float>(atlas_side),
		static_cast<float>(0 + 16) / static_cast<float>(atlas_side));
	Block::type_table.push_back(BlockType(rect_a, rect_a, rect_a));
	unsigned int type_index_a = Block::type_table.size() - 1;

	AtlasRect rect_b;
	rect_b.atlas_coords_min = glm::vec2(
		static_cast<float>(200) / static_cast<float>(atlas_side),
		static_cast<float>(200) / static_cast<float>(atlas_side));
	rect_b.atlas_coords_max = glm::vec2(
		static_cast<float>(200 + 16) / static_cast<float>(atlas_side),
		static_cast<float>(200 + 16) / static_cast<float>(atlas_side));
	Block::type_table.push_back(BlockType(rect_b, rect_b, rect_b));
	unsigned int type_index_b = Block::type_table.size() - 1;
	
	
	GLint max_atlas_side;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_atlas_side);
	if (static_cast<unsigned int>(max_atlas_side) < atlas_side)
	{
		std::cerr << "GL_MAX_TEXTURE_SIZE is " << max_atlas_side <<
			" which is considered as too small for the atlas" << std::endl;
	}
	GLuint atlas_openglid;
	glGenTextures(1, &atlas_openglid);
	glBindTexture(GL_TEXTURE_2D, atlas_openglid);
	glTexImage2D(GL_TEXTURE_2D,
		0, GL_RGBA, atlas_side, atlas_side, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, atlas_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	unsigned int atlas_opengltextureid = 0;
	glActiveTexture(GL_TEXTURE0 + atlas_opengltextureid);
	glBindTexture(GL_TEXTURE_2D, atlas_openglid);


	UniformValues uniform_values;
	uniform_values.atlas_opengltextureid = atlas_opengltextureid;

	ShaderProgramBlocks shader_program_blocks;
	if (shader_program_blocks.init() == ErrorCode::ERROR)
	{
		std::cerr << "Error occured during shader compilation" << std::endl;
		return EXIT_FAILURE;
	}


	Chunk chunk(RectInt(CoordsInt(-5, -5, -5), CoordsInt(5, 5, 5)));
	{
		Block& block = chunk.block(CoordsInt(-2, 4, 0));
		block.is_air = false;
		block.type_index = type_index_a;
	}
	{
		Block& block = chunk.block(CoordsInt(3, -1, 1));
		block.is_air = false;
		block.type_index = type_index_a;
	}
	{
		Block& block = chunk.block(CoordsInt(1, 3, -1));
		block.is_air = false;
		block.type_index = type_index_a;
	}
	{
		Block& block = chunk.block(CoordsInt(4, -1, 1));
		block.is_air = false;
		block.type_index = type_index_b;
	}
	{
		Block& block = chunk.block(CoordsInt(4, -2, 1));
		block.is_air = false;
		block.type_index = type_index_b;
	}
	{
		Block& block = chunk.block(CoordsInt(4, -2, 2));
		block.is_air = false;
		block.type_index = type_index_b;
	}
	chunk.recompute_mesh();


	Camera camera;

	glm::vec3 player_position{-3.0f, 0.0f, 0.0f};
	float player_horizontal_angle = 0.0f;
	float player_vertical_angle = 0.0f;

	SDL_Event event;

	bool moving_forward = false;
	bool moving_backward = false;
	bool moving_leftward = false;
	bool moving_rightward = false;
	const float moving_factor = 0.05f;

	glm::vec3 player_motion{0.0f, 0.0f, 0.0f};
	bool flying = false;
	bool flying_initial = false;
	const float flying_factor = 0.003f;
	const float flying_initial_value = 0.1f;
	const float falling_factor = 0.012f;

	SDL_SetRelativeMouseMode(SDL_TRUE);
	const float moving_angle_factor = 0.005f;


	using clock = std::chrono::high_resolution_clock;
	const auto clock_time_beginning = clock::now();
	float time = 0.0f; /* Time in seconds. */

	bool running = true;
	while (running)
	{
		time = std::chrono::duration<float>(clock::now() - clock_time_beginning).count();

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
		float forward_motion = moving_factor *
			((moving_forward ? 1.0f : 0.0f) - (moving_backward ? 1.0f : 0.0f));
		float rightward_motion = moving_factor *
			((moving_rightward ? 1.0f : 0.0f) - (moving_leftward ? 1.0f : 0.0f));

		if (flying)
		{
			if (flying_initial)
			{
				player_motion.z = flying_initial_value;
			}
			else
			{
				player_motion.z += flying_factor;
			}
		}
		else if (player_position.z > 0.0001f)
		{
			player_motion.z -= falling_factor;
		}
		else
		{
			player_motion.z = 0.0f;
			player_position.z = 0.0f;
		}
		player_position +=
			player_horizontal_direction * forward_motion +
			player_horizontal_right * rightward_motion +
			player_motion;

		glm::vec3 player_direction = glm::rotate(player_horizontal_direction,
			player_vertical_angle, player_horizontal_right);

		camera.set_position(player_position + glm::vec3(0.0f, 0.0f, 2.0f));
		camera.set_direction(player_direction);
		uniform_values.camera_matrix = camera.matrix;
		shader_program_blocks.update_uniforms(uniform_values);
		
		const float v1 = time / 10.0f;
		const float v2 = (std::cos(v1) + 1.0f) / 2.0f;
		glClearColor(v2 * 0.2f, 0.0f, (1.0f - v2) * 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_program_blocks.draw(chunk.mesh_openglid, chunk.mesh_vertex_count());

		SDL_GL_SwapWindow(g_window);
	}

	cleanup_window_graphics();

	return 0;
}
