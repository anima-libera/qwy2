
#ifndef QWY2_HEADER_COMMAND_
#define QWY2_HEADER_COMMAND_

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <functional>
#include <variant>
#include <iostream>
#include <SDL2/SDL.h>

namespace qwy2
{

class Game;
class Command;

/* Identifies one key of the keyboard (like the A or the F12). */
class KeyboardKey
{
public:
	SDL_Keycode sdl_keycode;

public:
	KeyboardKey(SDL_Keycode sdl_keycode);
};

/* Identifies one button of the mouse (like the left click or the wheel click). */
class MouseButton
{
public:
	unsigned char sdl_button;

public:
	MouseButton(unsigned char sdl_button);
};

/* Describes a possible event like a press or release of a key or a click on a mouse button.
 * Here we are only interested in the kind of event that can be bound to a control, like
 * what a used can expect to be able to set as the key to jump or to break a block.
 * This does not describes one specific occurence of an event, it is rather used to bind
 * events to commands. */
class ControlEvent
{
public:
	/* Is the event described be when a key/button is pressed (down, `true`)
	 * or released (up, `false`)? */
	bool when_down;

	std::variant<KeyboardKey, MouseButton> control;

public:
	ControlEvent(std::variant<KeyboardKey, MouseButton> control, bool when_down);

	friend std::ostream& operator<<(std::ostream& out_stream, ControlEvent const& control_event);
};

using CommandObject = std::variant<std::string, int, float, ControlEvent, Command*>;

class BuiltinCommandName
{
public:
	/* The table of all the commands indexed by their name.
	 * Calling `register_builtin_command_names` will fill that table. */
	static std::unordered_map<std::string_view, BuiltinCommandName> table;

public:
	static BuiltinCommandName* get(std::string_view name);

public:
	using Callback = std::function<void(std::vector<CommandObject> const& args)>;
	Callback callback;

public:
	BuiltinCommandName(Callback callback);
};

/* Defines the command names and their code. Must be called before attempting to run commands. */
void register_builtin_command_names();

class Command
{
public:
	/* The source code that was parsed into that command. */
	std::string source;

	/* The name component of the command. */
	BuiltinCommandName name;

	/* The argument values that are to be passed to the name's callback
	 * when running the command. */
	std::vector<CommandObject> args;

public:
	Command(BuiltinCommandName name);

	/* Executes the command, calling the corresponding callback. */
	void run();
};

/* Parses a command's source code text line into a callable command allocated on the heap.
 * TODO: Make it better. */
Command* parse_command(std::string_view string_command, unsigned int* out_command_length = nullptr);

/* Parses and runs all the commands in the given string. */
void run_commands(std::string const& string_commands);

/* One binding of a control event to a command.
 * Whenever the control event is recieved (and handled by the `InputEventHandler`)
 * it triggers the execution of the associated command. */
class ControlBinding
{
public:
	ControlEvent event;
	Command* command;

public:
	ControlBinding(ControlEvent event, Command* command);
};

class InputEventHandler
{
public:
	std::vector<ControlBinding> control_bindings;

public:
	void add_binding(ControlBinding control_binding);

	/* The classic SDL2 event loop, it will run the commands bound to triggered control events
	 * and other small stuff. */
	void handle_events();
};

} /* qwy2 */

#endif /* QWY2_HEADER_COMMAND_ */
