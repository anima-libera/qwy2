
/* GLOP stands for Game Loop Output Performance,
 * it is a small part of Qwy2 that aims at profiling the performance of
 * various sections of the game loop. */

#ifndef QWY2_HEADER_GLOP_
#define QWY2_HEADER_GLOP_

#include <vector>
#include <fstream>
#include <chrono>

namespace qwy2
{

using GlopColumnId = unsigned int;

class Glop
{
private:
	std::ofstream output_stream;
	std::vector<char const*> column_name_table;
	std::vector<float> row_value_table;

public:
	GlopColumnId add_column(char const* name);
	void open_output_stream();
	void close_output_stream();
	void set_column_value(GlopColumnId id, float value);
	void emit_row();
};

class GlopTimer
{
private:
	Glop& glop;
	GlopColumnId id;
	std::chrono::high_resolution_clock::time_point const start_time;

public:
	GlopTimer(Glop& glop, GlopColumnId id);
	~GlopTimer();
};

} /* qwy2 */

#endif /* QWY2_HEADER_GLOP_ */
