# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#Set the path to the .cmake file in vcpkg that will allow cmake to find information about Pagmo, Boost and all the libraries installed through vcpkg
#The path to the toolchain file can be set in 3 ways:
#     1. when preparing cmake run 
#         `cmake .. --toolchain /path/to/the/file.cmake`        // equivalent to `cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/the/file.cmake`
#     2. when preparing cmake run
#         `cmake .. -DVCPKG_ROOT=/path/to/vcpkg`
#     3. set the enviromental variable VCPKG_ROOT = /path/to/vcpkg // see how to do it base on your OS
#         `cmake ..` 
IF(DEFINED ENV{VCPKG_ROOT})
  set( VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "Path to the rootfolder to vcpkg")
  message(STATUS "VCPKG_ROOT env found, using ${VCPKG_ROOT}")
ELSE()
  set( VCPKG_ROOT "../vcpkg" CACHE PATH "Path to the rootfolder to vcpkg")
  message(STATUS "VCPKG_ROOT env NOT found, using ${VCPKG_ROOT}")
ENDIF()
set( FULL_TOOLCHAIN_FILENAME "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" )    #FILEPATH "Full path to the file Vcpkg"
set( CMAKE_TOOLCHAIN_FILE ${FULL_TOOLCHAIN_FILENAME}
	   CACHE FILEPATH "Vcpkg toolchain file")

# Allow manually specifying an older version during configuration, if not, default to current
set(CURRENT_VERSION 250100)
set(FIRST_VERSION 230600)
if(NOT PROJECT_VERSION)
  math(EXPR MAJOR_VERSION "${CURRENT_VERSION} / 10000")
  math(EXPR MINOR_VERSION "(${CURRENT_VERSION} % 10000) / 100")
  math(EXPR PATCH_VERSION "${CURRENT_VERSION} % 100")
  set(PROJECT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
endif()

# Parse the version string into components
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 MAJOR_VERSION)
list(GET VERSION_LIST 1 MINOR_VERSION)
list(GET VERSION_LIST 2 PATCH_VERSION)

# Calculate version as a single integer for easy comparison
math(EXPR LIBRARY_VERSION 
    "${MAJOR_VERSION} * 10000 + ${MINOR_VERSION} * 100 + ${PATCH_VERSION}")

# Check that the project version is a valid version string (in bounds/years, no patch version greater than 99, minor version is a valid month
if(LIBRARY_VERSION LESS FIRST_VERSION OR LIBRARY_VERSION GREATER CURRENT_VERSION OR PATCH_VERSION GREATER 99 OR MINOR_VERSION GREATER 12 OR MINOR_VERSION EQUAL 0)
  message(FATAL_ERROR "Invalid version string: ${PROJECT_VERSION}")
endif()

# ---------------------------------------------------------------------
# START OF THE MAIN CMAKE FILE
# ---------------------------------------------------------------------

project ("BèvarMéjo" VERSION ${PROJECT_VERSION})

# Add version-specific preprocessor definitions
add_definitions(-DLIBRARY_VERSION=${LIBRARY_VERSION})
add_definitions(-DLIBRARY_MAJOR_VERSION=${MAJOR_VERSION})
add_definitions(-DLIBRARY_MINOR_VERSION=${MINOR_VERSION})
add_definitions(-DLIBRARY_PATCH_VERSION=${PATCH_VERSION})

message(STATUS "BèvarMéjo version: ${PROJECT_VERSION}")

# ---------------------------------------------------------------------
# [OPTIONAL] Set the style for the keys in the output file
# ---------------------------------------------------------------------
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

#Set the path to the EPANET folder and library
#The path to the folder can be set in 2 ways:
#     1. pass a flag at the first cmake invocation
#         `cmake .. -DEPANET_ROOT=/path/to/EPANET`
#     3. set the enviromental variable EPANET_ROOT = /path/to/EPANET // see how to do it base on your OS
#         `cmake ..` 
IF(DEFINED ENV{EPANET_ROOT})
  set( EPANET_ROOT "$ENV{EPANET_ROOT}" CACHE PATH "Path to the rootfolder to EPANET")
  message(STATUS "EPANET_ROOT env found, using ${EPANET_ROOT}")
ELSE()
  set( EPANET_ROOT "${PROJECT_SOURCE_DIR}/../EPANET" CACHE PATH "Path to the rootfolder to EPANET")
  message(STATUS "EPANET_ROOT env NOT found, using ${EPANET_ROOT}")
ENDIF()

# Look for an installation of pagmo in the system.
find_package(Pagmo REQUIRED)

# Look for an installation of nlohmann json in the system.
IF(DEFINED ENV{JSON_ROOT})
  set( JSON_ROOT "$ENV{JSON_ROOT}" CACHE PATH "Path to the rootfolder to JSON")
  message(STATUS "JSON_ROOT env found, using ${JSON_ROOT}")
ELSE()
  set( JSON_ROOT "${PROJECT_SOURCE_DIR}/../json" CACHE PATH "Path to the rootfolder to JSON")
  message(STATUS "JSON_ROOT env NOT found, using ${JSON_ROOT}")
ENDIF()
# Include in the cmake prefix path 
list(APPEND CMAKE_PREFIX_PATH "${JSON_ROOT}/build")
find_package(nlohmann_json 3.2.0 REQUIRED)

# "Look" for an installation of EPANET in the system.
IF(WIN32)
  add_library(epanet SHARED IMPORTED )
  set_target_properties(epanet PROPERTIES
            IMPORTED_LOCATION "${EPANET_ROOT}/build/bin/Release/epanet2.dll"
            IMPORTED_IMPLIB "${EPANET_ROOT}/build/lib/Release/epanet2.lib"
            )
  target_include_directories(epanet INTERFACE ${EPANET_ROOT}/include ${EPANET_ROOT}/src)
ENDIF(WIN32)

IF(APPLE)
  # On my Mac, I want to include the epanet library from source to debug it. 
  IF(CMAKE_BUILD_TYPE MATCHES Debug)
    set(EPANET_FROM_SOURCE TRUE)
  ELSE()
    SET(EPANET_FROM_SOURCE FALSE)
  ENDIF()

  IF(EPANET_FROM_SOURCE)
    message(STATUS "Building EPANET from source")
    set(EPANET_FILES 
      "${EPANET_ROOT}/src/enumstxt.h"
      "${EPANET_ROOT}/src/epanet.c" 
      "${EPANET_ROOT}/src/epanet2.c" 
      "${EPANET_ROOT}/src/errors.dat" 
      "${EPANET_ROOT}/src/funcs.h" 
      "${EPANET_ROOT}/src/genmmd.c" 
      "${EPANET_ROOT}/src/hash.c" 
      "${EPANET_ROOT}/src/hash.h" 
      "${EPANET_ROOT}/src/hydcoeffs.c" 
      "${EPANET_ROOT}/src/hydraul.c" 
      "${EPANET_ROOT}/src/hydsolver.c" 
      "${EPANET_ROOT}/src/hydstatus.c" 
      "${EPANET_ROOT}/src/inpfile.c" 
      "${EPANET_ROOT}/src/input1.c" 
      "${EPANET_ROOT}/src/input2.c" 
      "${EPANET_ROOT}/src/input3.c" 
      "${EPANET_ROOT}/src/mempool.c" 
      "${EPANET_ROOT}/src/mempool.h" 
      "${EPANET_ROOT}/src/output.c" 
      "${EPANET_ROOT}/src/project.c" 
      "${EPANET_ROOT}/src/quality.c" 
      "${EPANET_ROOT}/src/qualreact.c" 
      "${EPANET_ROOT}/src/qualroute.c" 
      "${EPANET_ROOT}/src/report.c" 
      "${EPANET_ROOT}/src/rules.c" 
      "${EPANET_ROOT}/src/smatrix.c" 
      "${EPANET_ROOT}/src/text.h" 
      "${EPANET_ROOT}/src/types.h" 
      "${EPANET_ROOT}/src/validate.c"
      "${EPANET_ROOT}/src/util/cstr_helper.c"
      "${EPANET_ROOT}/src/util/cstr_helper.h"
      "${EPANET_ROOT}/src/util/errormanager.c"
      "${EPANET_ROOT}/src/util/errormanager.h"
      "${EPANET_ROOT}/src/util/filemanager.c"
      "${EPANET_ROOT}/src/util/filemanager.h"
    )

    add_library(epanet ${EPANET_FILES})

    target_include_directories(epanet PUBLIC 
                                  "${EPANET_ROOT}/include"
                                  "${EPANET_ROOT}/src"
    )
  ELSE() # EPANET non from source
    add_library(epanet SHARED IMPORTED)
    set_target_properties(epanet PROPERTIES
              IMPORTED_LOCATION "${EPANET_ROOT}/build/lib/libepanet2.dylib" 
              )
    target_include_directories(epanet INTERFACE ${EPANET_ROOT}/include ${EPANET_ROOT}/src)
  ENDIF()
ENDIF(APPLE)

IF(LINUX)
  add_library(epanet SHARED IMPORTED)
  set_target_properties(epanet PROPERTIES
        IMPORTED_LOCATION "${EPANET_ROOT}/build/lib/libepanet2.so"
        )
  target_include_directories(epanet INTERFACE ${EPANET_ROOT}/include ${EPANET_ROOT}/src)
ENDIF(LINUX)

# Allow the user to define ENABLE_SAFETY_CHECKS manually
option(ENABLE_SAFETY_CHECKS "Enable safety checks" OFF)

# Define ENABLE_SAFETY_CHECKS in Debug mode if not defined manually
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_SAFETY_CHECKS)
  set(ENABLE_SAFETY_CHECKS ON)
endif()

# Add the definition to the compiler flags
if (ENABLE_SAFETY_CHECKS)
  add_definitions(-DENABLE_SAFETY_CHECKS)
endif()

# Include my library
add_subdirectory("bevarmejolib")

# Include the specific directories of the problems
add_subdirectory("Hanoi")
add_subdirectory("Anytown")

# Include the executables and the experiment classes
add_subdirectory("cli")