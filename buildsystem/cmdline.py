
""" Command line parsing.

An instance of `Options` will contain the result of the parsing.
See the `HELP_MESSAGE` for the available options.
"""

import sys
from buildsystem.utils import *

HELP_MESSAGE = """Compiles and links the Qwy2 source code.

Usage:
  {script} [options]

Options:
  -h   --help        Prints this message and halts.
  -l   --launch      Executes the executable if no error, with what follows as args.
  -d   --debug       Standard debuging build, defines DEBUG, launches with -d.
  -c=X --compiler=X  Uses the X compiler, where X is gcc (g++) or clang.
  -v   --verbose     Prints details during building.
  --clear            Erases results and data from previous builds.
  --dont-build       Refrains from building anything (useful with --clear).
  --graph            Outputs the dependency graph of source files in dot.
  --glop             Enables GLOP that will output performance measurements.
  --opengl-notifs    Enables OpenGL notifications.
  --sdl2-static      Statically links to the SDL2, default is dynamic.
  --use-glew         Uses the GLEW OpenGL extention loader.

Example usage:
  {script} -v --opengl-notifs --compiler=g++ -l
"""

def cmdline_option(cmdline_args: List[str], expects_value: bool, *option_names):
	""" Pops the given option value from the command line arguments (removing it).
	If the option is a flag (there or not there) then a bool is returned.
	If the option expects a value (--option=value), then the value str is returned if found,
	and None is returned if not found. """
	# Checks for the given option names and gets its value if one is expected.
	found, value = None, None
	for option in cmdline_args:
		option_name, *option_value = option.split("=", 1)
		if option_name in option_names:
			if expects_value and len(option_value) == 0:
				print_error("Cmdline error",
					f"Argument option \"{option_name}\" expects a value " +
					f"given via the \"{option_name}=value\" syntax.")
				value = None
			elif not expects_value and len(option_value) == 1:
				print_error("Cmdline error",
					f"Argument option \"{option_name}\" " +
					"doesn't expect a value.")
			elif expects_value and len(option_value) == 1:
				value = option_value[0]
			else:
				value = True
			found = option
			break
	if found == None:
		return None if expects_value else False
	# Check for duplicates of the given option.
	cmdline_args.remove(found)
	for option in cmdline_args:
		option_name, *_ = option.split("=", 1)
		if option_name in option_names:
			print_error("Cmdline error",
				f"Argument option \"{option_name}\" is redundant.")
			cmdline_args.remove(option)
	return value

class Options:
	""" Represents the parsing of the command line arguments given to the buildsystem,
	the parsing is done on creation. """

	def __init__(self):
		# Checks for the lauch option and separates what is after (to be given to the binary)
		# from what is before (other arguments given to this script).
		self.launch = False
		for i in range(1, len(sys.argv)):
			if sys.argv[i] in ("-l", "--launch"):
				self.launch = True
				launch_index = i
				break
		if self.launch:
			cmdline_args = sys.argv[1:launch_index]
			self.launch_args = sys.argv[launch_index+1:]
		else:
			cmdline_args = sys.argv[1:]

		self.help = cmdline_option(cmdline_args, False, "-h", "--help")
		self.debug = cmdline_option(cmdline_args, False, "-d", "--debug")
		self.compiler = cmdline_option(cmdline_args, True, "-c", "--compiler")
		if self.compiler == None or self.compiler == "gcc":
			self.compiler = "g++"
		if self.compiler not in ("g++", "clang"):
			print_error("Cmdline error", f"The \"{self.compiler}\" compiler " +
				"is not supported.")
			sys.exit(1) # Comment this out to use the given compiler anyway.
		if self.debug:
			print(f"Using compiler {self.compiler}")
		self.clear = cmdline_option(cmdline_args, False, "--clear")
		self.dont_build = cmdline_option(cmdline_args, False, "--dont-build")
		self.verbose = cmdline_option(cmdline_args, False, "-v", "--verbose")
		self.dependency_graph = cmdline_option(cmdline_args, False, "--graph")
		self.glop = cmdline_option(cmdline_args, False, "--glop")
		self.sdl2_static = cmdline_option(cmdline_args, False, "--sdl2-static")
		self.use_glew = cmdline_option(cmdline_args, False, "--use-glew")
		self.opengl_notifs = cmdline_option(cmdline_args, False, "--opengl-notifs")

		for unknown_arg in cmdline_args:
			print_error("Cmdline error", f"Unknown argument \"{unknown_arg}\".")
