
#ifndef QWY2_HEADER_GAMELOOP_
#define QWY2_HEADER_GAMELOOP_

#include "chunk.hpp"
#include "camera.hpp"
#include "coords.hpp"
#include "input_events.hpp"
#include "player.hpp"
#include "shaders/line/line_rect.hpp"
#include "shaders/table.hpp"
#include "threadpool.hpp"
#include "glop.hpp"
#include <glm/vec3.hpp>
#include <vector>
#include <chrono>

namespace qwy2
{

class Nature;
class Config;

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
	ChunkGenerationManager chunk_generation_manager;
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
	bool auto_close;
	InputEventHandler input_event_handler;
	PlayerControls player_controls;
	ThreadPool thread_pool;
	Mesh<VertexDataLineUi> pointer_cross_mesh;
	#ifdef GLOP_ENABLED
	Glop glop;
	#endif

public:
	Game(Config const& config);
	
	/* Run the main game loop of Qwy2. */
	void loop();
};

} /* qwy2 */

#endif /* QWY2_HEADER_GAMELOOP_ */
