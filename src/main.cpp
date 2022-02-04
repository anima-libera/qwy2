
#include "window.hpp"
#include "shaders/shader.hpp"
#include "shaders/blocks/blocks.hpp"
#include "camera.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cmath>

int main()
{
	using namespace qwy2;

	if (init_window_graphics() == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}


	Camera camera;

	UniformValues uniform_values;
	uniform_values.camera_matrix = camera.matrix;

	ShaderProgramBlocks shader_program_blocks;
	if (shader_program_blocks.init() == ErrorCode::ERROR)
	{
		std::cerr << "ono" << std::endl;
		return EXIT_FAILURE;
	}
	shader_program_blocks.update_uniforms(uniform_values);


	float triangles[] = {
		0.0f, 0.0f, 2.0f, 1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	};

	GLuint triangle_buffer_openglid;
	glGenBuffers(1, &triangle_buffer_openglid);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_openglid);
	glBufferData(GL_ARRAY_BUFFER, sizeof triangles, triangles, GL_DYNAMIC_DRAW);


	using clock = std::chrono::high_resolution_clock;
	const auto clock_time_beginning = clock::now();
	float time = 0.0f; /* Time in seconds. */

	SDL_Event event;
	bool moving_forward = false;
	bool moving_backward = false;
	bool moving_leftward = false;
	bool moving_rightward = false;
	const float moving_factor = 0.05f;

	SDL_SetRelativeMouseMode(SDL_TRUE);
	const float moving_angle_factor = 0.005f;

	glm::vec3 player_position{-3.0f, 0.0f, 0.0f};
	float player_horizontal_angle = 0.0f;
	float player_vertical_angle = 0.0f;

	bool running = true;
	while (running)
	{
		time = std::chrono::duration<float>(clock::now() - clock_time_beginning).count();

		float horizontal_angle_motion = 0.0f;
		float vertical_angle_motion = 0.0f;

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

				case SDL_MOUSEMOTION:
					horizontal_angle_motion += -event.motion.xrel * moving_angle_factor;
					vertical_angle_motion += -event.motion.yrel * moving_angle_factor;
				break;
			}
		}

		player_horizontal_angle += horizontal_angle_motion;
		player_vertical_angle += vertical_angle_motion;
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
		player_position +=
			player_horizontal_direction * forward_motion +
			player_horizontal_right * rightward_motion;

		glm::vec3 player_direction = glm::rotate(player_horizontal_direction,
			player_vertical_angle, player_horizontal_right);

		camera.set_position(player_position + glm::vec3(0.0f, 0.0f, 2.0f));
		camera.set_direction(player_direction);
		uniform_values.camera_matrix = camera.matrix;
		shader_program_blocks.update_uniforms(uniform_values);
		
		const float v1 = time / 10.0f;
		const float v2 = v1 - std::floor(v1);
		glClearColor(v2 * 0.2f, 0.0f, (1.0f - v2) * 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader_program_blocks.draw(triangle_buffer_openglid);

		SDL_GL_SwapWindow(g_window);
	}

	cleanup_window_graphics();

	return 0;
}
