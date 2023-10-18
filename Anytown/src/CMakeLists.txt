# CMakeList.txt : CMake project for the Model of the Anytown problem
#

add_library(modelAnytown "model_anytown.cpp" "model_anytown.hpp" )

target_link_libraries(modelAnytown PUBLIC Pagmo::pagmo epanet bemelib)

target_include_directories(modelAnytown PUBLIC
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include"
)
