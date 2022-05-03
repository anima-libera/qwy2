
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

/* Handles the outputting of profiling data to a file in a simple text format:
 * each column has a title (given in the first line) and
 * each row has one single-precision floating point value per column,
 * each row is one line, and values are separated by semicolons.
 * It is intended to be used in the game loop (if the GLOP_ENABLED macro is defined)
 * with one row per iteration. */
class Glop
{
private:
	static constexpr char const* OUTPUT_FILE_NAME = "glop";

private:
	std::ofstream output_stream;
	std::vector<char const*> column_name_table;
	std::vector<float> row_value_table;

public:
	/* Defines a column, should only be called before `Glop::open_output_stream`. */
	GlopColumnId add_column(char const* name);

	/* Opens the output file, and outputs the column names in it. */
	void open_output_stream();

	void close_output_stream();

	/* Sets the value of the cell of the given column for the next row that will be emitted.
	 * The values for columns that are not set are 0.0f by default. */
	void set_column_value(GlopColumnId id, float value);

	/* Outputs a row of values. It is intended to be called at the end of each game loop iteration
	 * after all the column values are set for the next row via `Glop::set_column_value`. */
	void emit_row();
};

/* Initializing a variable of this type will measure the duration of the time between
 * its construction and destruction and report it to the given GLOP via a call to
 * `Glop::set_column_value`. */
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
