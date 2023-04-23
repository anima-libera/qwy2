
#ifndef QWY2_HEADER_GAMELOOP_
#define QWY2_HEADER_GAMELOOP_

#include "chunk.hpp"
#include "camera.hpp"
#include "coords.hpp"
#include "command.hpp"
#include "player.hpp"
#include "shaders/line/line_rect.hpp"
#include "shaders/table.hpp"
#include "threadpool.hpp"
#include "glop.hpp"
#include "entity.hpp"
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
	bool load_save_enabled;
	std::string_view save_name;
	std::string save_directory;
	bool cursor_is_captured;
	float loaded_radius;
	float unloaded_margin;
	LineRectDrawer line_rect_drawer;
	ShaderTable shader_table;
	Nature* nature;
	ChunkGrid* chunk_grid;
	/* The length of the edges of the chunks, in blocks.
	* It must be odd, and should be at least 15 or something. */
	unsigned int chunk_side;
	glm::vec3 sun_position;
	Camera<OrthographicProjection> sun_camera;
	unsigned int shadow_framebuffer_openglid;
	unsigned int shadow_framebuffer_side;
	ChunkGenerationManager chunk_generation_manager;
	glm::vec3 sky_color;
	Camera<PerspectiveProjection> player_camera;
	Player player;
	PlayerControls player_controls;
	std::chrono::time_point<std::chrono::high_resolution_clock> clock_time_beginning;
	/* Time since some moment during the initialization, in seconds. */
	float time;
	float previous_time;
	bool see_from_sun;
	bool see_through_walls;
	bool see_boxes;
	bool see_entity_hitboxes;
	bool see_chunk_borders;
	bool see_from_behind;
	bool render_shadows;
	bool loop_running;
	bool auto_close;
	InputEventHandler input_event_handler;
	ThreadPool thread_pool;
	Mesh<VertexDataLineUi> pointer_cross_mesh;
	std::optional<BlockFace> pointed_face_opt;
	#ifdef GLOP_ENABLED
		Glop glop;
	#endif

public:
	Game();

	/* Initializes the whole `Game` (at a time when `g_game` already points
	 * to a valid `Game` instance, which would not be the case in the constructor
	 * (and a placement new operator hack does not solves the issue of the in-construction
	 * instance being invalid until the constructor's initializer list finishes)). */
	void init(Config const& config);
	
	/* Runs the main game loop of Qwy2. */
	void loop();
};

/* The global and only `Game` instance.
 * Note that global variables are not *always* a bad thing. */
extern Game* g_game;

} /* qwy2 */

#endif /* QWY2_HEADER_GAMELOOP_ */
