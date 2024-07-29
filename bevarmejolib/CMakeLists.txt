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

set(WDS_INC_ROOT    "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds" 
         CACHE PATH "Path to the rootfolder of the WDS"
)
set(WDS_FILES   "${WDS_INC_ROOT}/water_distribution_system.cpp" "${WDS_INC_ROOT}/water_distribution_system.hpp" 
                "${WDS_INC_ROOT}/elements_group.hpp" 
                "${WDS_INC_ROOT}/user_defined_elements_group.hpp" "${WDS_INC_ROOT}/user_defined_elements_group.cpp"
                "${WDS_INC_ROOT}/auxiliary/curve.hpp"
                "${WDS_INC_ROOT}/auxiliary/curves.hpp" "${WDS_INC_ROOT}/auxiliary/curves.cpp"
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

set(WDS_AUX     "${WDS_INC_ROOT}/auxiliary/curve.hpp"
                "${WDS_INC_ROOT}/auxiliary/curves.hpp" "${WDS_INC_ROOT}/auxiliary/curves.cpp"
                "${WDS_INC_ROOT}/auxiliary/demand.hpp"
                "${WDS_INC_ROOT}/auxiliary/pattern.hpp" "${WDS_INC_ROOT}/auxiliary/pattern.cpp"
                )

set(WDS_DATAS   "${WDS_INC_ROOT}/data_structures/variable.hpp"
                "${WDS_INC_ROOT}/data_structures/temporal.hpp"
)

set(HELPERS     "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/epanet_helpers/en_help.hpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/epanet_helpers/en_time_options.hpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/pagmo_helpers/containers_help.hpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/src/pagmo_helpers/containers_help.cpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/pagmo_helpers/udc_help.hpp"
                "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp")

add_library(bemelib "include/bevarmejo/io.hpp" "src/io.cpp" 
                    "include/bevarmejo/constants.hpp"
                    "include/bevarmejo/econometric_functions.hpp"
                    "include/bevarmejo/hydraulic_functions.hpp" "src/hydraulic_functions.cpp"
                    "include/bevarmejo/labels.hpp"
                    "include/bevarmejo/type_traits.hpp"
                    "include/bevarmejo/time_series.hpp" "src/time_series.cpp"
                    "include/bevarmejo/quantity_series.hpp"
                    ${WDS_FILES}
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
)
