# CMakeList.txt : CMake project for logger, include source and define
# project specific logic here.
#

add_library(logger "populationsHandler.cpp" )

target_link_libraries(logger PUBLIC Pagmo::pagmo)

target_include_directories(logger PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}"
)
