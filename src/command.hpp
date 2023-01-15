
#ifndef QWY2_HEADER_COMMAND_
#define QWY2_HEADER_COMMAND_

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <functional>
#include <variant>
#include <SDL2/SDL.h>

namespace qwy2
{

class Game;
class Command;

class KeyPress
{
public:
	SDL_Keycode sdl_keycode;

public:
	KeyPress(SDL_Keycode sdl_keycode);
};

class MouseClick
{
public:
	unsigned char sdl_button;

public:
	MouseClick(unsigned char sdl_button);
};

class ControlEvent
{
public:
	bool when_down;
	std::variant<KeyPress, MouseClick> control;

public:
	ControlEvent(std::variant<KeyPress, MouseClick> control, bool when_down);
};

using CommandObject = std::variant<std::string, int, ControlEvent, Command*>;

class BuiltinCommandName
{
public:
	static std::unordered_map<std::string_view, BuiltinCommandName> table;

public:
	static BuiltinCommandName* get(std::string_view name);

public:
	using Callback = std::function<void(Game& game, std::vector<CommandObject> const& args)>;
	Callback callback;

public:
	BuiltinCommandName(Callback callback);
};

void register_builtin_command_names();

class Command
{
public:
	std::string source;
	BuiltinCommandName name;
	std::vector<CommandObject> args;

public:
	Command(BuiltinCommandName name);
	void run(Game& game);
};

Command* parse_command(std::string_view string_command, int* out_command_length = nullptr);

class Control
{
public:
	ControlEvent event;
	Command* command;

public:
	Control(ControlEvent event, Command* command);
};

class InputEventHandler
{
public:
	std::vector<Control> controls;

public:
	void handle_events(Game& game);
};

} /* qwy2 */

#endif /* QWY2_HEADER_COMMAND_ */
