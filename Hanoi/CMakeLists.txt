# CMakeList.txt : CMake project for Hanoi, include source and define
# project specific logic here.
#

add_subdirectory(src)

# Add source to this project's executable.
add_executable (hanoiOptimization "main_hanoi.cpp" "main_hanoi.h" )


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET hanoiOptimization PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(hanoiOptimization PRIVATE Pagmo::pagmo bemelib_logger modelHanoi)

target_include_directories(hanoiOptimization PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/Hanoi/include"
                          "${PROJECT_SOURCE_DIR}/lib"
                          )
