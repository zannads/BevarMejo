#Set the path to the EPANET folder and library
#The path to the folder can be set in 2 ways:
#     1. pass a flag at the first cmake invocation
#         `cmake .. -DEPANET_ROOT=/path/to/EPANET`
#     3. set the enviromental variable EPANET_ROOT = /path/to/EPANET // see how to do it base on your OS
#         `cmake ..` 

set( EPANET_ROOT "$ENV{EPANET_ROOT}" CACHE PATH "Path to the rootfolder to EPANET")

IF(WIN32)
	add_library(epanet SHARED IMPORTED )
	set_target_properties(epanet PROPERTIES
	            IMPORTED_LOCATION "${EPANET_ROOT}/build/bin/Release/epanet2.dll"
#	            )
#	set_property(TARGET epanet PROPERTY
		        IMPORTED_IMPLIB "${EPANET_ROOT}/build/lib/Release/epanet2.lib"
		        )
ENDIF(WIN32)

IF(APPLE)
	add_library(epanet SHARED IMPORTED)
	set_target_properties(epanet PROPERTIES
						 IMPORTED_LOCATION "${EPANET_ROOT}/build/lib/libepanet2.dylib" 
						 )
ENDIF(APPLE)

IF(LINUX)
	add_library(epanet SHARED IMPORTED)
	set_target_properties(epanet PROPERTIES
				IMPORTED_LOCATION "${EPANET_ROOT}/build/lib/libepanet2.so"
				)
ENDIF(LINUX)

add_library(modelHanoi "model_hanoi.cpp" )

target_link_libraries(modelHanoi PUBLIC Pagmo::pagmo epanet)

target_include_directories(modelHanoi PUBLIC 
							Pagmo::pagmo
							"${PROJECT_BINARY_DIR}"
							"${PROJECT_SOURCE_DIR}"
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${PROJECT_SOURCE_DIR}/Hanoi/include"
							"${EPANET_ROOT}/include"
)
