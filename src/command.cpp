
#include "command.hpp"
#include "gameloop.hpp"
#include "coords.hpp"
#include "chunk.hpp"
#include "keycode.hpp"
#include <iostream>

namespace qwy2
{

KeyPress::KeyPress(SDL_Keycode sdl_keycode):
	sdl_keycode(sdl_keycode)
{
	;
}

MouseClick::MouseClick(unsigned char sdl_button):
	sdl_button(sdl_button)
{
	;
}

ControlEvent::ControlEvent(std::variant<KeyPress, MouseClick> control, bool when_down):
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

void register_builtin_command_names()
{
	using namespace std::literals::string_view_literals;
	
	BuiltinCommandName::table.insert(std::make_pair("log"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.size() == 1);
			assert(std::holds_alternative<std::string>(args[0]));
			std::cout << std::get<std::string>(args[0]) << std::endl;
		}}));
	
	BuiltinCommandName::table.insert(std::make_pair("run"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.size() == 1);
			assert(std::holds_alternative<Command*>(args[0]));
			Command* command = std::get<Command*>(args[0]);
			command->run(game);
			std::cout << "Run command [" << command->source << "]." << std::endl;
		}}));
	
	BuiltinCommandName::table.insert(std::make_pair("bind_control"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.size() == 2);
			assert(std::holds_alternative<ControlEvent>(args[0]));
			assert(std::holds_alternative<Command*>(args[1]));
			ControlEvent control_event = std::get<ControlEvent>(args[0]);
			Command* command = std::get<Command*>(args[1]);
			game.input_event_handler.controls.push_back(Control{control_event, command});
			if (std::holds_alternative<KeyPress>(control_event.control))
			{
				std::cout << "Bound key "
					<< "K" << (control_event.when_down ? "D" : "U") << ":"
					<< key_sdl_keycode_to_name(
						std::get<KeyPress>(control_event.control).sdl_keycode)
					<< " to command [" << command->source << "]." << std::endl;
			}
			else
			{
				std::cout << "Bound mouse button "
					<< "M" << (control_event.when_down ? "D" : "U") << ":"
					<< mouse_button_code_to_name(
						std::get<MouseClick>(control_event.control).sdl_button)
					<< " to command [" << command->source << "]." << std::endl;
			}
		}}));

	BuiltinCommandName::table.insert(std::make_pair("quit_game"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.size() == 0);
			game.loop_running = false;
			std::cout << "Quit game" << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("player_move_forward"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_forward += 1;
		}}));
	BuiltinCommandName::table.insert(std::make_pair("player_move_backward"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_forward -= 1;
		}}));
	BuiltinCommandName::table.insert(std::make_pair("player_move_rightward"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_rightward += 1;
		}}));
	BuiltinCommandName::table.insert(std::make_pair("player_move_leftward"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player_controls.walking_rightward -= 1;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("player_jump"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			if ((game.player.is_falling && game.player.allowed_fast_and_infinite_jumps)
				|| (not game.player.is_falling))
			{
				game.player_controls.will_jump = true;
			}
		}}));

	BuiltinCommandName::table.insert(std::make_pair("player_place_block"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
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
		}}));
	
	BuiltinCommandName::table.insert(std::make_pair("player_break_block"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			if (game.pointed_face_opt.has_value())
			{
				BlockCoords const coords = game.pointed_face_opt->internal_coords;
				BlockTypeId const new_type_id = 0;
				game.chunk_grid->set_block(game.nature, coords, new_type_id);
				std::cout << "Break block." << std::endl;
			}
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_capture_cursor"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
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
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_see_from_sun"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_from_sun = not game.see_from_sun;
			std::cout << (game.see_from_sun ? "Enable" : "Disable")
				<< " seeing from the sun." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_see_through_walls"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_through_walls = not game.see_through_walls;
			std::cout << (game.see_through_walls ? "Enable" : "Disable")
				<< " seeing through walls." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_see_boxes"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_boxes = not game.see_boxes;
			std::cout << (game.see_boxes ? "Enable" : "Disable")
				<< " seeing boxes." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_see_chunk_borders"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_chunk_borders = not game.see_chunk_borders;
			std::cout << (game.see_chunk_borders ? "Enable" : "Disable")
				<< " seeing chunk borders." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_see_from_behind"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.see_from_behind = not game.see_from_behind;
			std::cout << (game.see_from_behind ? "Enable" : "Disable")
				<< " seeing player from behind." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_fast_and_infinite_jumps"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.player.allowed_fast_and_infinite_jumps =
				not game.player.allowed_fast_and_infinite_jumps;
			std::cout << (game.player.allowed_fast_and_infinite_jumps ? "Enable" : "Disable")
				<< " fast-and-infinite-jumps mode." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_sun_shadows"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.render_shadows = not game.render_shadows;
			glBindFramebuffer(GL_FRAMEBUFFER, game.shadow_framebuffer_openglid);
			glClear(GL_DEPTH_BUFFER_BIT);
			std::cout << (game.render_shadows ? "Enable" : "Disable")
				<< " sun shadows." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_world_generation"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			game.chunk_generation_manager.generation_enabled =
				not game.chunk_generation_manager.generation_enabled;
			std::cout << (game.chunk_generation_manager.generation_enabled ? "Enable" : "Disable")
				<< " sun shadows." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("toggle_vsync"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
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
		}}));

	BuiltinCommandName::table.insert(std::make_pair("place_block_below_player"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
			assert(args.empty());
			BlockCoords const coords = game.player.box.center - glm::vec3{0.0f, 0.0f, 1.9f};
			game.chunk_grid->set_block(game.nature, coords, 3);
			std::cout << "Place block below the player." << std::endl;
		}}));

	BuiltinCommandName::table.insert(std::make_pair("teleport_player"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
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
		}}));
	BuiltinCommandName::table.insert(std::make_pair("teleport_relative_player"sv, BuiltinCommandName {
		[](Game& game, std::vector<CommandObject> const& args){
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
		}}));
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

Command* parse_command(std::string_view string_command, int* out_command_length)
{
	int i = 0;
	while (i < string_command.length() && string_command[i] == ' ')
	{
		i++;
	}
	
	int name_start = i;
	int name_end = i;
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
			int string_start = i;
			int string_end = i;
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
			int sub_command_length;
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

				ControlEvent control_event{KeyPress(key_code), when_down};
				command->args.push_back(control_event);
			}
			else
			{
				int key_name_start = i;
				int key_name_end = i;
				while (i < string_command.length() && is_key_name_char(string_command[i]))
				{
					i++;
					key_name_end = i;
				}
				std::string_view key_name =
					string_command.substr(key_name_start, key_name_end - key_name_start);
				
				SDL_Keycode sdl_keycode = key_name_to_sdl_keycode(key_name);
				ControlEvent control_event{KeyPress(sdl_keycode), when_down};
				command->args.push_back(control_event);
			}
		}
		else if (i+2 < string_command.length() && string_command[i] == 'M' && string_command[i+2] == ':')
		{
			bool when_down = string_command[i+1] == 'D';
			i += 3;

			int button_name_start = i;
			int button_name_end = i;
			while (i < string_command.length() && is_name_char(string_command[i]))
			{
				i++;
				button_name_end = i;
			}
			std::string_view button_name_string =
				string_command.substr(button_name_start, button_name_end - button_name_start);

			unsigned char sdl_button = mouse_button_name_to_code(button_name_string);
			ControlEvent control_event{MouseClick(sdl_button), when_down};
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
						std::holds_alternative<KeyPress>(control.event.control) &&
						std::get<KeyPress>(control.event.control).sdl_keycode ==
							event.key.keysym.sym)
					{
						control.command->run(game);
						break;
					}
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				for (Control const& control : this->controls)
				{
					if (control.event.when_down == (event.type == SDL_MOUSEBUTTONDOWN) &&
						std::holds_alternative<MouseClick>(control.event.control) &&
						std::get<MouseClick>(control.event.control).sdl_button ==
							event.button.button)
					{
						control.command->run(game);
						break;
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
