
""" Actually compiling and linking, it all hapens here. """

from buildsystem.utils import *
from buildsystem.cmdline import Options
from buildsystem.buildmode import get_build_mode
from buildsystem.structure import *
from buildsystem.persistent import *
from buildsystem.tu import TuFileScope

def get_src_file_date_table() -> Dict[PathSrc, float]:
	""" Returns a table of the last modification dates of all the source files. """
	src_file_date_table: Dict[PathSrc, float] = dict()
	for file_path in walk_file_paths(SRC_DIR):
		if os.path.splitext(file_path)[1] in (".cpp", ".hpp"):
			src_file_date_table[file_path] = os.path.getmtime(file_path)
	return src_file_date_table

def get_unused_build_mode_id(build_mode_table: Dict[BuildMode, BuildModeId]) -> BuildModeId:
	return max(build_mode_table.values(), default = -1) + 1

def get_build_mode_id(build_mode: BuildMode) -> BuildModeId:
	""" Gets the build mode id corresponding to the given build mode from the persistent
	build mode table. If necessary, the given build mode is added to the persistent table. """
	old_build_mode_table = get_build_mode_table()
	build_mode_id = old_build_mode_table.get(build_mode, None)
	if build_mode_id == None:
		# Add the given build mode to the persistent table.
		build_mode_id = get_unused_build_mode_id(old_build_mode_table)
		build_mode_table = old_build_mode_table
		build_mode_table[build_mode] = build_mode_id
		save_build_mode_table(build_mode_table)
	return build_mode_id

def get_unused_obj_id(tu_data_table: Dict[PathCpp, TuData]) -> ObjId:
	return max(map(lambda tu_data: tu_data.obj_id, tu_data_table.values()), default = -1) + 1

def build(options: Options) -> bool:
	""" Compiles and links the executable.
	Returns True iff everything went according to the plan. """

	print_verbose(f"Scanning {SRC_DIR} for dates and translation units file scopes.")
	src_file_date_table = get_src_file_date_table()
	cpp_file_paths: List[PathCpp] = list(filter(
		lambda file_path: os.path.splitext(file_path)[1] == ".cpp",
		src_file_date_table.keys()))
	tu_file_scope_table = {cpp_file_path: TuFileScope(cpp_file_path)
		for cpp_file_path in cpp_file_paths}

	build_mode = get_build_mode(options)
	make_sure_base_build_dir_exists()
	build_mode_id = get_build_mode_id(build_mode)
	print_verbose(f"Build mode id is {build_mode_id:04d}.")
	make_sure_build_dir_exists(build_mode_id)
	make_sure_obj_dir_exists(build_mode_id)
	tu_data_table = get_tu_data_table(build_mode_id)

	cpp_file_paths_to_build = []
	for tu_file_scope in tu_file_scope_table.values():
		cpp_file_path = tu_file_scope.cpp_file_path
		# Make sure every translation unit has an object id.
		if cpp_file_path not in tu_data_table:
			tu_data_table[cpp_file_path] = TuData(get_unused_obj_id(tu_data_table), dict())
			obj_id = tu_data_table[cpp_file_path].obj_id
			print_verbose(f"Assigned \"{cpp_file_path}\" to object id {obj_id:04d}.")
		tu_data = tu_data_table.get(cpp_file_path, None)
		# Determine if the examined translation unit has to be compiled/recompiled.
		# Compilation is needed if at least one file in the file scope of the translation unit
		# has been modified since the last successful compilation of the translation unit.
		for src_file_path in tu_file_scope.files():
			old_src_file_date = tu_data.file_dates.get(src_file_path, 0.0)
			if old_src_file_date < src_file_date_table[src_file_path]:
				cpp_file_paths_to_build.append(cpp_file_path)
				break

	if cpp_file_paths_to_build:
		print_verbose("Compiling required for the following cpp files:")
		for cpp_file_path in cpp_file_paths_to_build:
			print_verbose(f"- \"{cpp_file_path}\"")
	for cpp_file_path in cpp_file_paths_to_build:
		tu_data = tu_data_table[cpp_file_path]
		obj_file_path = get_obj_file_path(build_mode_id, tu_data.obj_id)
		print_verbose(f"Compiling \"{cpp_file_path}\" into \"{obj_file_path}\":")
		formatted_compilation_command = build_mode.compilation_command.format(
			cpp = cpp_file_path, obj = obj_file_path)
		print_blue(formatted_compilation_command)
		compilation_exit_status = os.system(formatted_compilation_command)
		if compilation_exit_status != 0:
			return False
		else:
			# Save the dates of the files in the file scope of the translation unit that just got
			# successfully compiled, so that it does not get needlessly recopiled if none of these
			# files is modified until next time.
			tu_data_table[cpp_file_path].file_dates = {
				src_file_path: src_file_date_table[src_file_path]
				for src_file_path in tu_file_scope_table[cpp_file_path].files()}
			save_tu_data_table(build_mode_id, tu_data_table)

	# Determine if linking is necessary
	link_state = get_link_state(build_mode_id)
	bin_file_path = get_bin_file_path()
	make_sure_bin_dir_exists()
	liking_required = False
	if cpp_file_paths_to_build:
		print_verbose("Linking due to new object files:")
		liking_required = True
	elif not link_state.success:
		print_verbose("Linking because the last linking attempt failed:")
		liking_required = True
	elif not os.path.isfile(bin_file_path):
		print_verbose("Linking to produce the executable:")
		liking_required = True
	elif link_state.date < os.path.getmtime(bin_file_path):
		print_verbose("Linking because " +
			"the currenht executable comes from another build mode:")
		liking_required = True
	if liking_required:
		all_obj_file_paths = list(map(
			lambda cpp_file_path: get_obj_file_path(
				build_mode_id, tu_data_table[cpp_file_path].obj_id),
			cpp_file_paths))
		formatted_linking_command = build_mode.linking_command.format(
			obj = " ".join(all_obj_file_paths),
			bin = bin_file_path)
		print_blue(formatted_linking_command)
		linking_exit_status = os.system(formatted_linking_command)
		if linking_exit_status != 0:
			link_state.success = False
			save_link_state(build_mode_id, link_state)
			return False
		else:
			link_state.success = True
			link_state.date = os.path.getmtime(bin_file_path)
			save_link_state(build_mode_id, link_state)
	return True
