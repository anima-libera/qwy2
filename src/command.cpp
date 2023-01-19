
#include "command.hpp"
#include "gameloop.hpp"
#include "coords.hpp"
#include "chunk.hpp"
#include "keycode.hpp"
#include <iostream>

namespace qwy2
{

KeyboardKey::KeyboardKey(SDL_Keycode sdl_keycode):
	sdl_keycode(sdl_keycode)
{
	;
}

MouseButton::MouseButton(unsigned char sdl_button):
	sdl_button(sdl_button)
{
	;
}

ControlEvent::ControlEvent(std::variant<KeyboardKey, MouseButton> control, bool when_down):
	when_down(when_down), control(control)
{
	;
}

std::unordered_map<std::string_view, BuiltinCommandName> BuiltinCommandName::table;

BuiltinCommandName* BuiltinCommandName::get(std::string_view name)
{
	auto it = BuiltinCommandName::table.find(name);
	if (it != BuiltinCommandName::table.end())
	{
		return &it->second;
	}
	else
	{
		return nullptr;
	}
}

BuiltinCommandName::BuiltinCommandName(Callback callback):
	callback(callback)
{
	;
}

static void register_one_builtin_command_name(std::string_view name,
	BuiltinCommandName::Callback callback)
{
	BuiltinCommandName::table.insert(std::make_pair(name, BuiltinCommandName {callback}));
}

void register_builtin_command_names()
{
	using namespace std::literals::string_view_literals;

	/* Here they are, the built-in commands! */

	/* TODO: Replace the `assert`s in command callbacks with something that
	 * produces some kind of error message even in release mode. */
	
	/* Logs a string (arg 0) to stdout. */
	register_one_builtin_command_name("log"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.size() == 1);
			assert(std::holds_alternative<std::string>(args[0]));
			std::cout << std::get<std::string>(args[0]) << std::endl;
		});
	
	/* Runs a command (arg 0) immediately. */
	register_one_builtin_command_name("run"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.size() == 1);
			assert(std::holds_alternative<Command*>(args[0]));
			Command* command = std::get<Command*>(args[0]);
			command->run(game);
			std::cout << "Run command [" << command->source << "]." << std::endl;
		});
	
	/* Binds a control event (arg 0) (stuff like pressing or releasing a key or a mouse button)
	 * to a command (arg 1).
	 * Now, every time that control event is triggered the command will be run.
	 * One control event can be bound to multiple commands (all of which are run unpon trigger). */
	register_one_builtin_command_name("bind_control"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.size() == 2);
			assert(std::holds_alternative<ControlEvent>(args[0]));
			assert(std::holds_alternative<Command*>(args[1]));
			ControlEvent control_event = std::get<ControlEvent>(args[0]);
			Command* command = std::get<Command*>(args[1]);
			game.input_event_handler.controls.push_back(Control{control_event, command});
			if (std::holds_alternative<KeyboardKey>(control_event.control))
			{
				std::cout << "Bound key "
					<< "K" << (control_event.when_down ? "D" : "U") << ":"
					<< keyboard_key_code_to_name(
						std::get<KeyboardKey>(control_event.control).sdl_keycode)
					<< " to command [" << command->source << "]." << std::endl;
			}
			else
			{
				std::cout << "Bound mouse button "
					<< "M" << (control_event.when_down ? "D" : "U") << ":"
					<< mouse_button_code_to_name(
						std::get<MouseButton>(control_event.control).sdl_button)
					<< " to command [" << command->source << "]." << std::endl;
			}
		});

	/* Makes the game to close and terminate execution. */
	register_one_builtin_command_name("quit_game"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.size() == 0);
			game.loop_running = false;
			std::cout << "Quit game." << std::endl;
		});

	/* Makes the player to start moving in a given direction relative to the camera orientation.
	 * Running the opposite command will make the player to stop moving along the given axis,
	 * so it is recommended to bind key presses and releases in a way in which the pressing
	 * of a given key makes the player to walk in some direction and the release of that key
	 * makes the player walk in the opposite direction (so that it stops walking). */
	register_one_builtin_command_name("player_move_forward"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_forward += 1;
			assert(game.player_controls.walking_forward <= 1);
		});
	register_one_builtin_command_name("player_move_backward"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_forward -= 1;
			assert(-1 <= game.player_controls.walking_forward);
		});
	register_one_builtin_command_name("player_move_rightward"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_rightward += 1;
			assert(game.player_controls.walking_rightward <= 1);
		});
	register_one_builtin_command_name("player_move_leftward"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_rightward -= 1;
			assert(-1 <= game.player_controls.walking_rightward);
		});

	/* Makes the player to jump, if allowed. */
	register_one_builtin_command_name("player_jump"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			if ((game.player.is_falling && game.player.allowed_fast_and_infinite_jumps)
				|| (not game.player.is_falling))
			{
				game.player_controls.will_jump = true;
			}
		});

	/* Make the player to place a block on the pointed face. */
	register_one_builtin_command_name("player_place_block"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			if (game.pointed_face_opt.has_value())
			{
				BlockCoords const coords = game.pointed_face_opt->external_coords();
				if (not game.player.box.containing_block_rect().contains(coords))
				{
					BlockTypeId const new_type_id = 3;
					game.chunk_grid->set_block(game.nature, coords, new_type_id);
					std::cout << "Place block." << std::endl;
				}
			}
		});
	
	/* Make the player to break the pointed block. */
	register_one_builtin_command_name("player_break_block"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			if (game.pointed_face_opt.has_value())
			{
				BlockCoords const coords = game.pointed_face_opt->internal_coords;
				BlockTypeId const new_type_id = 0;
				game.chunk_grid->set_block(game.nature, coords, new_type_id);
				std::cout << "Break block." << std::endl;
			}
		});

	/* Toggles the fact that the mouse cursor is visible and free to exit the window or not.
	 * Playing with the mouse cursor free to go is annoying, but when a bug makes closing
	 * the game difficult then it can help to be able to pull out the mouse cursor from there. */
	register_one_builtin_command_name("toggle_capture_cursor"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			if (SDL_GetRelativeMouseMode() == SDL_TRUE)
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
				std::cout << "Release mouse cursor." << std::endl;
			}
			else
			{
				std::cout << "Capture mouse cursor." << std::endl;
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
		});

	/* Displays the world as seen by the sun, from afar. */
	register_one_builtin_command_name("toggle_see_from_sun"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_from_sun = not game.see_from_sun;
			std::cout << (game.see_from_sun ? "Enable" : "Disable")
				<< " seeing from the sun." << std::endl;
		});

	/* Displays the back faces instead of the front faces,
	 * which allows to see through opaque surfaces somewhat. */
	register_one_builtin_command_name("toggle_see_through_walls"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_through_walls = not game.see_through_walls;
			std::cout << (game.see_through_walls ? "Enable" : "Disable")
				<< " seeing through walls." << std::endl;
		});

	/* Displays the hitboxes of stuff like the player. */
	register_one_builtin_command_name("toggle_see_boxes"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_boxes = not game.see_boxes;
			std::cout << (game.see_boxes ? "Enable" : "Disable")
				<< " seeing boxes." << std::endl;
		});

	/* Displays the chunk borders (more like the chunk edges actually). */
	register_one_builtin_command_name("toggle_see_chunk_borders"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_chunk_borders = not game.see_chunk_borders;
			std::cout << (game.see_chunk_borders ? "Enable" : "Disable")
				<< " seeing chunk borders." << std::endl;
		});

	/* Displays the world from a point a bit behind the player, in a 3rd person way. */
	register_one_builtin_command_name("toggle_see_from_behind"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_from_behind = not game.see_from_behind;
			std::cout << (game.see_from_behind ? "Enable" : "Disable")
				<< " seeing player from behind." << std::endl;
		});

	/* Toggles a mode that allows for infinite jumps in the air and faster motions
	 * to explore the world faster. */
	register_one_builtin_command_name("toggle_fast_and_infinite_jumps"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player.allowed_fast_and_infinite_jumps =
				not game.player.allowed_fast_and_infinite_jumps;
			std::cout << (game.player.allowed_fast_and_infinite_jumps ? "Enable" : "Disable")
				<< " fast-and-infinite-jumps mode." << std::endl;
		});

	/* Toggles shadow mapping (the sun projecting shadows on surfaces that it does not see).
	 * Turning this off may help when performances become a problem, as shadow mapping
	 * takes a significant time of the rendering. */
	register_one_builtin_command_name("toggle_sun_shadows"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.render_shadows = not game.render_shadows;
			glBindFramebuffer(GL_FRAMEBUFFER, game.shadow_framebuffer_openglid);
			glClear(GL_DEPTH_BUFFER_BIT);
			std::cout << (game.render_shadows ? "Enable" : "Disable")
				<< " sun shadows." << std::endl;
		});

	/* Toggles the fact that non-generated chunks near the player shall be generated. */
	register_one_builtin_command_name("toggle_world_generation"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			game.chunk_generation_manager.generation_enabled =
				not game.chunk_generation_manager.generation_enabled;
			std::cout << (game.chunk_generation_manager.generation_enabled ? "Enable" : "Disable")
				<< " sun shadows." << std::endl;
		});

	/* Toggle VSync. Letting VSync on is recommened unless it is a problem for some reason. */
	register_one_builtin_command_name("toggle_vsync"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			if (SDL_GL_GetSwapInterval() == 0)
			{
				if (SDL_GL_SetSwapInterval(-1) != 0)
				{
					std::cout << "SDL_GL_SetSwapInterval(-1) is not supported" << std::endl;
					SDL_GL_SetSwapInterval(1);
				}
			}
			else
			{
				SDL_GL_SetSwapInterval(0);
			}
			std::cout << ((SDL_GL_GetSwapInterval() != 0) ? "Enable" : "Disable")
				<< " vsync." << std::endl;
		});

	/* Place a block below the player. This allows to stop falling or to place blocks in an
	 * empty area with nothing to place blocks on with `player_place_block`. */
	register_one_builtin_command_name("place_block_below_player"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.empty());
			BlockCoords const coords = game.player.box.center - glm::vec3{0.0f, 0.0f, 1.9f};
			game.chunk_grid->set_block(game.nature, coords, 3);
			std::cout << "Place block below the player." << std::endl;
		});

	/* Teleports the (center of the) player to the point at the given coordinates
	 * x (arg 0), y (arg 1) and z (arg 2). */
	register_one_builtin_command_name("teleport_player"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.size() == 3);
			assert(std::holds_alternative<int>(args[0]));
			assert(std::holds_alternative<int>(args[1]));
			assert(std::holds_alternative<int>(args[2]));
			int x = std::get<int>(args[0]);
			int y = std::get<int>(args[1]);
			int z = std::get<int>(args[2]);
			game.player.box.center.x = static_cast<float>(x);
			game.player.box.center.y = static_cast<float>(y);
			game.player.box.center.z = static_cast<float>(z);
			std::cout << "Teleport player at "
				<< "(" << x << ", " << y << ", " << z << ")." << std::endl;
		});
	/* Teleports the (center of the) player to the point at the given displacement along the 3 axis
	 * +x (arg 0), +y (arg 1) and +z (arg 2). */
	register_one_builtin_command_name("teleport_relative_player"sv, 
		[]([[maybe_unused]] Game& game, [[maybe_unused]] std::vector<CommandObject> const& args){
			assert(args.size() == 3);
			assert(std::holds_alternative<int>(args[0]));
			assert(std::holds_alternative<int>(args[1]));
			assert(std::holds_alternative<int>(args[2]));
			int x = std::get<int>(args[0]);
			int y = std::get<int>(args[1]);
			int z = std::get<int>(args[2]);
			game.player.box.center.x += static_cast<float>(x);
			game.player.box.center.y += static_cast<float>(y);
			game.player.box.center.z += static_cast<float>(z);
			std::cout << "Teleport player at "
				<< "(" << static_cast<int>(game.player.box.center.x)
				<< ", " << static_cast<int>(game.player.box.center.y)
				<< ", " << static_cast<int>(game.player.box.center.z)
				<< ") with the relative move "
				<< "(" << x << ", " << y << ", " << z << ")." << std::endl;
		});
}

Command::Command(BuiltinCommandName name):
	name(name)
{
	;
}

void Command::run(Game& game)
{
	this->name.callback(game, this->args);
}

static inline bool is_name_char(char c)
{
	return ('a' <= c && c <= 'z') || c == '_';
}
static inline bool is_digit_char(char c)
{
	return ('0' <= c && c <= '9');
}
static inline bool is_key_name_char(char c)
{
	return is_name_char(c) || is_digit_char(c);
}

Command* parse_command(std::string_view string_command, unsigned int* out_command_length)
{
	/* TODO: Make this better! Implement proper abstractions to make this parser eaiser
	 * to read and extend */
	
	unsigned int i = 0;
	while (i < string_command.length() && string_command[i] == ' ')
	{
		i++;
	}
	
	unsigned int name_start = i;
	unsigned int name_end = i;
	while (is_name_char(string_command[i]))
	{
		i++;
		name_end = i;
	}
	std::string_view name_string = string_command.substr(name_start, name_end - name_start);

	auto it = BuiltinCommandName::table.find(name_string);
	if (it == BuiltinCommandName::table.end())
	{
		std::cout << "\x1b[31m" << "Error: " <<
			"No command named \"" << name_string << "\" found."
			<< "\x1b[39m" << std::endl;
		return nullptr;
	}

	BuiltinCommandName name = it->second;
	Command* command = new Command{name};

	while (true)
	{
		while (i < string_command.length() && string_command[i] == ' ')
		{
			i++;
		}

		if (i >= string_command.length() || string_command[i] == ']')
		{
			break;
		}
		else if (string_command[i] == '\"')
		{
			i++;
			unsigned int string_start = i;
			unsigned int string_end = i;
			while (string_command[i] != '\"')
			{
				i++;
				string_end = i;
			}
			i++;
			command->args.emplace_back(std::string(
				string_command.substr(string_start, string_end - string_start)));
		}
		else if (string_command[i] == '[')
		{
			i++;
			std::string_view starting_at_sub_command = string_command.substr(i);
			unsigned int sub_command_length;
			Command* sub_command = parse_command(starting_at_sub_command, &sub_command_length);
			i += sub_command_length;
			i++; /* Discard the matching closing bracket ']'. */
			command->args.push_back(sub_command);
		}
		else if (is_digit_char(string_command[i]))
		{
			int value = 0;
			while (i < string_command.length() && is_digit_char(string_command[i]))
			{
				value = value * 10 + string_command[i] - '0';
				i++;
			}
			command->args.push_back(value);
		}
		else if (i+2 < string_command.length() && string_command[i] == 'K' && string_command[i+2] == ':')
		{
			bool when_down = string_command[i+1] == 'D';
			i += 3;

			if (i < string_command.length() && string_command[i] == 'N')
			{
				i++;
				int key_code = 0;
				while (i < string_command.length() && is_digit_char(string_command[i]))
				{
					key_code = key_code * 10 + string_command[i] - '0';
					i++;
				}

				ControlEvent control_event{KeyboardKey(key_code), when_down};
				command->args.push_back(control_event);
			}
			else
			{
				unsigned int key_name_start = i;
				unsigned int key_name_end = i;
				while (i < string_command.length() && is_key_name_char(string_command[i]))
				{
					i++;
					key_name_end = i;
				}
				std::string_view key_name =
					string_command.substr(key_name_start, key_name_end - key_name_start);
				
				SDL_Keycode sdl_keycode = keyboard_key_name_to_code(key_name);
				ControlEvent control_event{KeyboardKey(sdl_keycode), when_down};
				command->args.push_back(control_event);
			}
		}
		else if (i+2 < string_command.length() && string_command[i] == 'M' && string_command[i+2] == ':')
		{
			bool when_down = string_command[i+1] == 'D';
			i += 3;

			unsigned int button_name_start = i;
			unsigned int button_name_end = i;
			while (i < string_command.length() && is_name_char(string_command[i]))
			{
				i++;
				button_name_end = i;
			}
			std::string_view button_name_string =
				string_command.substr(button_name_start, button_name_end - button_name_start);

			unsigned char sdl_button = mouse_button_name_to_code(button_name_string);
			ControlEvent control_event{MouseButton(sdl_button), when_down};
			command->args.push_back(control_event);
		}
		else
		{
			std::cout << "\x1b[31m" << "Error: " <<
				"Cannot parse arguments in command [" << string_command << "]."
				<< "\x1b[39m" << std::endl;
			if (out_command_length != nullptr)
			{
				*out_command_length = i;
			}
			return nullptr;
		}
	}

	command->source = std::string(string_command.substr(name_start, i - name_start));

	if (out_command_length != nullptr)
	{
		*out_command_length = i;
	}
	return command;
}

Control::Control(ControlEvent event, Command* command):
	event(event), command(command)
{
	;
}

void InputEventHandler::handle_events(Game& game)
{
	/* TODO: Is this the right or best place for this? */
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
				for (Control const& control : this->controls)
				{
					if (control.event.when_down == (event.type == SDL_KEYDOWN) &&
						std::holds_alternative<KeyboardKey>(control.event.control) &&
						std::get<KeyboardKey>(control.event.control).sdl_keycode ==
							event.key.keysym.sym)
					{
						control.command->run(game);
						/* Don't break, there might be other bindings
						 * to the same control event. */
					}
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				for (Control const& control : this->controls)
				{
					if (control.event.when_down == (event.type == SDL_MOUSEBUTTONDOWN) &&
						std::holds_alternative<MouseButton>(control.event.control) &&
						std::get<MouseButton>(control.event.control).sdl_button ==
							event.button.button)
					{
						control.command->run(game);
						/* Don't break, there might be other bindings
						 * to the same control event. */
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
