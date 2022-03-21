#!/usr/bin/env python3

""" Compiles the Qwy2 source code.

Usage:
  {this_script} [options]

Options:
  -h   --help       Prints this docstring and halts.
  -l   --launch     Executes the bin if compiled, with what follows as args.
  -d   --debug      Standard debuging build, defines DEBUG, launches with -d.
  -c=X --compiler=X Uses the X compiler, where X is g++ or clang.
  -v   --verbose    Prints details during building.
  --clear           Erases results and data from previous builds.
  --sdl2-static     Statically link to the SDL2, default is dynamic.
  --use-glew        Uses the GLEW OpenGL extention loader.
  --opengl-notifs   Enables OpenGL notifications.

Example usage for debug:
  {this_script} -d --compiler=g++ -l
"""

import sys
import os
import shutil
import re
import ast
import pprint

def print_blue(*args, **kwargs):
	print("\x1b[36m", end = "")
	print(*args, **kwargs)
	print("\x1b[39m", end = "", flush = True)

def print_error(error, *args, **kwargs):
	print("\x1b[1m", end = "")
	print(f"\x1b[31m{error} error:\x1b[39m ", end = "")
	print(*args, **kwargs)
	print("\x1b[22m", end = "", flush = True)

## COMMAND LINE PARSING

# Checks for the lauch option and separates what is after (to be given to the binary)
# from what is before (other arguments given to this script).
option_launch = False
for i in range(1, len(sys.argv)):
	if sys.argv[i] in ("-l", "--launch"):
		option_launch = True
		launch_index = i
		break
if option_launch:
	options = sys.argv[1:launch_index]
	launch_args = sys.argv[launch_index+1:]
else:
	options = sys.argv[1:]

def cmdline_has_option(expects_value, *option_names):
	# Checks for the given option names and gets its value if one is expected.
	found, value = None, None
	for option in options:
		option_name, *option_value = option.split("=", 1)
		if option_name in option_names:
			if expects_value and len(option_value) == 0:
				print_error("Cmdline",
					f"Argument option \"{option_name}\" expects a value " +
					f"given via the \"{option_name}=value\" syntax.")
				value = None
			elif not expects_value and len(option_value) == 1:
				print_error("Cmdline",
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
	options.remove(found)
	for option in options:
		option_name, *_ = option.split("=", 1)
		if option_name in option_names:
			print_error("Cmdline",
				f"Argument option \"{option_name}\" is redundant.")
			options.remove(option)
	return value

option_help = cmdline_has_option(False, "-h", "--help")
option_debug = cmdline_has_option(False, "-d", "--debug")
option_compiler = cmdline_has_option(True, "-c", "--compiler")
if option_compiler == None or option_compiler == "gcc":
	option_compiler = "g++"
if option_compiler not in ("g++", "clang"):
	print_error("Cmdline", f"The \"{option_compiler}\" compiler " +
		"is not supported.")
	sys.exit(1) # Comment this out to use the given compiler anyway.
if option_debug:
	print(f"Using compiler {option_compiler}")
option_clear = cmdline_has_option(False, "--clear")
option_verbose = cmdline_has_option(False, "-v", "--verbose")
option_sdl2_static = cmdline_has_option(False, "--sdl2-static")
option_use_glew = cmdline_has_option(False, "--use-glew")
option_opengl_notifs = cmdline_has_option(False, "--opengl-notifs")

for unknown_option in options:
	print_error("Cmdline", f"Unknown argument option \"{unknown_option}\".")

# Handle the help message printing which is the docstring of this module
# that is at the beginning of this script (obtained via `__doc__`).
if option_help:
	this_script = sys.argv[0]
	python = "" if this_script.startswith("./") else "python3 "
	print(__doc__.strip().format(this_script = python + sys.argv[0]))
	sys.exit(0)

# The project file system layout should be as follow:
# - A source directory (`src_dir`) containing the C++ and GLSL source code.
# - A binary directory (`bin_dir`) containing the compiled binaries.
# - A build directory (`build_dir`) containing build artifacts such as
#   the previous known date of last modification for each file in the source directory.
# - An object directory (`obj_dir`) inside the build directory (`build_dir`) that contains
#   the object files (.o) resulting from the compilation of all the translation units.
src_dir = "src"
bin_dir = "bin"
build_dir = "build"
obj_dir = os.path.join(build_dir, "obj")

if option_clear:
	if option_verbose:
		print("Clearing previous build results and data.")
	shutil.rmtree(bin_dir)
	shutil.rmtree(build_dir)

bin_name = "Qwy2" if not option_debug else "Qwy2-debug"
print(f"{'Debug' if option_debug else 'Release'} build.")

## PRESISTENT BUILD DATA

date_table_file_name = "date_table.py"
obj_id_table_file_name = "obj_id_table.py"

# In the build directory (`build_dir`) is saved some data about the past builds
# that can be used to avoid recompiling the whole project each time a change is made
# in the cases when it is possible.
# In this project, each source file (.cpp) will be treated a translation unit and
# built separately from the other source files. One source file is given an object id
# which is a number used to name the object file (.o) that is the result from its compilation.
# `obj_dir` is the directory where object files are storred.
# `obj_id_table_file_name` is the name of the file that contains the object ids for all
# the source files (.cpp).
# `date_table_file_name` is the name of the file that contains the date of the last modification
# for all the files that are recursively in source directory (`src_dir`).

# Get the last modification dates saved from the last build.
try:
	date_table_file_path = os.path.join(build_dir, date_table_file_name)
	with open(date_table_file_path, "r") as date_table_file:
		old_date_table = ast.literal_eval(date_table_file.read())
	assert type(old_date_table) == dict
	if option_verbose:
		print(f"Previous date table read from \"{date_table_file_path}\".")
except:
	print("No previous date table.")
	old_date_table = {}
new_date_table = dict(old_date_table)

def is_file_new(file_path):
	# Has the given file been changed from the last build?
	date = os.path.getmtime(file_path)
	old_date = old_date_table.get(file_path, 0.0)
	return old_date < date

# Get the src file to obejct file mapping.
try:
	obj_id_table_file_path = os.path.join(build_dir, obj_id_table_file_name)
	with open(obj_id_table_file_path, "r") as obj_id_table_file:
		obj_id_table = ast.literal_eval(obj_id_table_file.read())
	assert type(obj_id_table) == dict
	if option_verbose:
		print(f"Previous object id table read from \"{obj_id_table_file_path}\".")
except:
	print("No previous object id table.")
	obj_id_table = {}
next_obj_id = max(-1, -1, *obj_id_table.values()) + 1 # Unused object id.

def corresponding_obj_file_path(src_file_path):
	# Path to object file that should be the result of the compilation on the given source file.
	global next_obj_id
	if src_file_path not in obj_id_table:
		obj_id_table[src_file_path] = next_obj_id
		next_obj_id += 1
		if option_verbose:
			print(f"Newly added source file \"{src_file_path}\" " +
				f"is compiled into \"{corresponding_obj_file_path(src_file_path)}\".")
	obj_id = obj_id_table[src_file_path]
	return os.path.join(obj_dir, f"{obj_id:04d}.o")

## EMBEDDED CONTENT

embedded_header_file_name = "embedded.hpp" # See this file for some explanations.
embedded_source_file_name = "embedded.cpp" # This one will be overwritten.
embedded_re = r"EMBEDDED\s*\(\s*\"([^\"]+)\"\s*,\s*(TEXT|BINARY|SIZE)\s*\)\s*([^\s][^;]+[^\s])\s*;"

def escape_file_content(file_path, escape_mode):
	try:
		# Returns the C++ literal corresponding to the content of the given file
		# escaped in the given way.
		def escape_as_string(string):
			return "\"" + string.translate({
				ord("\""): "\\\"", ord("\\"): "\\\\",
				ord("\n"): "\\n", ord("\t"): "\\t"}) + "\""
		opening_mode, escape_function = {
			"TEXT": ("rt", escape_as_string),
			"BINARY": ("rb", lambda bytes: "{" + ", ".join([hex(b) for b in bytes]) + "}"),
			"SIZE": ("rb", lambda bytes: str(len(bytes))),
		}[escape_mode]
		with open(file_path, opening_mode) as file:
			return escape_function(file.read())
	except FileNotFoundError as error:
		print("\x1b[31mEmbedded file error:\x1b[39m " +
			"The embedded content generator couldn't find the file " +
			f"\"{file_path}\" used in an EMBEDDED macro in the " +
			f"\"{embedded_header_file_name}\" header file.")
		raise error

generated_cpp = []
generated_cpp.append("")
generated_cpp.append("/* This file is overwritten at each compilation.")
generated_cpp.append(f" * Do not modify, see \"{embedded_header_file_name}\" " +
	"or \"_comp.py\" instead. */")
generated_cpp.append("")

embedded_header_path = os.path.join(src_dir, embedded_header_file_name)
there_are_new_embedded_files = False
there_are_new_embedded_files |= is_file_new(embedded_header_path)
with open(embedded_header_path, "r") as embedded_header_file:
	for match in re.finditer(embedded_re, embedded_header_file.read()):
		partial_file_path = match.group(1)
		file_path = os.path.join(src_dir, partial_file_path)
		file_is_new = is_file_new(file_path)
		new_date_table[file_path] = os.path.getmtime(file_path)
		there_are_new_embedded_files |= file_is_new
		escape_mode = match.group(2)
		if file_is_new:
			print(f"Embedding file \"{file_path}\" escaped as {escape_mode}.")
		escaped_content = escape_file_content(file_path, escape_mode)
		variable_declaration = match.group(3)
		what = "Size in bytes" if escape_mode == "SIZE" else "Content"
		generated_cpp.append("")
		generated_cpp.append(f"/* {what} of \"{partial_file_path}\". */")
		generated_cpp.append(f"extern {variable_declaration} = {escaped_content};")

if there_are_new_embedded_files:
	embedded_source_path = os.path.join(src_dir, embedded_source_file_name)
	if option_verbose:
		print(f"Generating \"{embedded_source_path}\" with the new embedded files.")
	with open(embedded_source_path, "w") as embedded_source_file:
		embedded_source_file.write("\n".join(generated_cpp) + "\n")

## COMPILE TRANSLATION UNITS

# Recusrively list the files in the source directory (`src_dir`).
src_file_paths = []
new_src_file_paths = []
there_are_new_header_files = False
for dir_name, _, file_names in os.walk(src_dir):
	for file_name in file_names:
		file_path = os.path.join(dir_name, file_name)
		if file_name.split(".")[-1] == "cpp":
			src_file_paths.append(file_path)
			if is_file_new(file_path):
				new_src_file_paths.append(file_path)
			new_date_table[file_path] = os.path.getmtime(file_path)
		elif file_name.split(".")[-1] == "hpp":
			if is_file_new(file_path):
				there_are_new_header_files = True
			new_date_table[file_path] = os.path.getmtime(file_path)

if not os.path.exists(build_dir):
	if option_verbose:
		print(f"Creating the build directory \"{build_dir}\".")
	os.makedirs(build_dir)
if not os.path.exists(obj_dir):
	if option_verbose:
		print(f"Creating the object directory \"{obj_dir}\".")
	os.makedirs(obj_dir) # Should be contained in the `build_dir` directory.

def build_translation_unit(src_file_path):
	# Build the given translation unit
	# and return True iff the compiler does not complain in its exit status.
	build_command_args = []
	build_command_args.append(option_compiler)
	build_command_args.append(src_file_path)
	build_command_args.append("-c")
	build_command_args.append("-o")
	build_command_args.append(corresponding_obj_file_path(src_file_path))
	build_command_args.append("-I" + src_dir)
	build_command_args.append("-std=c++17")
	#build_command_args.append("-pipe")
	build_command_args.append("-Wall")
	build_command_args.append("-Wextra")
	build_command_args.append("-pedantic")
	#build_command_args.append("-Wno-unused-result")
	#build_command_args.append("-Wno-unused-function")
	#if option_compiler == "gcc":
	#	build_command_args.append("-Wno-maybe-uninitialized")
	if option_debug:
		build_command_args.append("-fsanitize=undefined")
		build_command_args.append("-DDEBUG")
		build_command_args.append("-g")
		build_command_args.append("-Og")
	else:
		build_command_args.append("-DNDEBUG") # Should discard asserts
		build_command_args.append("-O3")
		build_command_args.append("-no-pie")
		build_command_args.append("-fno-stack-protector")
	if False:
		build_command_args.append("-v")
		build_command_args.append("-Wl,-v")
	if option_opengl_notifs:
		build_command_args.append("-DENABLE_OPENGL_NOTIFICATIONS")
	build_command = " ".join(build_command_args)
	print_blue(build_command)
	build_exit_status = os.system(build_command)
	return build_exit_status == 0

# If one header has changed, since it could be included in all the source files, 
# then we have to recompile every source file.
# If not, we can only recompile the modified source files.
src_files_to_build = src_file_paths if there_are_new_header_files else new_src_file_paths
if option_verbose:
	if there_are_new_header_files:
		print("The modification of a header file may change all the translation units.")
	if src_files_to_build:
		print("Source files to be compiled:")
		print("- \"" + "\"\n- \"".join(src_files_to_build) + "\"")
	else:
		print("There are no source files to be compiled.")
for src_file_path in src_files_to_build:
	build_is_successful = build_translation_unit(src_file_path)
	if not build_is_successful:
		# If one compiler unit does not compile, then we don't want to save anything to
		# the persistent build data for the next build to start as this one.
		# Note: This could be optimized some day...
		if option_verbose:
			print(f"Building of \"{src_file_path}\" failed, build halts.")
		sys.exit(1)

# Save the current file dates and object file ids to the persistent build data.
if option_verbose:
	print(f"Saving the new date table to \"{date_table_file_path}\".")
with open(date_table_file_path, "w") as date_table_file:
	date_table_file.write(pprint.pformat(new_date_table))
if option_verbose:
	print(f"Saving the new object id table to \"{obj_id_table_file_path}\".")
with open(obj_id_table_file_path, "w") as obj_id_table_file:
	obj_id_table_file.write(pprint.pformat(obj_id_table))

## LINK

if not os.path.exists(bin_dir):
	if option_verbose:
		print(f"Creating the binary directory \"{bin_dir}\".")
	os.makedirs(bin_dir)
bin_path = os.path.join(bin_dir, bin_name)

if option_verbose:
	print(f"Linking of the binary \"{bin_path}\":")
link_command_args = []
link_command_args.append(option_compiler)
for src_file_path in src_file_paths:
	obj_file_path = corresponding_obj_file_path(src_file_path)
	link_command_args.append(obj_file_path)
link_command_args.append("-o")
link_command_args.append(bin_path)
link_command_args.append("-std=c++17")
#link_command_args.append("-pipe")
if option_debug:
	link_command_args.append("-fsanitize=undefined")
	link_command_args.append("-DDEBUG")
	link_command_args.append("-g")
	link_command_args.append("-Og")
else:
	link_command_args.append("-DNDEBUG") # Should discard asserts.
	link_command_args.append("-O3")
	link_command_args.append("-no-pie")
	link_command_args.append("-fno-stack-protector")
	link_command_args.append("-flto")
	link_command_args.append("-s")
if False:
	link_command_args.append("-v")
	link_command_args.append("-Wl,-v")
if option_opengl_notifs:
	link_command_args.append("-DENABLE_OPENGL_NOTIFICATIONS")
link_command_args.append("-lm")
link_command_args.append("-lstdc++")
link_command_args.append("-lpthread")
link_command_args.append("-lGL")
if option_use_glew:
	link_command_args.append("-DGLEW_STATIC") # Doesn't seem to be enough ><...
	link_command_args.append("-lGLEW")
	link_command_args.append("-DUSE_GLEW")
if option_sdl2_static:
	link_command_args.append("`sdl2-config --cflags`")
	link_command_args.append("-static")
	link_command_args.append("-Wl,-Bstatic")
	link_command_args.append("`sdl2-config --static-libs`")
	link_command_args.append("-Wl,-Bdynamic")
	# TODO: At least confirm that it works, which requiers to statically
	# link to a lot of libraries..
else:
	link_command_args.append("`sdl2-config --cflags --libs`")
link_command = " ".join(link_command_args)
print_blue(link_command)
link_exit_status = os.system(link_command)
if link_exit_status != 0 and option_verbose:
	print(f"Linking of \"{bin_path}\" failed, build halts.")
	sys.exit(1)

## LAUNCH

if option_launch:
	print(f"Running {bin_name} from {bin_dir}:")
	launch_command_args = ["./" + bin_name]
	if option_debug:
		launch_command_args.append("-d")
	for launch_arg in launch_args:
		launch_command_args.append(launch_arg)
	launch_command = " ".join(launch_command_args)
	os.chdir(bin_dir)
	print_blue(launch_command)
	launch_exit_status_raw = os.system(launch_command)
	launch_exit_status = launch_exit_status_raw >> 8
	if bin_dir != ".":
		os.chdir("..")
	if launch_exit_status != 0:
		print_blue("exit status {}".format(launch_exit_status))
