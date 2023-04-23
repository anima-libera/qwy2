
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
	/* Describes the laws of physics of this Qwy2 world.
	 * Some stuff is common to all Qwy2 worlds and this gets hardcoded,
	 * and some stuff can change depending on the seed and this is what `Nature` describes. */
	Nature* nature;

	/* Most of the chunk data about all the chunks are in there. */
	ChunkGrid* chunk_grid;
	/* The length of the edges of the chunks, in blocks.
	 * It must be odd, and should be at least 15 or something. */
	unsigned int chunk_side;

	/* Manages chunk generatation, like which thread get to do which step of which chunk, etc. */
	ChunkGenerationManager chunk_generation_manager;
	float loaded_radius;
	float unloaded_margin;
	
	/* If we want this world to survive the game being closed, and modified chunks to survive
	 * being unloaded, then we would enable loading/saving and have all the world data
	 * be saved to the disk when necessary to be reloaded later when necessary. */
	bool load_save_enabled;
	std::string_view save_name;
	std::string save_directory;

	bool cursor_is_captured;
	
	ShaderTable shader_table;

	bool see_from_sun;
	bool see_through_walls;
	bool see_player_hitboxe;
	bool see_entity_hitboxes;
	bool see_chunk_borders;
	bool see_chunk_generation;
	bool see_from_behind;
	bool render_shadows;
	bool loop_running;
	bool auto_close;

	LineRectDrawer line_rect_drawer;
	Mesh<VertexDataLineUi> pointer_cross_mesh;
	std::optional<BlockFace> pointed_face_opt;

	glm::vec3 sun_position;
	Camera<OrthographicProjection> sun_camera;
	unsigned int shadow_framebuffer_openglid;
	unsigned int shadow_framebuffer_side;

	glm::vec3 sky_color;

	Camera<PerspectiveProjection> player_camera;
	Player player;
	PlayerControls player_controls;

	/* Time at some point during initiialization. */
	std::chrono::time_point<std::chrono::high_resolution_clock> clock_time_beginning;
	/* Time elapsed since some point during the initialization, in seconds. */
	float time;
	float previous_time;

	InputEventHandler input_event_handler;

	ThreadPool thread_pool;

	/* Gameloop profiling, opt-in at compile-time. */
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
 * Note that global variables are not *always* a bad thing,
 * but at some point I might want to get rid of this one. */
extern Game* g_game;

} /* qwy2 */

#endif /* QWY2_HEADER_GAMELOOP_ */
