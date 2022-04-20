
""" Structure of the project file tree regarding building.

Qwy2 repository root
├─src
│ ├─embedded.hpp (header file allowed for embedding declarations, see buildsystem/embed.py)
│ ├─embedded.cpp (generated cpp file containing embedded content, see buildsystem/embed.py)
│ └─... (all the source files, sub-directories)
├─bin
│ ├─Qwy2 executable
│ └─... (all the stuff that the executable interacts with)
└─build
  ├─build_mode.py (the table mapping the build modes to their ids)
  ├─0000 (build mode id 0000, the corresponding build mode is described in build_mode.py)
  │ ├─tu_data.py (information on the previous compilations of translation units)
  │ ├─link_state.py (information on the previous linking attempt)
  │ └─obj (all the object files)
  │   ├─0000.o (object id 0000, the corresponding translation unit is described in tu_data.py)
  │   ├─0001.o (object id 0001, etc.)
  │   ...
  ├─0001 (build mode id 0001, etc.)
  ...
"""

import os
from buildsystem.utils import *

# All the Qwy2 source files are supposed to be in there, sub-directories are allowed, as well as
# other types of files (that are neither .cpp nor .hpp files).
SRC_DIR = "src"

EMBEDDED_HPP_FILE_NAME = "embedded.hpp"

def get_embedded_hpp_file_path() -> PathHpp:
	return os.path.join(SRC_DIR, EMBEDDED_HPP_FILE_NAME)

EMBEDDED_CPP_FILE_NAME = "embedded.cpp"

def get_embedded_cpp_file_path() -> PathCpp:
	return os.path.join(SRC_DIR, EMBEDDED_CPP_FILE_NAME)

# The executable will be outputted in there, and is supposed to be executed from in there.
BIN_DIR = "bin"

def make_sure_bin_dir_exists() -> None:
	if not os.path.isdir(BIN_DIR):
		os.makedirs(BIN_DIR)

BIN_NAME = "Qwy2"

def get_bin_file_path() -> Path:
	return os.path.join(BIN_DIR, BIN_NAME)

DEPENDENCY_GRAPH_DOT_FILE_NAME = "dependency_graph.dot"

# All the build artifacts are supposed to be produced in there.
BASE_BUILD_DIR = "build"

def make_sure_base_build_dir_exists() -> None:
	if not os.path.isdir(BASE_BUILD_DIR):
		os.makedirs(BASE_BUILD_DIR)

def get_build_dir(build_mode_id: BuildModeId) -> Path:
	return os.path.join(BASE_BUILD_DIR, f"{build_mode_id:04d}")

def make_sure_build_dir_exists(build_mode_id: BuildModeId) -> None:
	build_dir = get_build_dir(build_mode_id)
	if not os.path.isdir(build_dir):
		os.makedirs(build_dir)

def get_obj_dir(build_mode_id: BuildModeId) -> Path:
	return os.path.join(get_build_dir(build_mode_id), "obj")

def make_sure_obj_dir_exists(build_mode_id: BuildModeId) -> None:
	obj_dir = get_obj_dir(build_mode_id)
	if not os.path.isdir(obj_dir):
		os.makedirs(obj_dir)

def get_obj_file_path(build_mode_id: BuildModeId, obj_id: ObjId) -> Path:
	return os.path.join(get_obj_dir(build_mode_id), f"{obj_id:04d}.o")

BUILD_MODE_TABLE_FILE_NAME = "build_mode.py"

def get_build_mode_table_file_path() -> Path:
	return os.path.join(BASE_BUILD_DIR, BUILD_MODE_TABLE_FILE_NAME)

TU_DATA_TABLE_FILE_NAME = "tu_data.py"

def get_tu_data_table_file_path(build_mode_id: BuildModeId) -> Path:
	return os.path.join(get_build_dir(build_mode_id), TU_DATA_TABLE_FILE_NAME)

LINK_STATE_FILE_NAME = "link_state.py"

def get_link_state_file_path(build_mode_id: BuildModeId) -> Path:
	return os.path.join(get_build_dir(build_mode_id), LINK_STATE_FILE_NAME)
