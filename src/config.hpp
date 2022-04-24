
#ifndef QWY2_HEADER_CONFIG_
#define QWY2_HEADER_CONFIG_

#include "utils.hpp"
#include <variant>
#include <string_view>
#include <unordered_map>
#include <functional>

namespace qwy2
{

/* Contains parameters set by default that can be overwritten by the user
 * when launching the game, for example via the command line arguments. */
class Config
{
private:
	/* Parameter values. */
	using ParameterType = std::variant<bool, int, float, std::string_view>;
	using ParameterTableType = std::unordered_map<std::string_view, ParameterType>;
	ParameterTableType parameter_table;

	/* Parameter correctors, callables that make sure that the parameter values are valid
	 * and may output warning or error messages aout these values.
	 * In the case a corrector return value is false, then the parameter keeps its default value.
	 * It is fine for a parameter to not have a corrector. */
	using CorrectorType = std::function<bool(ParameterType&)>;
	using CorrectorTableType = std::unordered_map<std::string_view, CorrectorType const>;
	CorrectorTableType corrector_table;

public:
	/* See the implementation for the supported parameters
	 * and their default value and types. */
	Config();

	/* Parses the command line arguments via the arguments of main to get
	 * user-defined parameters. */
	ErrorCode parse_command_line(int argc, char const* const* argv);

private:
	template<typename ValueType>
	void set_parameter(ParameterTableType::iterator const parameter, char const* value_as_cstring);

public:
	template<typename ValueType>
	ValueType get(std::string_view parameter_name) const;
};

} /* qwy2 */

#endif /* QWY2_HEADER_CONFIG_ */
