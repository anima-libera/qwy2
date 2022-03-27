
#ifndef QWY2_HEADER_INPUT_EVENTS_
#define QWY2_HEADER_INPUT_EVENTS_

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

class InputEventHandler
{
public:
	/* All the new input events (obtained via the `SDL_PollEvent` function) are processed. */
	void handle_events(Game& game);
};

} /* qwy2 */

#endif /* QWY2_HEADER_INPUT_EVENTS_ */
