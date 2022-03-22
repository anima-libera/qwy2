
#ifndef QWY2_HEADER_CONFIG_
#define QWY2_HEADER_CONFIG_

#include "utils.hpp"
#include <variant>
#include <string_view>
#include <unordered_map>

namespace qwy2
{

/* Contains parameters set by default that can be overwritten by the user
 * when launching the game, for example via the command line arguments. */
class Config
{
public:
	using ParameterType = std::variant<bool, int, float, std::string_view>;
	using ParameterTableType = std::unordered_map<std::string_view, ParameterType>;
	ParameterTableType parameter_table;

public:
	/* See the implementation for the supported parameters
	 * and their default value and types. */
	Config();

	/* Parses the command line arguments via the arguments of main to get
	 * user-defined parameters. */
	ErrorCode parse_command_line(int argc, char const* const* argv);

	template<typename ValueType>
	ValueType get(std::string_view parameter_name) const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_CONFIG_ */
