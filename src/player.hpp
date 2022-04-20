
#ifndef QWY2_HEADER_PLAYER_
#define QWY2_HEADER_PLAYER_

#include "coords.hpp"
#include <glm/vec3.hpp>

namespace qwy2
{

class ChunkGrid;
class PlayerControls;

/* Note that this class was only made as a way to take away a part of the gameloop code
 * in other files. It is temporary, and further entities should not be implemented as classes
 * but rather in an Entity Component System that will also handle the player when it is done. */
class Player
{
public:
	/* Motionless state. */
	AlignedBox box;
	float horizontal_angle;
	float vertical_angle;

	/* Motion state. */
	bool is_falling;
	glm::vec3 motion;

	/* Settings that act on how controls apply on the player. */
	float moving_angle_factor;
	bool allowed_fast_and_infinite_jumps;

public:
	Player();
	void apply_motion(ChunkGrid const& chunk_grid, PlayerControls const& controls);
	glm::vec3 direction() const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_PLAYER_ */
