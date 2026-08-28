cmake_minimum_required(VERSION 3.10)
project(daemon_cli_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Discover and add third-party libraries located under src/third_party
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party")
	file(GLOB THIRD_PARTY_PATHS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/*")
	list(SORT THIRD_PARTY_PATHS)

	# Ensure RapidJson (if present) is built first
	set(_rapidjson_rel "src/third_party/RapidJson")
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_rapidjson_rel}")
		list(FIND THIRD_PARTY_PATHS "${_rapidjson_rel}" _found_idx)
		if(_found_idx EQUAL -1)
			list(INSERT THIRD_PARTY_PATHS 0 "${_rapidjson_rel}")
		endif()
	endif()

	foreach(tp_dir IN LISTS THIRD_PARTY_PATHS)
		if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${tp_dir}" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${tp_dir}/CMakeLists.txt")
			add_subdirectory(${tp_dir})
		endif()
	endforeach()
endif()

add_subdirectory(src/daemon_cli)

# Custom target to configure+build using Ninja in an out-of-source directory 'build.ninja'
add_custom_target(ninja_build
	COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_SOURCE_DIR}/build.ninja -G "Ninja"
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/build.ninja --parallel
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Configure and build using Ninja (out-of-source: build.ninja)"
)
