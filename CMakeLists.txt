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


project ("BèvarMéjo")

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

# "Look" for an installation of EPANET in the system.
IF(WIN32)
  add_library(epanet SHARED IMPORTED )
  set_target_properties(epanet PROPERTIES
            IMPORTED_LOCATION "${EPANET_ROOT}/build/bin/Release/epanet2.dll"
            IMPORTED_IMPLIB "${EPANET_ROOT}/build/lib/Release/epanet2.lib"
            )
ENDIF(WIN32)

IF(APPLE)
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
  target_include_directories(epanet PUBLIC "${EPANET_ROOT}/include")

  #add_library(epanet SHARED IMPORTED)
  #set_target_properties(epanet PROPERTIES
   #          IMPORTED_LOCATION "${EPANET_ROOT}/build/lib/libepanet2.dylib" 
    #         )
ENDIF(APPLE)

IF(LINUX)
  add_library(epanet SHARED IMPORTED)
  set_target_properties(epanet PROPERTIES
        IMPORTED_LOCATION "${EPANET_ROOT}/build/lib/libepanet2.so"
        )
ENDIF(LINUX)

# Include my library
add_subdirectory("bevarmejolib")

# Include sub-projects.
add_subdirectory ("Hanoi")
add_subdirectory ("Anytown")