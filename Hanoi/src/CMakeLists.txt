# PUGIXML library
IF(DEFINED ENV{PUGIXML_ROOT})
	set( PUGIXML_ROOT "$ENV{PUGIXML_ROOT}" CACHE PATH "Path to the rootfolder to pugiXML" )
	message(STATUS "PUGIXML_ROOT environmental variable found, using ${PUGIXML_ROOT}" )
ELSE()
	set( PUGIXML_ROOT "${PROJECT_SOURCE_DIR}/../pugixml/src" CACHE PATH "Path to the rootfolder of PUGIXML")
	message(STATUS "PUGIXML_ROOT environmental variable NOT found, using ${PUGIXML_ROOT}" )
ENDIF()

add_library(modelHanoi "model_hanoi.cpp" "hanoi.cpp" "${PUGIXML_ROOT}/pugixml.cpp")

target_link_libraries(modelHanoi PUBLIC Pagmo::pagmo epanet bemelib_classes)

target_include_directories(modelHanoi PUBLIC 
							Pagmo::pagmo
							"${CMAKE_CURRENT_SOURCE_DIR}/../include"
							"${EPANET_ROOT}/include"
							"${PUGIXML_ROOT}"
							"${PROJECT_SOURCE_DIR}/lib"
)
