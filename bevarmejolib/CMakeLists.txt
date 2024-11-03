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
        "${WDS_SRC_ROOT}/water_distribution_system.cpp" 
        "${WDS_SRC_ROOT}/epanet_helpers/water_distribution_system.cpp"
        "${WDS_SRC_ROOT}/user_defined_elements_group.cpp"
)

set(WDS_BASE_ELEMENTS
        "${WDS_SRC_ROOT}/elements/element.cpp"
        "${WDS_SRC_ROOT}/elements/network_element.cpp"
)
set(WDS_NODES   
        "${WDS_SRC_ROOT}/elements/junction.cpp"
        "${WDS_SRC_ROOT}/elements/tank.cpp"
        "${WDS_SRC_ROOT}/elements/reservoir.cpp"
        "${WDS_SRC_ROOT}/elements/source.cpp"
        "${WDS_SRC_ROOT}/elements/node.cpp"
)
set(WDS_LINKS   
        "${WDS_SRC_ROOT}/elements/pipe.cpp"
        "${WDS_SRC_ROOT}/elements/dimensioned_link.cpp"
        "${WDS_SRC_ROOT}/elements/link.cpp"
        "${WDS_SRC_ROOT}/elements/pump.cpp"
)

set(WDS_AUX     
        "${WDS_SRC_ROOT}/auxiliary/curves.cpp"
        "${WDS_SRC_ROOT}/auxiliary/pattern.cpp"
        "${WDS_SRC_ROOT}/auxiliary/global_times.cpp"
        "${WDS_SRC_ROOT}/auxiliary/time_series.cpp"
)

set(IO_FILES
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/io/fsys_helpers.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/io/key.cpp"
)

set(HELPERS     
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/pagmo_helpers/containers_help.cpp"
)

set(PAGMO_H
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/pagmo_helpers/containers_serializers.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/pagmo_helpers/default_objects_serializers.cpp"
)

set(UTILS
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/string_manip.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/unique_string_sequence.cpp"
)

add_library(bemelib 
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/hydraulic_functions.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/factories.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/wds_problem.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/wds_problem_detail.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/library_metadata.cpp"
        ${WDS_FILES}
        ${WDS_BASE_ELEMENTS}
        ${WDS_NODES}
        ${WDS_LINKS}
        ${WDS_AUX}
        ${WDS_DATAS}
        ${IO_FILES}
        ${HELPERS}
        ${PAGMO_H}
        ${UTILS}
)
            
set_property(TARGET bemelib PROPERTY CXX_STANDARD 17)

target_link_libraries(bemelib PUBLIC Pagmo::pagmo epanet nlohmann_json::nlohmann_json)

target_include_directories(bemelib PUBLIC
                            "include"
                            "${CMAKE_SOURCE_DIR}"
)

target_include_directories(bemelib PRIVATE
                        "include/bevarmejo"
                        "include/bevarmejo/io"
                        "include/bevarmejo/epanet_helpers"
                        "include/bevarmejo/pagmo_helpers"
                        "include/bevarmejo/pagmo_helpers/algorithms"
                        "include/bevarmejo/wds"
                        "include/bevarmejo/wds/elements"
                        "include/bevarmejo/wds/auxiliary"
                        "include/bevarmejo/wds/epanet_helpers"
)
