
import os
import re
from buildsystem.utils import *
from buildsystem.structure import SRC_DIR, DEPENDENCY_GRAPH_DOT_FILE_NAME

def find_direct_dependencies(file_path: PathSrc) -> Set[PathHpp]:
	""" Returns the set of headers directly included in the given source file
	by `#include` preprocessor directives (only with double-quotes, not angle brackets).
	These paths start from `SRC_DIR`, their validity is tested and they are canonicalized. """
	direct_dependencies: Set[PathHpp] = set()
	with open(file_path, "r") as file:
		INCLUDE_RE = r"^\s*#\s*include\s*\"([^\"]+)\"\s*$"
		for match in re.finditer(INCLUDE_RE, file.read(), re.MULTILINE):
			included_raw_path: str = match.group(1)
			# To get the full path starting from `SRC_DIR`,
			# we first try to interpret `included_raw_path` as just missing the `SRC_DIR` part.
			included_file_path = os.path.normpath(os.path.join(SRC_DIR, included_raw_path))
			if not os.path.isfile(included_file_path):
				# If it is not that, we then try to interpret `included_raw_path`
				# as a path relative to the source file it is in.
				local_dir = os.path.dirname(file_path)
				included_file_path = os.path.normpath(os.path.join(local_dir, included_raw_path))
			if not os.path.isfile(included_file_path):
				# If this problem araises, then it should be fixable in different ways:
				# - Change the problematic include directive to make it compatible with
				#   the current state of this dependency analyser (should be preferred).
				# - Extend this dependency analyser to handle the problematic include
				#   directive (only if this seems more reasonable than the first way).
				# - Hack around it (only if deemed necessary, comment it).
				# Note: Include paths should not contain the `SRC_DIR` part (arbitrary decision).
				print_error("Include warning",
					f"Include of \"{included_raw_path}\" in \"{file_path}\" " +
					"could not be interpreted by the build system dependency analyser. " +
					"This may cause the build system to behave improperly " +
					"if not used with `--clear`.")
				continue
			if not os.path.splitext(included_file_path)[1] == ".hpp":
				# Making sure that we know what we are doing if we start including
				# files that are not header files (which is not supposed to happen
				# at the time this is written). If that is the case, this warning should
				# be adapted appropriately, as well as quite a few comments, docstrings,
				# types, names, etc.
				print_error("Include warning",
					f"Include of \"{included_file_path}\" in \"{file_path}\" " +
					"which is not a header file ending with `.hpp`.")
			direct_dependencies.add(included_file_path)
	return direct_dependencies

def find_dependencies(file_path: PathSrc) -> Set[PathHpp]:
	""" Returns the set of headers directly or indirectly included in the given source file.
	Basically just explore the graph induced by `find_direct_dependencies`. """
	dependencies: Set[PathHpp] = set()
	dependencies_to_process = find_direct_dependencies(file_path)
	while dependencies_to_process:
		dependency = dependencies_to_process.pop()
		if (dependency not in dependencies) and (dependency != file_path):
			dependencies.add(dependency)
			dependencies_to_process |= find_direct_dependencies(dependency)
	return dependencies

class TuFileScope:
	""" Sets of files that describe the file scope of one translation unit.
	Some scanning on the src files is done on creation. """

	def __init__(self, cpp_file_path: PathCpp):
		self.cpp_file_path = cpp_file_path
		self.direct_dependencies = find_direct_dependencies(cpp_file_path)
		self.dependencies = find_dependencies(cpp_file_path)

	def files(self):
		return (self.cpp_file_path, *self.dependencies)

def produce_dependency_graph_dot() -> None:
	""" Outputs the include-dependency graph in a DOT file (.dot).
	DOT is a graph description language, there are tools to turn DOT files into
	graph visualizations, such as `dot` from Graphviz (https://graphviz.org/download/).
	On Linux, if dot is installed, running `sh dot.sh` should do. """
	print_verbose("Producing the dependency graph " +
		f"outputted into \"{DEPENDENCY_GRAPH_DOT_FILE_NAME}\".")
	with open(DEPENDENCY_GRAPH_DOT_FILE_NAME, "w") as dot_file:
		dot_file.write("digraph {\n")
		for file_path in walk_file_paths(SRC_DIR):
			if os.path.splitext(file_path)[1] not in (".cpp", ".hpp"):
				continue
			dependencies = find_direct_dependencies(file_path)
			is_cpp = os.path.splitext(file_path)[1] == ".cpp"
			dot_file.write(f"\t\"{file_path}\" [" +
				", ".join((
					f"label=\"{os.path.basename(file_path)}\"", # Will maybe cause confusion...
					f"shape=\"{'box' if is_cpp else 'oval'}\""
				)) + "]\n")
			for dependency in dependencies:
				dot_file.write(f"\t\"{file_path}\" -> \"{dependency}\"\n")
		dot_file.write("}\n")
