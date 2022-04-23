
""" Crafting of the compilation and linking commands. """

from typing import *
from buildsystem.cmdline import Options
from buildsystem.structure import SRC_DIR
from buildsystem.persistent import BuildMode

def get_build_mode(options: Options) -> BuildMode:
	""" Uses some of the options to return the compilation and linking commands. """

	# The compilation command is used to call the compiler on each cpp file that has to be
	# compiled/recompiled. The "{cpp}" part will be replaced by the path to the cpp file, and
	# the "{obj}" part will be replaced by the the path to the output object file.
	compilation_command_args: List[str] = []
	compilation_command_args.append(options.compiler)
	compilation_command_args.append("{cpp}")
	compilation_command_args.append("-c")
	compilation_command_args.append("-o")
	compilation_command_args.append("{obj}")
	compilation_command_args.append("-I" + SRC_DIR)
	compilation_command_args.append("-std=c++17")
	#compilation_command_args.append("-pipe")
	compilation_command_args.append("-Wall")
	compilation_command_args.append("-Wextra")
	compilation_command_args.append("-pedantic")
	#compilation_command_args.append("-Wno-unused-result")
	#compilation_command_args.append("-Wno-unused-function")
	#if options.compiler == "g++":
	#	compilation_command_args.append("-Wno-maybe-uninitialized")
	if options.debug:
		compilation_command_args.append("-fsanitize=undefined")
		compilation_command_args.append("-DDEBUG")
		compilation_command_args.append("-g")
		compilation_command_args.append("-Og")
	else:
		compilation_command_args.append("-DNDEBUG") # Should discard asserts.
		compilation_command_args.append("-O3")
	#	compilation_command_args.append("-no-pie")
		compilation_command_args.append("-fno-stack-protector")
	if options.glop:
		compilation_command_args.append("-DGLOP_ENABLED")
	if False:
		compilation_command_args.append("-v")
		compilation_command_args.append("-Wl,-v")
	if options.opengl_notifs:
		compilation_command_args.append("-DENABLE_OPENGL_NOTIFICATIONS")
	compilation_command = " ".join(compilation_command_args)

	# The linking command is used to call the compiler on all the object files that have to be
	# linked into the final executable. The "{obj}" part will be replaced by a space-separated
	# list of all the paths to the object files, and the "{bin}" part will be replaced by
	# the path to the output final executable.
	linking_command_args: List[str] = []
	linking_command_args.append("g++")
	linking_command_args.append("{obj}")
	linking_command_args.append("-o")
	linking_command_args.append("{bin}")
	#linking_command_args.append("-std=c++17")
	#linking_command_args.append("-Wall")
	#linking_command_args.append("-Wextra")
	#linking_command_args.append("-pedantic")
	#linking_command_args.append("-pipe")
	if options.debug:
		linking_command_args.append("-fsanitize=undefined") # GCC links to a runtime lib.
	#	linking_command_args.append("-DDEBUG")
	#	linking_command_args.append("-g")
	#	linking_command_args.append("-Og")
	else:
	#	linking_command_args.append("-DNDEBUG") # Should discard asserts.
	#	linking_command_args.append("-O3")
	#	linking_command_args.append("-no-pie")
	#	linking_command_args.append("-fno-stack-protector")
		linking_command_args.append("-flto")
		linking_command_args.append("-s")
	if False:
		linking_command_args.append("-v")
		linking_command_args.append("-Wl,-v")
	linking_command_args.append("-lm")
	linking_command_args.append("-lstdc++")
	linking_command_args.append("-lpthread")
	linking_command_args.append("-lGL")
	if options.use_glew:
		linking_command_args.append("-DGLEW_STATIC") # Doesn't seem to be enough ><...
		linking_command_args.append("-lGLEW")
		linking_command_args.append("-DUSE_GLEW")
	if options.sdl2_static:
		linking_command_args.append("`sdl2-config --cflags`")
		linking_command_args.append("-static")
		linking_command_args.append("-Wl,-Bstatic")
		linking_command_args.append("`sdl2-config --static-libs`")
		linking_command_args.append("-Wl,-Bdynamic")
		# TODO: At least confirm that it works, which requiers to statically
		# link to a lot of libraries..
	else:
		linking_command_args.append("`sdl2-config --cflags --libs`")
	linking_command = " ".join(linking_command_args)

	return BuildMode(compilation_command, linking_command)
