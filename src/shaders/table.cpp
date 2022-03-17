
#include "shaders/table.hpp"
#include "shaders/classic/classic.hpp"
#include "shaders/line/line.hpp"
#include "shaders/shadow/shadow.hpp"
#include <iostream>

namespace qwy2
{

ErrorCode ShaderTable::init()
{
	this->table.resize(ShaderTable::SHADER_COUNT);
	unsigned int count = 0;
	this->table[count++, ShaderTable::CLASSIC] = new ShaderProgramClassic{};
	this->table[count++, ShaderTable::LINE] = new ShaderProgramLine{};
	this->table[count++, ShaderTable::SHADOW] = new ShaderProgramShadow{};
	assert(count == ShaderTable::SHADER_COUNT);

	for (unsigned int i = 0; i < ShaderTable::SHADER_COUNT; i++)
	{
		if (this->table[i]->init() == ErrorCode::ERROR)
		{
			std::cerr << "Error occured during shader " << i << " compilation" << std::endl;
			this->table.clear();
			return ErrorCode::ERROR;
		}
	}
	return ErrorCode::OK;
}

void ShaderTable::update_uniform(Uniform uniform, UniformValue value)
{
	for (unsigned int i = 0; i < ShaderTable::SHADER_COUNT; i++)
	{
		//std::cerr << static_cast<unsigned int>(uniform) << " [" << std::endl;
		this->table[i]->update_uniform(uniform, value);
		//std::cerr << "]" << std::endl;
	}
}

#if 0
ShaderProgram& ShaderTable::operator[](unsigned int shader_index)
{
	return *this->table[shader_index];
}
#endif

ShaderProgramClassic& ShaderTable::classic()
{
	return *dynamic_cast<ShaderProgramClassic*>(this->table[ShaderTable::CLASSIC]);
}

ShaderProgramLine& ShaderTable::line()
{
	return *dynamic_cast<ShaderProgramLine*>(this->table[ShaderTable::LINE]);
}

ShaderProgramShadow& ShaderTable::shadow()
{
	return *dynamic_cast<ShaderProgramShadow*>(this->table[ShaderTable::SHADOW]);
}

} /* qwy2 */
