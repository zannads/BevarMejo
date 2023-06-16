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
set( VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "Path to the rootfolder to vcpkg")
set( FULL_TOOLCHAIN_FILENAME "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" )    #FILEPATH "Full path to the file Vcpkg"
set( CMAKE_TOOLCHAIN_FILE ${FULL_TOOLCHAIN_FILENAME}
	   CACHE FILEPATH "Vcpkg toolchain file")

project ("BèvarMéjo")


# Include sub-projects.
add_subdirectory ("Hanoi")
