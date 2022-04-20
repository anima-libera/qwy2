

#ifndef QWY2_HEADER_SHADER_TABLE_
#define QWY2_HEADER_SHADER_TABLE_

#include "shaders/shader.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include "shaders/shadow/shadow.hpp"
#include <vector>

namespace qwy2
{

/* Holds all the shader programs used by the game. */
class ShaderTable
{
public:
	std::vector<ShaderProgram*> table;

public:
	/* Shader indices. */
	enum : unsigned int
	{
		CLASSIC = 0,
		LINE,
		SHADOW,
		SHADER_COUNT,
	};

public:
	/* Initializes the table and compiles all the shaders. */
	ErrorCode init();

	/* Updates the value of the designated uniform to the given value,
	 * across all the shaders (those who are not concerned will ignore this). */
	void update_uniform(Uniform uniform, UniformValue value);

	/* Getters for the contained shaders.
	 * TODO: Refactor these in some more elegant way (maybe using templates). */
	ShaderProgramClassic& classic();
	ShaderProgramLine& line();
	ShaderProgramShadow& shadow();
};

} /* qwy2 */

#endif /* QWY2_HEADER_SHADER_TABLE_ */
