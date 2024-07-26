# CMakeList.txt : CMake project for all the Anytown formulations
#

add_executable(beme-opt "bemeopt.cpp" 
							"src/experiment.cpp" "include/bevarmejo/experiment.hpp"
							"src/parsers.cpp" "include/bevarmejo/parsers.hpp"
)

set_property(TARGET beme-opt PROPERTY CXX_STANDARD 17)

target_link_libraries(beme-opt PRIVATE bemelib anytown hanoi)

target_include_directories(beme-opt PRIVATE
		"${CMAKE_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/bevarmejo"
)

add_executable(beme-sim "bemesim.cpp" 
							"include/bevarmejo/factories.hpp"
							"src/parsers.cpp" "include/bevarmejo/parsers.hpp"
							
)

set_property(TARGET beme-sim PROPERTY CXX_STANDARD 17)

target_link_libraries(beme-sim PRIVATE bemelib anytown hanoi)

target_include_directories(beme-sim PRIVATE
		"${CMAKE_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/bevarmejo"
)