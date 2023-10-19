# CMakeList.txt : CMake project for the Model of the Hanoi problem, include source and define
# project specific logic here.
#


add_library(modelHanoi "model_hanoi.cpp" "hanoi.cpp" "../include/model_hanoi.hpp" "../include/hanoi.hpp" )

target_link_libraries(modelHanoi PUBLIC Pagmo::pagmo epanet bemelib)

target_include_directories(modelHanoi PUBLIC
							"${CMAKE_CURRENT_SOURCE_DIR}/../include"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
)
