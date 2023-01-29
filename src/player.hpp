
#ifndef QWY2_HEADER_PLAYER_
#define QWY2_HEADER_PLAYER_

#include "coords.hpp"
#include <glm/vec3.hpp>
#include <optional>

namespace qwy2
{

class ChunkGrid;

/* This type sums up what some input events tell the player to do in terms of motions. */
class PlayerControls
{
private:
	int walking_forward; /* -1, 0 or 1 */
	int walking_rightward; /* -1, 0 or 1 */
	bool will_jump_if_allowed;
	float horizontal_angle_motion;
	float vertical_angle_motion;

	friend class Player;

public:
	PlayerControls();

	/* Resets controls that are to be reset between game loop iterations. */
	void reset();

	void walk_forward();
	void walk_backward();
	void walk_rightward();
	void walk_leftward();
	void jump_if_allowed();

	void move_camera_angle(float horizontal_angle_motion, float vertical_angle_motion);
};

/* Note that this class was only made as a way to take away a part of the gameloop code
 * in other files. It is temporary, and further entities should not be implemented as classes
 * but rather in an Entity Component System that will also handle the player when it is done. */
class Player
{
public:
	/* Non-motion-related state. */
	AlignedBox box;
	float horizontal_angle;
	float vertical_angle;

	/* Motion-related state. */
	bool is_falling;
	glm::vec3 motion;

	/* Settings that act on how controls apply to the player. */
	float moving_angle_factor;
	bool allowed_fast_and_infinite_jumps;

public:
	Player();
	void apply_motion(ChunkGrid const& chunk_grid, PlayerControls const& controls,
		float delta_time);
	std::optional<BlockFace> pointed_face(ChunkGrid const& chunk_grid) const;
	glm::vec3 direction() const;
	glm::vec3 camera_position() const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_PLAYER_ */
