
#if 0

#ifndef QWY2_HEADER_INPUT_EVENTS_
#define QWY2_HEADER_INPUT_EVENTS_

#include <SDL2/SDL.h>
#include <vector>
#include <variant>

namespace qwy2
{

/* This type sums up what some input events tell the player to do. */
class PlayerControls
{
public:
	int walking_forward; /* -1, 0 or 1 */
	int walking_rightward; /* -1, 0 or 1 */
	bool will_jump;
	float horizontal_angle_motion;
	float vertical_angle_motion;

public:
	PlayerControls();
};

class Game;

class ControlAction
{
public:
	virtual void perform(Game& game) = 0;
};

class ControlActionPlaceBlock : public ControlAction
{
public:
	virtual void perform(Game& game) override final;
};

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
};

using ControlEvent = std::variant<KeyPress, MouseClick>;

class Control
{
public:
	ControlEvent event;
	ControlAction* action;

public:
	Control(ControlEvent event, ControlAction* action);
};

class InputEventHandler
{
public:
	std::vector<Control> controls;

public:
	InputEventHandler();

	/* All the new input events (obtained via the `SDL_PollEvent` function) are processed. */
	void handle_events(Game& game);
};

} /* qwy2 */

#endif /* QWY2_HEADER_INPUT_EVENTS_ */

#endif
