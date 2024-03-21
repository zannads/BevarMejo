# CMakeList.txt : CMake project for all the Anytown formulations
#

add_library(anytown "prob_anytown.hpp" "prob_anytown.cpp"
                    "rehab/prob_at_reh_f1.hpp" "rehab/prob_at_reh_f1.cpp"
)

target_link_libraries(anytown PUBLIC Pagmo::pagmo epanet bemelib)

target_include_directories(anytown PUBLIC
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
)

add_executable(bemeoptAnytown "opt_anytown.cpp")

set_property(TARGET bemeoptAnytown PROPERTY CXX_STANDARD 17)

target_link_libraries(bemeoptAnytown PRIVATE Pagmo::pagmo bemelib anytown)
