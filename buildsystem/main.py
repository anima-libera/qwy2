
""" The main function of the buildsystem that call everything in the right order. """

import os
import sys
import shutil
import buildsystem.globals as globals
from buildsystem.cmdline import Options, HELP_MESSAGE
from buildsystem.utils import *
from buildsystem.structure import *
from buildsystem.embed import handle_embedding
from buildsystem.build import build
from buildsystem.tu import produce_dependency_graph_dot

def clear() -> None:
	""" Erases almost all the traces of previous buildsystem actions.
	Start anew in a new world cleand from the folly of the ancients. """
	print_verbose("Clearing all previous build results and data.")
	embedded_cpp_file_path = get_embedded_cpp_file_path()
	if os.path.isfile(embedded_cpp_file_path):
		print_verbose(f"Deleting the generated cpp file \"{embedded_cpp_file_path}\".")
		os.remove(embedded_cpp_file_path)
	if os.path.isdir(BASE_BUILD_DIR):
		print_verbose(f"Deleting the directory \"{BASE_BUILD_DIR}\".")
		shutil.rmtree(BASE_BUILD_DIR)
	bin_file_path = get_bin_file_path()
	if os.path.isfile(bin_file_path):
		print_verbose(f"Deleting the binary \"{bin_file_path}\".")
		os.remove(bin_file_path)
	if os.path.isfile(DEPENDENCY_GRAPH_DOT_FILE_NAME):
		print_verbose(f"Deleting the generated dot file \"{DEPENDENCY_GRAPH_DOT_FILE_NAME}\".")
		os.remove(DEPENDENCY_GRAPH_DOT_FILE_NAME)

def launch(options: Options) -> None:
	""" Runs the executable from its directory with the proper command line arguments. """
	print(f"Running {BIN_NAME} from \"{BIN_DIR}\":")
	launch_command_args = ["./" + BIN_NAME]
	if options.debug:
		launch_command_args.append("-d")
	for launch_arg in options.launch_args:
		launch_command_args.append(launch_arg)
	launch_command = " ".join(launch_command_args)
	os.chdir(BIN_DIR)
	print_blue(launch_command)
	launch_exit_status_raw = os.system(launch_command)
	launch_exit_status = launch_exit_status_raw >> 8 # On my machine it works...
	if BIN_DIR != ".":
		os.chdir("..")
	if launch_exit_status != 0:
		print_blue("exit status {}".format(launch_exit_status))

def main() -> None:
	""" Buildsystem entrypoint. Calls everything in the right order. """

	options = Options()
	globals.verbose = options.verbose

	if options.help:
		script = sys.argv[0]
		python = "" if script.startswith("./") else "python3 "
		print(HELP_MESSAGE.format(script = python + sys.argv[0]))
		sys.exit(0)

	if options.clear:
		clear()
	
	if options.dependency_graph:
		produce_dependency_graph_dot()

	build_passed = True
	if not options.dont_build:
		# Make sure to handle the embedding before getting dates in the building process so that
		# the generated cpp file will be compiled if it is modified by the embedding.
		handle_embedding()
		# Actually compiling and linking, the core step of all this.
		build_passed = build(options)
	else:
		print_verbose("Not building.")

	if options.launch and build_passed and os.path.isfile(get_bin_file_path()):
		launch(options)
