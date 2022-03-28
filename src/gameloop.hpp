
#ifndef QWY2_HEADER_GAMELOOP_
#define QWY2_HEADER_GAMELOOP_

#include "config.hpp"
#include "chunk.hpp"
#include "camera.hpp"
#include "coords.hpp"
#include "nature.hpp"
#include "input_events.hpp"
#include "player.hpp"
#include "shaders/line/line_rect.hpp"
#include "shaders/table.hpp"
#include <glm/vec3.hpp>
#include <vector>
#include <optional>
#include <future>

namespace qwy2
{

/* TODO: Move this in some more relevant file. */
class GeneratingChunkWrapper
{
public:
	ChunkCoords chunk_coords;
	std::future<IsolatedChunk*> future;
public:
	GeneratingChunkWrapper(ChunkCoords chunk_coords,
		BlockRect block_rect, Nature const& nature);
};

/* TODO: Refactor in some way that make it easy to find and tinker with any component.
 * That includes (but is not limited to) organizing the member variables. */
class Game
{
public:
	bool cursor_is_captured;
	float loaded_radius;
	LineRectDrawer line_rect_drawer;
	ShaderTable shader_table;
	Nature* nature;
	ChunkGrid* chunk_grid;
	glm::vec3 sun_position;
	Camera<OrthographicProjection> sun_camera;
	unsigned int shadow_framebuffer_openglid;
	unsigned int shadow_framebuffer_side;
	std::vector<std::optional<GeneratingChunkWrapper>> generating_chunk_table;
	bool keep_generating_chunks;
	glm::vec3 sky_color;
	Camera<PerspectiveProjection> player_camera;
	Player player;
	std::chrono::time_point<std::chrono::high_resolution_clock> clock_time_beginning;
	float time; /* In seconds. */
	float previous_time;
	bool see_from_sun;
	bool see_through_walls;
	bool see_boxes;
	bool see_chunk_borders;
	bool see_from_behind;
	bool render_shadows;
	bool loop_running;
	InputEventHandler input_event_handler;
	PlayerControls player_controls;

public:
	Game(Config const& config);
	
	/* Run the main game loop of Qwy2. */
	void loop();
};

} /* qwy2 */

#endif /* QWY2_HEADER_GAMELOOP_ */
