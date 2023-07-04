# CMakeList.txt : CMake project for logger, include source and define
# project specific logic here.
#

add_library(bemelib_logger "populationsHandler.cpp" "populationsHandler.hpp")

target_link_libraries(bemelib_logger PUBLIC Pagmo::pagmo)

target_include_directories(bemelib_logger PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}"
)
