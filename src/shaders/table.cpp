
#include "shaders/table.hpp"
#include <cassert>
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
	this->table[count++, ShaderTable::LINE_UI] = new ShaderProgramLineUi{};
	assert(count == ShaderTable::SHADER_COUNT);

	/* Compile all the shaders. */
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
		this->table[i]->update_uniform(uniform, value);
	}
}

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

ShaderProgramLineUi& ShaderTable::line_ui()
{
	return *dynamic_cast<ShaderProgramLineUi*>(this->table[ShaderTable::LINE_UI]);
}

} /* qwy2 */
