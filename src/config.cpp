
#include "config.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

namespace qwy2
{

Config::Config()
{
	using namespace std::literals::string_view_literals;

	this->parameter_table.insert({"cursor_capture"sv, true});
	this->parameter_table.insert({"loaded_radius"sv, 160.0f});
	this->parameter_table.insert({"chunk_side"sv, 45});
	this->parameter_table.insert({"seed"sv, 9});
	this->parameter_table.insert({"loading_threads"sv, 2});
	this->parameter_table.insert({"shadow_map_resolution"sv, 4096});
	this->parameter_table.insert({"flat"sv, false});
}

ErrorCode Config::parse_command_line(int argc, char const* const* argv)
{
	for (unsigned int i = 1; i < static_cast<unsigned int>(argc); i++)
	{
		unsigned int j = 0;
		if (argv[i][j++] == '-' && argv[i][j++] == '-')
		{
			/* Command line argument expected of the form "--name=value"
			 * with `name` corresponding to a key of the parameter_table
			 * already filled with all the supported keys (each given a default value)
			 * and `value` being some literal convertible to the value type
			 * expected for the given name (the type of the default value being replaced). */

			std::string name{};
			for (; argv[i][j] != '\0' && argv[i][j] != '='; j++)
			{
				/* Replace '-' characters with '_' characters. */
				name += argv[i][j] == '-' ? '_' : argv[i][j];
			}
			j++; /* Skip the '=' character. */
			char const* const value_as_cstring = &argv[i][j];
			std::cout << "[Config] " << name << " = " << value_as_cstring << std::endl;

			/* Parse the given parameter according to the type of the previous value
			 * (that must not change) and replace the previous value. */
			ParameterTableType::iterator const parameter = this->parameter_table.find(name);
			if (parameter == this->parameter_table.end())
			{
				std::cout << "Command line error: Unknown config parameter name "
					<< "\"" << argv[i] << "\"." << std::endl;
				return ErrorCode::ERROR;
			}
			std::stringstream string_stream{value_as_cstring};
			if (std::holds_alternative<bool>(parameter->second))
			{
				bool value;
				string_stream >> value;
				parameter->second = value;
			}
			else if (std::holds_alternative<int>(parameter->second))
			{
				int value;
				string_stream >> value;
				parameter->second = value;
			}
			else if (std::holds_alternative<float>(parameter->second))
			{
				float value;
				string_stream >> value;
				parameter->second = value;
			}
			else if (std::holds_alternative<std::string_view>(parameter->second))
			{
				parameter->second = std::string_view{value_as_cstring};
			}
			else
			{
				std::cout << "Bug: Unsupported type for the config parameter named "
					<< "\"" << name << "\"." << std::endl;
				return ErrorCode::ERROR;
			}
		}
		else if (std::strcmp(argv[i], "-d") == 0)
		{
			std::cout << "Debug mode huh?" << std::endl;
			/* The build system may add a `-d` command line argument when building
			 * with `-d` and `-l`, but this is not supported yet here. */
		}
		else
		{
			std::cout << "Command line error: Unknown argument "
				<< "\"" << argv[i] << "\"." << std::endl;
			return ErrorCode::ERROR;
		}
	}
	return ErrorCode::OK;
}

template<typename ValueType>
ValueType Config::get(std::string_view parameter_name) const
{
	return std::get<ValueType>(this->parameter_table.at(parameter_name));
}

template bool Config::get(std::string_view parameter_name) const;
template int Config::get(std::string_view parameter_name) const;
template float Config::get(std::string_view parameter_name) const;
template std::string_view Config::get(std::string_view parameter_name) const;

} /* qwy2 */
