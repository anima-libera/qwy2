
#include "window.hpp"
#include "shaders/shader.hpp"
#include "shaders/blocks/blocks.hpp"
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cmath>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

int main()
{
	using namespace qwy2;

	if (init_window_graphics() == ErrorCode::ERROR)
	{
		return EXIT_FAILURE;
	}



	int width, height;
	SDL_GetWindowSize(g_window, &width, &height);
	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	
	const float fovy = TAU / 8.0f;
	const float near = 0.1f;
	const float far = 100.0f;
	const glm::mat4 projection = glm::perspective(fovy, aspect_ratio, near, far);

	glm::mat4 view = glm::lookAt(
		glm::vec3(1.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 camera = projection * view;

	UniformValues uniform_values;
	uniform_values.camera_matrix = camera;


	ShaderProgramBlocks shader_program_blocks;
	if (shader_program_blocks.init() == ErrorCode::ERROR)
	{
		std::cerr << "ono" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << shader_program_blocks.openglid << std::endl;
	shader_program_blocks.update_uniforms(uniform_values);


	float triangles[] = {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	};

	GLuint triangle_buffer_openglid;
	glGenBuffers(1, &triangle_buffer_openglid);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_openglid);
	glBufferData(GL_ARRAY_BUFFER, sizeof triangles, triangles, GL_DYNAMIC_DRAW);



	using clock = std::chrono::high_resolution_clock;

	const auto clock_time_beginning = clock::now();
	float time = 0.0f; /* Time in seconds. */

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
				break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							running = false;
						break;
					}
				break;
			}
		}

		time = std::chrono::duration<float>(clock::now() - clock_time_beginning).count();
		
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
