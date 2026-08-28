cmake_minimum_required(VERSION 3.10)
project(daemon_cli_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/RapidJson/CMakeLists.txt")
	add_subdirectory(third_party/RapidJson)
endif()

add_subdirectory(src/daemon_cli)
