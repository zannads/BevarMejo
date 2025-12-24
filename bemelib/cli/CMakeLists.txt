# CMakeList.txt : CMake project for all the Anytown formulations
#

add_executable(beme-opt "bemeopt.cpp" 
						"${PROJECT_SOURCE_DIR}/src/experiment.cpp"
)

set_property(TARGET beme-opt PROPERTY CXX_STANDARD 17)

target_link_libraries(beme-opt PRIVATE bemelib)

target_include_directories(beme-opt PRIVATE
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_SOURCE_DIR}/include/beme"
)

add_executable(beme-sim "bemesim.cpp"
						"${PROJECT_SOURCE_DIR}/src/simulator.cpp"
)

set_property(TARGET beme-sim PROPERTY CXX_STANDARD 17)

target_link_libraries(beme-sim PRIVATE bemelib)

target_include_directories(beme-sim PRIVATE
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_SOURCE_DIR}/include/beme"
)