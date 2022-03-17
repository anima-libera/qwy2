

#ifndef QWY2_HEADER_SHADER_TABLE_
#define QWY2_HEADER_SHADER_TABLE_

#include "shaders/shader.hpp"
#include "utils.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include "shaders/shadow/shadow.hpp"
#include <vector>

namespace qwy2
{

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
	ErrorCode init();
	void update_uniform(Uniform uniform, UniformValue value);
	//ShaderProgram& operator[](unsigned int shader_index);
	ShaderProgramClassic& classic();
	ShaderProgramLine& line();
	ShaderProgramShadow& shadow();
};

} /* qwy2 */

#endif /* QWY2_HEADER_SHADER_TABLE_ */
