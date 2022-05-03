
#include "glop.hpp"
#include <cassert>

namespace qwy2
{

GlopColumnId Glop::add_column(char const* name)
{
	this->column_name_table.push_back(name);
	this->row_value_table.push_back(0.0f);
	return this->column_name_table.size() - 1;
}

void Glop::open_output_stream()
{
	this->output_stream.open(Glop::OUTPUT_FILE_NAME, std::ios::out);
	for (GlopColumnId id = 0; id < this->column_name_table.size(); id++)
	{
		this->output_stream << this->column_name_table[id];
		if (id == this->column_name_table.size() - 1)
		{
			this->output_stream << '\n';
		}
		else
		{
			this->output_stream << ';';
		}
	}
}

void Glop::close_output_stream()
{
	this->output_stream.close();
}

void Glop::set_column_value(GlopColumnId id, float value)
{
	this->row_value_table[id] = value;
}

void Glop::emit_row()
{
	for (GlopColumnId id = 0; id < this->column_name_table.size(); id++)
	{
		this->output_stream << this->row_value_table[id];
		this->row_value_table[id] = 0.0f;
		if (id == this->column_name_table.size() - 1)
		{
			this->output_stream << '\n';
		}
		else
		{
			this->output_stream << ';';
		}
	}
}

GlopTimer::GlopTimer(Glop& glop, GlopColumnId id):
	glop{glop}, id{id}, start_time{std::chrono::high_resolution_clock::now()}
{
	;
}

GlopTimer::~GlopTimer()
{
	std::chrono::high_resolution_clock::time_point const end_time =
		std::chrono::high_resolution_clock::now();
	float const duration = std::chrono::duration<float>(end_time - this->start_time).count();
	this->glop.set_column_value(this->id, duration);
}

} /* qwy2 */
