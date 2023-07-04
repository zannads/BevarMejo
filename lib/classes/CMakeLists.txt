# CMakeList.txt : CMake project for classes, include source and define
# project specific logic here.
#

add_library(bemelib_classes "water_distribution_system.cpp" "water_distribution_system.hpp")

target_link_libraries(bemelib_classes PUBLIC epanet)

target_include_directories(bemelib_classes PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}"
                            "${EPANET_ROOT}/include"
)
