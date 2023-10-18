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
set(WDS_INC_ROOT "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds" CACHE PATH "Path to the rootfolder of the WDS")
set(WDS_FILES "${WDS_INC_ROOT}/water_distribution_system.cpp" "${WDS_INC_ROOT}/water_distribution_system.hpp" 
              "${WDS_INC_ROOT}/subnetwork.cpp" "${WDS_INC_ROOT}/subnetwork.hpp"
              "${WDS_INC_ROOT}/elements/variable.hpp"
              "${WDS_INC_ROOT}/elements/temporal.hpp")

add_library(bemelib "src/io.cpp" "include/bevarmejo/io.hpp" 
                    "src/experiment.cpp" "include/bevarmejo/experiment.hpp" 
                    "${PUGIXML_ROOT}/pugixml.cpp" 
                    "src/resilience_index.cpp" "include/bevarmejo/resilience_index.hpp"
                    "include/bevarmejo/constants.hpp"
                    "include/bevarmejo/econometric_functions.hpp"
                    ${WDS_FILES})
            
set_property(TARGET bemelib PROPERTY CXX_STANDARD 17)

target_link_libraries(bemelib PUBLIC Pagmo::pagmo)

target_include_directories(bemelib PUBLIC
                            "include"
                            "include/bevarmejo"
                            "include/bevarmejo/wds"
                            "${PUGIXML_ROOT}"
                            "${EPANET_ROOT}/include"
)
