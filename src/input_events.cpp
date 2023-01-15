
#if 0

#include "input_events.hpp"
#include "gameloop.hpp"
#include "chunk.hpp"
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

void ControlActionPlaceBlock::perform(Game& game)
{
	if (game.pointed_face_opt.has_value())
	{
		BlockCoords const coords =
			game.pointed_face_opt->external_coords();
		if (not game.player.box.containing_block_rect().contains(coords))
		{
			BlockTypeId const new_type_id = 3;
			game.chunk_grid->set_block(game.nature, coords, new_type_id);
			std::cout << "Place block" << std::endl;
		}
	}
}

KeyPress::KeyPress(SDL_Keycode sdl_keycode):
	sdl_keycode(sdl_keycode)
{
	;
}

Control::Control(ControlEvent event, ControlAction* action):
	event(event), action(action)
{
	;
}

InputEventHandler::InputEventHandler()
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
							#if 0
							for (auto const& [chunk_coords, chunk] : game.chunk_grid->table)
							{
								chunk->recompute_mesh(*game.nature);
							}
							#endif
							std::cout << "TODO: reimplement" << std::endl;
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
								<< game.chunk_grid->mesh.size() << " chunks" << std::endl;
							#if 0
							for (auto const& [chunk_coords, chunk] : game.chunk_grid->table)
							{
								delete chunk;
							}
							game.chunk_grid->table.clear();
							#endif
							std::cout << "TODO: reimplement" << std::endl;
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
							game.player.allowed_fast_and_infinite_jumps =
								not game.player.allowed_fast_and_infinite_jumps;
							std::cout << "[F] Infinite jumps "
								<< "and running fast like sonic "
								<< (game.player.allowed_fast_and_infinite_jumps ?
									"enabled" : "disabled")
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
							game.chunk_generation_manager.generation_enabled =
								not game.chunk_generation_manager.generation_enabled;
							std::cout << "[T] Chunk generation "
								<< (game.chunk_generation_manager.generation_enabled ?
									"enabled" : "disabled")
								<< std::endl;
						}
					break;

					case SDLK_x:
						if (event.type == SDL_KEYDOWN)
						{
							if (SDL_GL_GetSwapInterval() == 0)
							{
								if (SDL_GL_SetSwapInterval(-1) != 0)
								{
									std::cout << "SDL_GL_SetSwapInterval(-1) is not supported"
										<< std::endl;
									SDL_GL_SetSwapInterval(1);
								}
							}
							else
							{
								SDL_GL_SetSwapInterval(0);
							}
							std::cout << "[X] Vsync "
								<< ((SDL_GL_GetSwapInterval() != 0) ? "enabled" : "disabled")
								<< std::endl;
						}
					break;

					case SDLK_SPACE:
					case SDLK_w:
						if (event.type == SDL_KEYDOWN)
						{
							BlockCoords const coords =
								game.player.box.center - glm::vec3{0.0f, 0.0f, 1.9f};
							BlockTypeId const new_type_id =
								event.key.keysym.sym == SDLK_w ? 3 : 0;
							game.chunk_grid->set_block(game.nature, coords, new_type_id);
							std::cout <<
								(event.key.keysym.sym == SDLK_w ?
									"[W] Place block below" : "[Space] Remove block below")
								<< std::endl;
						}
					break;

					case SDLK_a:
						if (event.type == SDL_KEYDOWN)
						{
							if (game.pointed_face_opt.has_value())
							{
								BlockCoords const coords =
									game.pointed_face_opt->external_coords();
								if (not game.player.box.containing_block_rect().contains(coords))
								{
									BlockTypeId const new_type_id = 3;
									game.chunk_grid->set_block(game.nature, coords, new_type_id);
									std::cout << "[A] Place block"
										<< std::endl;
								}
							}
						}
					break;

					default:
						if (event.type == SDL_KEYDOWN)
						{
							for (Control const& control : this->controls)
							{
								if (std::holds_alternative<KeyPress>(control.event) &&
									std::get<KeyPress>(control.event).sdl_keycode ==
										event.key.keysym.sym)
								{
									control.action->perform(game);
									break;
								}
							}
						}
					break;
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					if ((game.player.is_falling && game.player.allowed_fast_and_infinite_jumps)
						|| (not game.player.is_falling))
					{
						game.player_controls.will_jump = true;
					}
				}
				else if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (game.pointed_face_opt.has_value())
					{
						BlockCoords const coords = game.pointed_face_opt->internal_coords;
						BlockTypeId const new_type_id = 0;
						game.chunk_grid->set_block(game.nature, coords, new_type_id);
						std::cout << "[Left Click] Remove block"
							<< std::endl;
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

#endif
