# CMakeList.txt : CMake project for all the Anytown formulations
#

add_executable(bemeoptimize "bemeopt.cpp" 
							"src/experiment.cpp" "include/bevarmejo/experiment.hpp"
							"src/parsers.cpp" "include/bevarmejo/parsers.hpp"
)

set_property(TARGET bemeoptimize PROPERTY CXX_STANDARD 17)

target_link_libraries(bemeoptimize PRIVATE bemelib anytown hanoi)

target_include_directories(bemeoptimize PRIVATE 
		"include"
		"include/bevarmejo"
		"${CMAKE_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}"
		)

add_executable(bemesimulate "bemesim.cpp" 
							"include/bevarmejo/factories.hpp"
							"src/parsers.cpp" "include/bevarmejo/parsers.hpp"
							
)

set_property(TARGET bemesimulate PROPERTY CXX_STANDARD 17)

target_link_libraries(bemesimulate PRIVATE bemelib anytown hanoi)

target_include_directories(bemesimulate PRIVATE 
		"include"
		"include/bevarmejo"
		"${CMAKE_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}"
		)