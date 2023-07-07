# CMakeList.txt : CMake project for Bèvar Méjo c++ library, include source and define
# project specific logic here.
#

#file(GLOB BEMELIB_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} src/*.cpp)
#add_library(bemelib ${BEMELIB_SOURCES})

add_library(bemelib "src/io.cpp" "src/experiment.cpp" "include/bevarmejo/io.hpp" "include/bevarmejo/experiment.hpp")
set_property(TARGET bemelib PROPERTY CXX_STANDARD 17)

target_link_libraries(bemelib PUBLIC Pagmo::pagmo)

target_include_directories(bemelib PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}"
                            "include"
                            "include/bevarmejo"
)
