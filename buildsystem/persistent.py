
""" Persistent data reading and writing.

Persistent data is written in text files as Python literals,
this ensures that these can be read by humans to help with debugging,
and it makes the serialization and deserialization process easy,
using `pprint.pformat` and `ast.literal_eval`.
"""

import ast
import pprint
from buildsystem.structure import *
from buildsystem.utils import *

def read_file_as_object(file_path: Path) -> Any:
	try:
		with open(file_path, "r") as file:
			return ast.literal_eval(file.read())
	except OSError:
		return None

## BUILD MODE TABLE

class BuildMode():
	""" The build mode is all there is to know about how the source files are compiled and linked,
	should it change a little then all the files are to be recompiled and linked again.
	These build modes are born from the work of the `buildmode.py` script. """

	def __init__(self, compilation_command: str, linking_command: str):
		self.compilation_command = compilation_command
		self.linking_command = linking_command

	def __eq__(self, right: "BuildMode") -> bool:
		return (self.compilation_command == right.compilation_command and
			self.linking_command == right.linking_command)

	def __hash__(self) -> int:
		return hash((self.compilation_command, self.linking_command))

	@classmethod
	def from_raw(cls, raw: Tuple[str, str]) -> "BuildMode":
		return BuildMode(*raw)

	def get_raw(self) -> Tuple[str, str]:
		return (self.compilation_command, self.linking_command)

def get_build_mode_table() -> Dict[BuildMode, BuildModeId]:
	build_mode_table_file_path = get_build_mode_table_file_path()
	build_mode_table_raw = read_file_as_object(build_mode_table_file_path)
	if build_mode_table_raw == None:
		return dict()
	else:
		build_mode_table = {BuildMode.from_raw(build_mode_raw): obj_id
			for build_mode_raw, obj_id in build_mode_table_raw.items()}
		return build_mode_table

def save_build_mode_table(build_mode_table: Dict[BuildMode, BuildModeId]) -> None:
	build_mode_table_raw = {build_mode.get_raw(): obj_id
		for build_mode, obj_id in build_mode_table.items()}
	build_mode_table_file_path = get_build_mode_table_file_path()
	with open(build_mode_table_file_path, "w") as build_mode_table_file:
		build_mode_table_file.write(pprint.pformat(build_mode_table_raw))

## TRANSLATION UNIT DATA TABLE

class TuData:
	def __init__(self, obj_id: ObjId, file_dates: Dict[Path, float]):
		self.obj_id = obj_id
		self.file_dates = file_dates

	@classmethod
	def from_raw(cls, raw: Tuple[ObjId, Dict[Path, float]]) -> "TuData":
		return TuData(*raw)

	def get_raw(self) -> Tuple[ObjId, Dict[Path, float]]:
		return (self.obj_id, self.file_dates)

def get_tu_data_table(build_mode_id: BuildModeId) -> Dict[PathCpp, TuData]:
	tu_data_table_file_path = get_tu_data_table_file_path(build_mode_id)
	tu_data_table_raw = read_file_as_object(tu_data_table_file_path)
	if tu_data_table_raw == None:
		return dict()
	else:
		tu_data_table = {cpp_file_path: TuData.from_raw(tu_data_raw)
			for cpp_file_path, tu_data_raw in tu_data_table_raw.items()}
		return tu_data_table

def save_tu_data_table(build_mode_id: BuildModeId, tu_data_table: Dict[PathCpp, TuData]) -> None:
	tu_data_table_raw = {cpp_file_path: tu_data.get_raw()
		for cpp_file_path, tu_data in tu_data_table.items()}
	tu_data_table_file_path = get_tu_data_table_file_path(build_mode_id)
	with open(tu_data_table_file_path, "w") as tu_data_table_file:
		tu_data_table_file.write(pprint.pformat(tu_data_table_raw))

## LINK STATE

class LinkState:
	def __init__(self, date: float, success: bool):
		self.date = date
		self.success = success

	@classmethod
	def default(cls) -> "LinkState":
		return LinkState(0.0, False)

	@classmethod
	def from_raw(cls, raw: Tuple[float, bool]) -> "LinkState":
		return LinkState(*raw)

	def get_raw(self) -> Tuple[float, bool]:
		return (self.date, self.success)

def get_link_state(build_mode_id: BuildModeId) -> LinkState:
	link_state_file_path = get_link_state_file_path(build_mode_id)
	link_state_raw = read_file_as_object(link_state_file_path)
	if link_state_raw == None:
		return LinkState.default()
	else:
		return LinkState(*link_state_raw)

def save_link_state(build_mode_id: BuildModeId, link_state: LinkState) -> None:
	link_state_raw = link_state.get_raw()
	link_state_file_path = get_link_state_file_path(build_mode_id)
	with open(link_state_file_path, "w") as link_state_file:
		link_state_file.write(pprint.pformat(link_state_raw))
