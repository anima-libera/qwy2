
import buildsystem.globals as globals
import os

Path = str
PathSrc = str
PathCpp = str
PathHpp = str
BuildModeId = int
ObjId = int

def print_blue(*args, **kwargs) -> None:
	print("\x1b[36m", end = "")
	print(*args, **kwargs)
	print("\x1b[39m", end = "", flush = True)

def print_error(error, *args, **kwargs) -> None:
	print("\x1b[1m", end = "")
	print(f"\x1b[31m{error}:\x1b[39m ", end = "")
	print(*args, **kwargs)
	print("\x1b[22m", end = "", flush = True)

def print_verbose(*args, **kwargs) -> None:
	if globals.verbose:
		print(*args, **kwargs)

def walk_file_paths(dir_path: Path):
	""" Iterate over all the paths of the files contained in the directory tree
	starting at the given directory. """
	for dir_path, _, file_names in os.walk(dir_path):
		for file_name in file_names:
			file_path = os.path.join(dir_path, file_name)
			yield file_path
