cmake_minimum_required(VERSION 3.10)
project(daemon_cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(daemon_cli
  main.cpp
  CommandParser.cpp
  CommandParser.h
  DaemonSocket.cpp
  DaemonSocket.h
  IniConfig.cpp
  IniConfig.h
  ConfigParser.cpp
  ConfigParser.h
)

target_include_directories(daemon_cli PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Ocelot
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

if(UNIX)
  target_compile_definitions(daemon_cli PRIVATE PLATFORM_UNIX)
endif()

# Link RapidJSON headers (header-only INTERFACE target provided by RapidJSON)
target_link_libraries(daemon_cli PRIVATE RapidJSON)
