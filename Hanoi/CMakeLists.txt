# CMakeList.txt : CMake project for Hanoi, include source and define
# project specific logic here.
#

add_library(hanoi "problem_hanoi_biobj.cpp" "problem_hanoi_biobj.hpp")

set_property(TARGET hanoi PROPERTY CXX_STANDARD 17)

target_link_libraries(hanoi PRIVATE bemelib)

target_link_directories(hanoi PUBLIC 
                            "${CMAKE_CURRENT_SOURCE_DIR}"
)