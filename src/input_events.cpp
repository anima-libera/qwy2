
#include "input_events.hpp"
#include "gameloop.hpp"
#include <SDL2/SDL.h>
#include <iostream>

namespace qwy2
{

PlayerControls::PlayerControls():
	walking_forward{0},
	walking_rightward{0},
	will_jump{false},
	horizontal_angle_motion{0.0f},
	vertical_angle_motion{0.0f}
{
	;
}

void InputEventHandler::handle_events(Game& game)
{
	game.player_controls.will_jump = false;
	game.player_controls.horizontal_angle_motion = 0.0f;
	game.player_controls.vertical_angle_motion = 0.0f;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				game.loop_running = false;
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
							std::cout << "[Escape] Quit" << std::endl;
							game.loop_running = false;
						}
					break;

					case SDLK_z:
						game.player_controls.walking_forward +=
							(event.type == SDL_KEYDOWN) * 2 - 1;
					break;
					case SDLK_s:
						game.player_controls.walking_forward -=
							(event.type == SDL_KEYDOWN) * 2 - 1;
					break;
					case SDLK_q:
						game.player_controls.walking_rightward -=
							(event.type == SDL_KEYDOWN) * 2 - 1;
					break;
					case SDLK_d:
						game.player_controls.walking_rightward +=
							(event.type == SDL_KEYDOWN) * 2 - 1;
					break;

					case SDLK_p:
						if (event.type == SDL_KEYDOWN)
						{
							std::cout << "[P] Recompute meshes" << std::endl;
							for (auto const& [chunk_coords, chunk] : game.chunk_grid->table)
							{
								chunk->recompute_mesh(*game.nature);
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
							game.see_from_sun = not game.see_from_sun;
							std::cout << "[M] See from sun "
								<< (game.see_from_sun ? "enabled" : "disabled") << std::endl;
						}
					break;

					case SDLK_u:
						if (event.type == SDL_KEYDOWN)
						{
							std::cout << "[U] Player tp upward (z+=20)" << std::endl;
							game.player.box.center.z += 20.0f;
						}
					break;
					case SDLK_j:
						if (event.type == SDL_KEYDOWN)
						{
							std::cout << "[J] Player tp at floor level (z=3)" << std::endl;
							game.player.box.center.z = 3.0f;
						}
					break;

					case SDLK_n:
						if (event.type == SDL_KEYDOWN)
						{
							std::cout << "[N] Unloading "
								<< game.chunk_grid->table.size() << " chunks" << std::endl;
							for (auto const& [chunk_coords, chunk] : game.chunk_grid->table)
							{
								delete chunk;
							}
							game.chunk_grid->table.clear();
						}
					break;

					case SDLK_h:
						if (event.type == SDL_KEYDOWN)
						{
							game.see_through_walls = not game.see_through_walls;
							std::cout << "[H] See through walls "
								<< (game.see_through_walls ? "enabled" : "disabled") << std::endl;
						}
					break;

					case SDLK_b:
						if (event.type == SDL_KEYDOWN)
						{
							game.see_boxes = not game.see_boxes;
							std::cout << "[B] See boxes "
								<< (game.see_boxes ? "enabled" : "disabled") << std::endl;
						}
					break;

					case SDLK_g:
						if (event.type == SDL_KEYDOWN)
						{
							game.see_chunk_borders = not game.see_chunk_borders;
							std::cout << "[B] See chunk borders "
								<< (game.see_chunk_borders ? "enabled" : "disabled") << std::endl;
						}
					break;

					case SDLK_v:
						if (event.type == SDL_KEYDOWN)
						{
							game.see_from_behind = not game.see_from_behind;
							std::cout << "[V] See from behind "
								<< (game.see_from_behind ? "enabled" : "disabled") << std::endl;
						}
					break;

					case SDLK_f:
						if (event.type == SDL_KEYDOWN)
						{
							game.player.allowed_fast_and_infinite_jumps = not game.player.allowed_fast_and_infinite_jumps;
							std::cout << "[F] Infinite jumps "
								<< "and running fast like sonic "
								<< (game.player.allowed_fast_and_infinite_jumps ? "enabled" : "disabled")
								<< std::endl;
						}
					break;

					case SDLK_c:
						if (event.type == SDL_KEYDOWN)
						{
							game.render_shadows = not game.render_shadows;
							std::cout << "[C] Sun shadows "
								<< (game.render_shadows ? "enabled" : "disabled") << std::endl;
							glBindFramebuffer(GL_FRAMEBUFFER, game.shadow_framebuffer_openglid);
							glClear(GL_DEPTH_BUFFER_BIT);
						}
					break;

					case SDLK_t:
						if (event.type == SDL_KEYDOWN)
						{
							game.keep_generating_chunks = not game.keep_generating_chunks;
							std::cout << "[T] Chunk generation "
								<< (game.keep_generating_chunks ? "enabled" : "disabled")
								<< std::endl;
						}
					break;
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_RIGHT && event.type == SDL_MOUSEBUTTONDOWN)
				{
					if ((game.player.is_falling && game.player.allowed_fast_and_infinite_jumps)
						|| (not game.player.is_falling))
					{
						game.player_controls.will_jump = true;
					}
				}
			break;

			case SDL_MOUSEMOTION:
				game.player_controls.horizontal_angle_motion +=
					-event.motion.xrel * game.player.moving_angle_factor;
				game.player_controls.vertical_angle_motion +=
					-event.motion.yrel * game.player.moving_angle_factor;
			break;
		}
	}
}

} /* qwy2 */