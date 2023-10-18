# CMakeList.txt : CMake project for the original Anytown problem
#

add_subdirectory(src)

# Add source to this project's executable.
add_executable (anytownTest "main_anytown_test.cpp" )


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET anytownTest PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(anytownTest PRIVATE Pagmo::pagmo bemelib modelAnytown)

target_include_directories(anytownTest PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/Anytown/src"
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
                          )
