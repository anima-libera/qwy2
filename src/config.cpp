
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

	/* The parameters that can be set by command line arguments are all defined here.
	 * The type of a default value is the type of the parameter
	 * and that cannot be changed at runtime. */

	/* Is the cursor initially captured by the game window?
	 * Setting it to false can be more confortable when launching the game in an IDE debugger. */
	this->parameter_table.insert({"cursor_capture"sv, true});

	/* Radius (in blocks) of the spherical zone around the player that gets to be loaded. */
	this->parameter_table.insert({"loaded_radius"sv, 160.0f});
	this->corrector_table.insert({"loaded_radius"sv, [](ParameterType& variant_value){
		float const value = std::get<float>(variant_value);
		if (value <= 0.0f)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The loaded_radius value should be strictly positive, "
				<< "thus " << value << " is not valid."
				<< std::endl;
			return false;
		}
		return true;
	}});

	/* The length (in blocks) of an edge of the cube that is the shape of all chunks.
	 * It should be odd for chunks to have a singe block at their center
	 * (I'm pretty sure I had a good reason when that decision was taken).
	 * This parameter has a big impact on the performances, some values are *significantly*
	 * better than others, and the best value probably depends on the machine.
	 * Beware setting this to too small or too big values can cause lag issues. */
	this->parameter_table.insert({"chunk_side"sv, 31});
	this->corrector_table.insert({"chunk_side"sv, [](ParameterType& variant_value){
		int const value = std::get<int>(variant_value);
		if (value <= 0)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The chunk_side value should be strictly positive, "
				<< "thus " << value << " is not valid."
				<< std::endl;
			return false;
		}
		if (value % 2 == 0)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The chunk_side value should be odd, "
				<< "thus " << value << " is not valid."
				<< std::endl;
			variant_value = value + 1;
		}
		return true;
	}});

	/* Seed of the whole nature and world generation. */
	this->parameter_table.insert({"seed"sv, 9});

	/* The number of threads in the thread pool dedicated to stuff like chunk generation. */
	this->parameter_table.insert({"loading_threads"sv, 2});
	this->corrector_table.insert({"loading_threads"sv, [](ParameterType& variant_value){
		int const value = std::get<int>(variant_value);
		if (value <= 0)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The loading_threads value should be strictly positive, "
				<< "thus " << value << " is not valid."
				<< std::endl;
			return false;
		}
		return true;
	}});

	/* The length (in pixels) of the side of the square-shaped framebuffer in which
	 * the shadow map (shadows cast by the sun) is stored.
	 * Less means faster rendering but also more pixelated shadows.
	 * There is an implementation-dependant maximum value that may cap this parameter
	 * (that is not done here). */
	this->parameter_table.insert({"shadow_map_resolution"sv, 4096});
	this->corrector_table.insert({"shadow_map_resolution"sv, [](ParameterType& variant_value){
		int const value = std::get<int>(variant_value);
		if (value <= 0)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The shadow_map_resolution value should be strictly positive, "
				<< "thus " << value << " is not valid."
				<< std::endl;
			return false;
		}
		return true;
	}});

	/* If ture, then the terrain generation will produce a flat world. */
	this->parameter_table.insert({"flat"sv, false});

	/* If ture, then the terrain generation will produce a flat-ish world with hills. */
	this->parameter_table.insert({"hills"sv, false});

	/* If ture, then the terrain generation will be homogenous in all directions. */
	this->parameter_table.insert({"homogenous"sv, false});

	/* If ture, then the terrain generation will be a flat plane with holes. */
	this->parameter_table.insert({"plane"sv, false});

	/* If ture, then the terrain generation will be infinitely many flat planes with holes. */
	this->parameter_table.insert({"planes"sv, false});

	/* The size (in blocks) of noise detail level used by world generation. */
	this->parameter_table.insert({"noise_size"sv, 15.0f});
	this->corrector_table.insert({"noise_size"sv, [](ParameterType& variant_value){
		int const value = std::get<float>(variant_value);
		if (value <= 0.0f)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The noise_size value should be strictly positive, "
				<< "thus " << value << " is not valid."
				<< std::endl;
			return false;
		}
		return true;
	}});

	/* How dense is the world generated, 0.0f produces an empty world and 1.0f produces
	 * a filled world.
	 * It only influences some world generation methods, and it is best noticed with
	 * world generations like "homogenous" or "plane". */
	this->parameter_table.insert({"density"sv, 0.5f});
	this->corrector_table.insert({"density"sv, [](ParameterType& variant_value){
		int const value = std::get<float>(variant_value);
		if (value < 0.0f || 1.0f < value)
		{
			std::cout << "\x1b[31mCommand line error:\x1b[39m "
				<< "The density value should be between 0.0 (included) and 1.0 (included), "
				<< "thus " << value << " is not valid."
				<< std::endl;
			return false;
		}
		return true;
	}});

	/* If true, then the texture atlas will be emitted as a bitmap file. */
	this->parameter_table.insert({"emit_bitmap"sv, false});

	/* If true, then the game loop will end after one iteration. */
	this->parameter_table.insert({"close"sv, false});
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
				/* Replace '-' characters with '_' characters to allow for the more idiomatic
				 * kebab-case for command line arguments. */
				name += argv[i][j] == '-' ? '_' : argv[i][j];
			}
			char const* value_as_cstring = nullptr;
			if (argv[i][j] == '=')
			{
				j++; /* Skip the '=' character. */
				value_as_cstring = &argv[i][j];
			}

			/* Checks and value change hapen here! */
			ParameterTableType::iterator const parameter = this->parameter_table.find(name);
			if (parameter == this->parameter_table.end())
			{
				std::cout << "\x1b[31mCommand line error:\x1b[39m "
					<< "Unknown parameter name \"" << name << "\"."
					<< std::endl;
				std::cout << "The parameter list can be found "
					<< "in the code of the constructor Config::Config."
					<< std::endl;
				return ErrorCode::ERROR;
			}
			else if (value_as_cstring == nullptr)
			{
				std::cout << "\x1b[31mCommand line error:\x1b[39m "
					<< "No value was given to the parameter \"" << name << "\"."
					<< std::endl;
				std::cout << "Syntax is \"--parameter-name=value\" "
					<< "(all in one command line argument, thus no spaces (or in quotes))."
					<< std::endl;
				return ErrorCode::ERROR;
			}
			if (std::holds_alternative<bool>(parameter->second))
			{
				this->set_parameter<bool>(parameter, value_as_cstring);
			}
			else if (std::holds_alternative<int>(parameter->second))
			{
				this->set_parameter<int>(parameter, value_as_cstring);
			}
			else if (std::holds_alternative<float>(parameter->second))
			{
				this->set_parameter<float>(parameter, value_as_cstring);
			}
			else if (std::holds_alternative<std::string_view>(parameter->second))
			{
				this->set_parameter<std::string_view>(parameter, value_as_cstring);
			}
			else
			{
				/* If this problem araises, then it means that a parameter have been defined
				 * with a type that is not a case handled above.
				 * To fix it, either change the parameter type
				 * (by making sure that its default value has a type handled here),
				 * or add the parameter type to the list of supported types
				 * (by adding it to the types of the variant ParameterType,
				 * adding it in the cases above, making sure Config::set_parameter works with it,
				 * and adding a line of the Config::get function template instantiation). */
				std::cout << "\x1b[31mBug:\x1b[39m "
					<< "Unsupported type for the parameter \"" << name << "\"."
					<< std::endl;
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
			std::cout << "\x1b[31mCommand line error:\x1b[31m "
				<< "Unknown argument syntax \"" << argv[i] << "\"."
				<< std::endl;
			std::cout << "Syntax for giving values to parameters is \"--parameter-name=value\"."
				<< std::endl;
			return ErrorCode::ERROR;
		}
	}
	return ErrorCode::OK;
}

template<typename ValueType>
void Config::set_parameter(
	ParameterTableType::iterator const parameter, char const* value_as_cstring)
{
	ValueType value;
	std::stringstream string_stream{value_as_cstring};
	if constexpr (not std::is_same_v<ValueType, std::string_view>)
	{
		string_stream >> std::boolalpha >> value;
	}
	else
	{
		value = std::string_view{value_as_cstring};
	}
	if (not string_stream.fail())
	{
		CorrectorTableType::iterator const corrector =
			this->corrector_table.find(parameter->first);
		if (corrector != this->corrector_table.end())
		{
			ParameterType variant_value{value};
			if (corrector->second(variant_value))
			{
				/* Since the corrector can modify the variant_value given to it
				 * (to adjust the value in case it is almost valid),
				 * we have to use this maybe modified variant_value instead of the
				 * value variable that could not have been modified. */
				parameter->second = std::get<ValueType>(variant_value);
			}
		}
		else
		{
			/* The parameter does not have a corrector,
			 * it just means that no specific check/adjustment is needed here. */
			parameter->second = value;
		}
	}
	else
	{
		constexpr char const* type =
			std::is_same_v<ValueType, bool> ? "a boolean" :
			std::is_same_v<ValueType, int> ? "an integer" :
			std::is_same_v<ValueType, float> ? "a single precision floating point value" :
			std::is_same_v<ValueType, std::string_view> ? "a string" :
			(assert(false), "h");
		std::cout << "\x1b[31mCommand line error:\x1b[39m "
			<< "Parameter \"" << parameter->first << "\" is " << type << " "
			<< "and \"" << value_as_cstring << "\" failed to be parsed as such."
			<< std::endl;
	}
	std::cout << "[Config] "
		<< parameter->first << " = " << std::get<ValueType>(parameter->second)
		<< std::endl;
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
