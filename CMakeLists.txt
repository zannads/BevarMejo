# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.22)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ------------------------------------------------------------------------------
# [MANDATORY] Vcpkg toolchain file
# ------------------------------------------------------------------------------

# Set the path to the .cmake file in vcpkg that will allow cmake to find information about Pagmo, Boost and all the libraries installed through vcpkg
# The path to the toolchain file can be set when preparing cmake run 
# `cmake .. --toolchain /path/to/the/file.cmake` // equivalent to `cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/the/file.cmake`
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")

# ------------------------------------------------------------------------------
# [OPTIONAL] Set the version of the project
# ------------------------------------------------------------------------------

# Allow manually specifying an older version during configuration, if not, default to current.
set(CURRENT_VERSION 250303)
set(FIRST_VERSION 230600)
if(NOT PROJECT_VERSION OR PROJECT_VERSION STREQUAL "latest")
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
math(EXPR BEME_VERSION 
    "${MAJOR_VERSION} * 10000 + ${MINOR_VERSION} * 100 + ${PATCH_VERSION}")

# Check that the project version is a valid version string (in bounds/years, no patch version greater than 99, minor version is a valid month
if(BEME_VERSION LESS FIRST_VERSION OR BEME_VERSION GREATER CURRENT_VERSION OR PATCH_VERSION GREATER 99 OR MINOR_VERSION GREATER 12 OR MINOR_VERSION EQUAL 0)
  message(FATAL_ERROR "Invalid version string: ${PROJECT_VERSION}")
endif()

# ------------------------------------------------------------------------------
# START OF THE MAIN CMAKE FILE
# ------------------------------------------------------------------------------

project ("BèvarMéjo" VERSION ${PROJECT_VERSION})

# Add version-specific preprocessor definitions
add_definitions(-DBEME_VERSION=${BEME_VERSION})
add_definitions(-DBEME_MAJOR_VERSION=${MAJOR_VERSION})
add_definitions(-DBEME_MINOR_VERSION=${MINOR_VERSION})
add_definitions(-DBEME_PATCH_VERSION=${PATCH_VERSION})

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

# Look for an installation of pagmo in the system.
find_package(Pagmo REQUIRED)

# ------------------------------------------------------------------------------
# Look for an installation of nlohmann json in the system.
message(STATUS "Required nlohmann_json version: 3.2.0")
set(JSON_ROOT "${PROJECT_SOURCE_DIR}/../json" CACHE PATH "Path to the rootfolder to JSON")
# Include in the cmake prefix path 
list(APPEND CMAKE_PREFIX_PATH "${JSON_ROOT}/build")
find_package(nlohmann_json 3.2.0 REQUIRED)
message(STATUS "Detected nlohmann_json version: ${nlohmann_json_VERSION}")
message(STATUS "nlohmann_json directory: ${nlohmann_json_DIR}")

# ------------------------------------------------------------------------------
# "Look" for an installation of EPANET in the system.
# Adjust for the EPANET version.
# Differently from vcpkg, JSON, and Pagmo libraries that have consistent versioning
# in their releases and are independent of the BeMe library, EPANET is a library that
# is being wrapped and is not properly versioned (only major and not frequent).
# Therefore, we need to make sure that we track the version of EPANET that we are using
# and that we are wrapping. We will use the date of the commit in the dev branch 
# of the OWA-EPANET repository as the version of the library ("YYMMDD").
# At the time of the writing, we are testing the code on EPANET on the dev branch 
# with the latest update at 21 December 2024.
set(EPANET_VERSION 241221)
IF(BEME_VERSION LESS 250200)
  # Before the update to this version, we were using 240618 (Merge pull request #802).
  set(EPANET_VERSION 240618)

  # Issue a warning that the version of EPANET is not the latest and it is known
  # to produce different results because of a bug fix on tanks never draining.
  # Therefore use this only in the case of reproducibility of the old results.
  message(WARNING "Using EPANET in a previous version. The results differ from the latest version. Use this only in case you are trying to reproduce old results.\nThis is known to happen after the fix of the tanks never draining issue (pull request #803).")
ENDIF()
message(STATUS "Using EPANET version: ${EPANET_VERSION}")

# Some of my code also chanes slighlty based on the version of EPANET, for example,
# before 240712 (merge pull request #808) the leakage was not implemented so it should 
# not appear in the code.
add_definitions(-DEPANET_VERSION=${EPANET_VERSION})

# Set the path to the EPANET library with the flag '-DEPANET_ROOT=/path/to/EPANET'
# Cached variable.
set( EPANET_ROOT "${PROJECT_SOURCE_DIR}/../EPANET" CACHE PATH "Path to the rootfolder to EPANET")
message(STATUS "EPANET directory: ${EPANET_ROOT}")

# In Debug mode, we want to wrap EPANET to debug errors and to have a better understanding.
cmake_dependent_option(EPANET_FROM_SOURCE "Build EPANET from source" ON "CMAKE_BUILD_TYPE MATCHES Debug" OFF)
message(STATUS "EPANET from source: ${EPANET_FROM_SOURCE}")
IF(EPANET_FROM_SOURCE)
  set(EPANET_SRC
      "${EPANET_ROOT}/src/epanet.c"
      # "${EPANET_ROOT}/src/epanet2.c"
      "${EPANET_ROOT}/src/genmmd.c"
      "${EPANET_ROOT}/src/hash.c"
      "${EPANET_ROOT}/src/hydcoeffs.c"
      "${EPANET_ROOT}/src/hydraul.c"
      "${EPANET_ROOT}/src/hydsolver.c"
      "${EPANET_ROOT}/src/hydstatus.c"
      "${EPANET_ROOT}/src/inpfile.c"
      "${EPANET_ROOT}/src/input1.c"
      "${EPANET_ROOT}/src/input2.c"
      "${EPANET_ROOT}/src/input3.c"
      "${EPANET_ROOT}/src/mempool.c"
      "${EPANET_ROOT}/src/output.c"
      "${EPANET_ROOT}/src/project.c"
      "${EPANET_ROOT}/src/quality.c"
      "${EPANET_ROOT}/src/qualreact.c"
      "${EPANET_ROOT}/src/qualroute.c"
      "${EPANET_ROOT}/src/report.c"
      "${EPANET_ROOT}/src/rules.c"
      "${EPANET_ROOT}/src/smatrix.c"
      "${EPANET_ROOT}/src/validate.c"
      "${EPANET_ROOT}/src/util/cstr_helper.c"
      "${EPANET_ROOT}/src/util/errormanager.c"
      "${EPANET_ROOT}/src/util/filemanager.c"
    )

    IF(EPANET_VERSION GREATER 240712) # Merge pull request #808 (Add leakage)
      list(APPEND EPANET_SRC
        "${EPANET_ROOT}/src/leakage.c"
        "${EPANET_ROOT}/src/flowbalance.c"
      )
    ENDIF()

    add_library(epanet ${EPANET_SRC})

    # All API functions of the EPANET library are defined with the DLLEXPORT macro.
    # Since we are building to source and between lines 21 and 38, when on windows, it 
    # defines the DLLEXPORT macro as __declspec(dllexport) __stdcall or __declspec(dllimport) __stdcall
    # we need to empty define the macro so that the functions are not exported.
    # The EPANET_VERSION is also defined but has no effect on the library.
    target_compile_definitions(epanet PUBLIC "DLLEXPORT=" "EPANET_VERSION=${EPANET_VERSION}")

    target_include_directories(epanet PUBLIC 
                                  "${EPANET_ROOT}/include"
                                  "${EPANET_ROOT}/src"
    )

ELSE() # NOT EPANET FROM SOURCE

  # Normal behaviour, we are using the library.
  add_library(epanet SHARED IMPORTED)
  
  # However, based on the operating system, we need to set the correct path to the library.
  # We also need to adjust for the EPANET version. We require that EPANET is 
  # built in builds/VERSION_YY.VERSION_MM.VERSION_DD folder for previous versions
  # and in build for the latest version.
  set(EPANET_BUILD_DIR "${EPANET_ROOT}/build")
  IF(BEME_VERSION LESS 250200)
    math(EXPR MAJOR_VERSION "${EPANET_VERSION} / 10000")
    math(EXPR MINOR_VERSION "(${EPANET_VERSION} % 10000) / 100")
    math(EXPR PATCH_VERSION "${EPANET_VERSION} % 100")
    set(EPANET_BUILD_DIR "${EPANET_ROOT}/builds/${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
    # string(REGEX REPLACE "([0-9][0-9])([0-9][0-9])([0-9][0-9])" "${EPANET_ROOT}/builds/\\1.\\2.\\3" EPANET_BUILD_DIR ${EPANET_VERSION})
    message(STATUS "EPANET build directory: ${EPANET_BUILD_DIR}")
  ENDIF()

  # When using Visual Studio on Windows, cmake is a multi-configuration generator.
  # We need to set the path for Release configuration. On Unix, makefiles are usually
  # used, so it is a single configuration to the folder.

  # To expose the library to the system we define the IMPORTED_LOCATION property to the target.
  IF (WIN32)
    set_target_properties(epanet PROPERTIES
      IMPORTED_LOCATION "${EPANET_BUILD_DIR}/bin/Release/epanet2.dll"
      IMPORTED_IMPLIB   "${EPANET_BUILD_DIR}/lib/Release/epanet2.lib"
    )
  ENDIF(WIN32)
  IF(APPLE)
    set_target_properties(epanet PROPERTIES
      IMPORTED_LOCATION "${EPANET_BUILD_DIR}/lib/libepanet2.dylib"
    )
  ENDIF(APPLE)
  IF(LINUX)
    set_target_properties(epanet PROPERTIES
      IMPORTED_LOCATION "${EPANET_BUILD_DIR}/lib/libepanet2.so"
    )
  ENDIF(LINUX)

  # Finally, we need to include the directories where the headers are located so
  # that my library can see them.
  target_include_directories(epanet INTERFACE "${EPANET_ROOT}/include" "${EPANET_ROOT}/src")

ENDIF()

# ------------------------------------------------------------------------------
# Include my library
add_subdirectory("bevarmejolib")

# Include the executables and the experiment classes
add_subdirectory("cli")
