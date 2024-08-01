# CMakeList.txt : CMake project for Bèvar Méjo c++ library, include source and define
# project specific logic here.
#

# PUGIXML library
# IF(DEFINED ENV{PUGIXML_ROOT})
#     set( PUGIXML_ROOT "$ENV{PUGIXML_ROOT}/src" CACHE PATH "Path to the rootfolder to pugiXML" )
#     message(STATUS "PUGIXML_ROOT environmental variable found, using ${PUGIXML_ROOT}" )
# ELSE()
#     set( PUGIXML_ROOT "${PROJECT_SOURCE_DIR}/../pugixml/src" CACHE PATH "Path to the rootfolder of PUGIXML")
#     message(STATUS "PUGIXML_ROOT environmental variable NOT found, using ${PUGIXML_ROOT}" )
# ENDIF()

set(WDS_INC_ROOT    
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds" 
        CACHE PATH "Path to the rootfolder of the WDS"
)
set(WDS_SRC_ROOT
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/wds" 
        CACHE PATH "Path to the rootfolder of the WDS"
)
set(WDS_FILES   
        "${WDS_INC_ROOT}/water_distribution_system.hpp" 
        "${WDS_SRC_ROOT}/water_distribution_system.cpp" 
        "${WDS_SRC_ROOT}/epanet_helpers/water_distribution_system.cpp"
        "${WDS_INC_ROOT}/elements_group.hpp" 
        "${WDS_INC_ROOT}/user_defined_elements_group.hpp" "${WDS_SRC_ROOT}/user_defined_elements_group.cpp"
)

set(WDS_BASE_ELEMENTS
                "${WDS_INC_ROOT}/elements/element.hpp" "${WDS_SRC_ROOT}/elements/element.cpp"
                "${WDS_INC_ROOT}/elements/network_element.hpp" "${WDS_SRC_ROOT}/elements/network_element.cpp"
)
set(WDS_NODES   
        "${WDS_INC_ROOT}/elements/junction.hpp" "${WDS_SRC_ROOT}/elements/junction.cpp"
        "${WDS_INC_ROOT}/elements/tank.hpp" "${WDS_SRC_ROOT}/elements/tank.cpp"
        "${WDS_INC_ROOT}/elements/reservoir.hpp" "${WDS_SRC_ROOT}/elements/reservoir.cpp"
        "${WDS_INC_ROOT}/elements/source.hpp" "${WDS_SRC_ROOT}/elements/source.cpp"
        "${WDS_INC_ROOT}/elements/node.hpp" "${WDS_SRC_ROOT}/elements/node.cpp"
)
set(WDS_LINKS   
        "${WDS_INC_ROOT}/elements/pipe.hpp" "${WDS_SRC_ROOT}/elements/pipe.cpp"
        "${WDS_INC_ROOT}/elements/dimensioned_link.hpp" "${WDS_SRC_ROOT}/elements/dimensioned_link.cpp"
        "${WDS_INC_ROOT}/elements/link.hpp" "${WDS_SRC_ROOT}/elements/link.cpp"
        "${WDS_INC_ROOT}/elements/pump.hpp" "${WDS_SRC_ROOT}/elements/pump.cpp"
)

set(WDS_AUX     
        "${WDS_INC_ROOT}/auxiliary/curve.hpp"
        "${WDS_INC_ROOT}/auxiliary/curves.hpp" "${WDS_SRC_ROOT}/auxiliary/curves.cpp"
        "${WDS_INC_ROOT}/auxiliary/demand.hpp"
        "${WDS_INC_ROOT}/auxiliary/pattern.hpp" "${WDS_SRC_ROOT}/auxiliary/pattern.cpp"
        "${WDS_INC_ROOT}/auxiliary/time_series.hpp" "${WDS_SRC_ROOT}/auxiliary/time_series.cpp"
        "${WDS_INC_ROOT}/auxiliary/quantity_series.hpp" 
)

# to be removed soon
set(WDS_DATAS   "${WDS_INC_ROOT}/data_structures/variable.hpp"
                "${WDS_INC_ROOT}/data_structures/temporal.hpp"
)

set(WDS_HELPERS
        "${WDS_INC_ROOT}/epanet_helpers/en_help.hpp"
        "${WDS_INC_ROOT}/epanet_helpers/en_time_options.hpp"
)

set(HELPERS     
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/pagmo_helpers/containers_help.hpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/src/pagmo_helpers/containers_help.cpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/pagmo_helpers/udc_help.hpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp")

add_library(bemelib 
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/io.hpp" "${PROJECT_SOURCE_DIR}/bevarmejolib/src/io.cpp" 
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/constants.hpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/econometric_functions.hpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/hydraulic_functions.hpp" "${PROJECT_SOURCE_DIR}/bevarmejolib/src/hydraulic_functions.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/labels.hpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/type_traits.hpp"
        ${WDS_FILES}
        ${WDS_BASE_ELEMENTS}
        ${WDS_NODES}
        ${WDS_LINKS}
        ${WDS_AUX}
        ${WDS_DATAS}
        ${HELPERS}
)
            
set_property(TARGET bemelib PROPERTY CXX_STANDARD 17)

target_link_libraries(bemelib PUBLIC Pagmo::pagmo epanet nlohmann_json::nlohmann_json)

target_include_directories(bemelib PUBLIC
                            "include"
                            "${CMAKE_SOURCE_DIR}"
)

target_include_directories(bemelib PRIVATE
                            "include/bevarmejo"
                            "include/bevarmejo/epanet_helpers"
                            "include/bevarmejo/pagmo_helpers"
                            "include/bevarmejo/pagmo_helpers/algorithms"
                            "include/bevarmejo/wds"
                            "include/bevarmejo/wds/elements"
                            "include/bevarmejo/wds/auxiliary"
                            "include/bevarmejo/wds/data_structures"
)
