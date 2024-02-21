# CMakeList.txt : CMake project for Bèvar Méjo c++ library, include source and define
# project specific logic here.
#

# PUGIXML library
IF(DEFINED ENV{PUGIXML_ROOT})
    set( PUGIXML_ROOT "$ENV{PUGIXML_ROOT}" CACHE PATH "Path to the rootfolder to pugiXML" )
    message(STATUS "PUGIXML_ROOT environmental variable found, using ${PUGIXML_ROOT}" )
ELSE()
    set( PUGIXML_ROOT "${PROJECT_SOURCE_DIR}/../pugixml/src" CACHE PATH "Path to the rootfolder of PUGIXML")
    message(STATUS "PUGIXML_ROOT environmental variable NOT found, using ${PUGIXML_ROOT}" )
ENDIF()

#file(GLOB BEMELIB_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} src/*.cpp)
#add_library(bemelib ${BEMELIB_SOURCES})
set(WDS_INC_ROOT    "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds" 
         CACHE PATH "Path to the rootfolder of the WDS"
)
set(WDS_FILES   "${WDS_INC_ROOT}/water_distribution_system.cpp" "${WDS_INC_ROOT}/water_distribution_system.hpp" 
                "${WDS_INC_ROOT}/elements_group.hpp" "${WDS_INC_ROOT}/elements_group.cpp"
                "${WDS_INC_ROOT}/elements/demand.hpp"
                "${WDS_INC_ROOT}/elements/curve.hpp"
                "${WDS_INC_ROOT}/elements/curves.hpp" "${WDS_INC_ROOT}/elements/curves.cpp"
                "${WDS_INC_ROOT}/elements/pattern.hpp" "${WDS_INC_ROOT}/elements/pattern.cpp"
                "${WDS_INC_ROOT}/elements/element.hpp" "${WDS_INC_ROOT}/elements/element.cpp"
                "${WDS_INC_ROOT}/elements/network_element.hpp" "${WDS_INC_ROOT}/elements/network_element.cpp"
)
set(WDS_NODES   "${WDS_INC_ROOT}/elements/junction.hpp" "${WDS_INC_ROOT}/elements/junction.cpp"
                "${WDS_INC_ROOT}/elements/tank.hpp" "${WDS_INC_ROOT}/elements/tank.cpp"
                "${WDS_INC_ROOT}/elements/reservoir.hpp" "${WDS_INC_ROOT}/elements/reservoir.cpp"
                "${WDS_INC_ROOT}/elements/source.hpp" "${WDS_INC_ROOT}/elements/source.cpp"
                "${WDS_INC_ROOT}/elements/node.hpp" "${WDS_INC_ROOT}/elements/node.cpp"
)
set(WDS_LINKS   "${WDS_INC_ROOT}/elements/pipe.hpp" "${WDS_INC_ROOT}/elements/pipe.cpp"
                "${WDS_INC_ROOT}/elements/dimensioned_link.hpp" "${WDS_INC_ROOT}/elements/dimensioned_link.cpp"
                "${WDS_INC_ROOT}/elements/link.hpp" "${WDS_INC_ROOT}/elements/link.cpp"
                "${WDS_INC_ROOT}/elements/pump.hpp" "${WDS_INC_ROOT}/elements/pump.cpp"
)
set(WDS_VARS    "${WDS_INC_ROOT}/elements/variable.hpp"
                "${WDS_INC_ROOT}/elements/temporal.hpp"
)

add_library(bemelib "src/io.cpp" "include/bevarmejo/io.hpp" 
                    "src/experiment.cpp" "include/bevarmejo/experiment.hpp" 
                    "${PUGIXML_ROOT}/pugixml.cpp" 
                    "include/bevarmejo/constants.hpp"
                    "include/bevarmejo/econometric_functions.hpp"
                    "src/hydraulic_functions.cpp" "include/bevarmejo/hydraulic_functions.hpp"
                    "include/bevarmejo/epanet_helpers/en_helpers.hpp"
                    ${WDS_FILES}
                    ${WDS_NODES}
                    ${WDS_LINKS}
                    ${WDS_VARS}
)
            
set_property(TARGET bemelib PROPERTY CXX_STANDARD 17)

target_link_libraries(bemelib PUBLIC Pagmo::pagmo epanet)

target_include_directories(bemelib PUBLIC
                            "include"
                            "include/bevarmejo"
                            "include/bevarmejo/wds"
                            "${PUGIXML_ROOT}"
                            "${EPANET_ROOT}/include"
                            "${EPANET_ROOT}/src/"
)
