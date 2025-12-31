# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.22)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# macOS SDK handling for CMake 3.30+
if(APPLE)
    if(NOT CMAKE_OSX_SYSROOT)
        set(CMAKE_OSX_SYSROOT "macosx" CACHE STRING "macOS SDK")
    endif()
endif()

# ------------------------------------------------------------------------------
# [OPTIONAL BUT RECOMMENDED] Vcpkg toolchain file
# ------------------------------------------------------------------------------
# Auto-detect vcpkg if VCPKG_ROOT is set, but allow override
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" 
        CACHE FILEPATH "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using user-specified toolchain: ${CMAKE_TOOLCHAIN_FILE}")
else()
    message(STATUS "No vcpkg toolchain detected. Using system packages or custom paths.")
endif()

# ------------------------------------------------------------------------------
# [OPTIONAL] Set the version of the project
# ------------------------------------------------------------------------------

include(cmake/LatestVersionConfig.cmake)

# ------------------------------------------------------------------------------
# START OF THE MAIN CMAKE FILE
# ------------------------------------------------------------------------------
project ("BèvarMéjo" VERSION ${PROJECT_VERSION})

include(cmake/ValidVersionConfig.cmake)

message(STATUS "BèvarMéjo version: ${PROJECT_VERSION}")

# ------------------------------------------------------------------------------
# [OPTIONAL] Set the style for the keys in the output file
# ------------------------------------------------------------------------------
# Define the list of valid styles.
set(STYLES "Sentence case" "CamelCase" "kebab-case" "pascalCase" "snake_case")

set(OUT_STYLE "snake_case" CACHE STRING "Style for the keys in the output file. Select between: ${STYLES}")

# Check if the selected style is valid.
list(FIND STYLES "${OUT_STYLE}" STYLE_INDEX)
if(STYLE_INDEX EQUAL -1)
  message(FATAL_ERROR "Invalid style selected: ${OUT_STYLE}. Please select one of: ${STYLES}")
endif()

# Add the style as a preprocessor definition.
add_compile_definitions(OUT_STYLE_${STYLE_INDEX})

message(STATUS "BèvarMéjo output files' keys style: ${OUT_STYLE}")

# ------------------------------------------------------------------------------
# SEARCH FOR THE REQUIRED LIBRARIES
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Pagmo
# ------------------------------------------------------------------------------
find_package(Pagmo REQUIRED)
message(STATUS "Found Pagmo: ${Pagmo_DIR}")

# ------------------------------------------------------------------------------
# nlohmann_json
# ------------------------------------------------------------------------------
find_package(nlohmann_json 3.2.0 REQUIRED)
message(STATUS "Found nlohmann_json ${nlohmann_json_VERSION}: ${nlohmann_json_DIR}")

# ------------------------------------------------------------------------------
# EPANET
# 
# EPANET is not managed through vcpkg, but it is a git submodule of the project
# in bemelib/extern/
# ------------------------------------------------------------------------------
include(extern/EPANET.cmake)

# ------------------------------------------------------------------------------
# Include my library and CLI
# ------------------------------------------------------------------------------
add_subdirectory("src")
add_subdirectory("cli")