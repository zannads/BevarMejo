# CMakeList.txt : CMake project for all the Anytown formulations
#

add_library(anytown "prob_anytown.hpp" "prob_anytown.cpp"
                    "rehab/prob_at_reh_f1.hpp" "rehab/prob_at_reh_f1.cpp"
					"mixed/prob_at_mix_f1.hpp" "mixed/prob_at_mix_f1.cpp"
)

set_property(TARGET anytown PROPERTY CXX_STANDARD 17)

target_link_libraries(anytown PRIVATE bemelib)

target_include_directories(anytown PRIVATE
							"${CMAKE_SOURCE_DIR}"
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${CMAKE_CURRENT_SOURCE_DIR}/rehab"
							"${CMAKE_CURRENT_SOURCE_DIR}/mixed"
							"${CMAKE_CURRENT_SOURCE_DIR}/twophases"
)
