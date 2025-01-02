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
set(BEME_IO
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/io/key.cpp"
)

set(BEME_UTILS
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/string_manip.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/unique_string_sequence.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/pagmo/containers_help.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/pagmo/containers_serializers.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/utility/pagmo/default_objects_serializers.cpp"
)

# ==============================
# WDS
# ==============================
set(WDS_INC_ROOT    
        "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds" 
        CACHE PATH "Path to the rootfolder of the WDS"
)
set(WDS_SRC_ROOT
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/wds" 
        CACHE PATH "Path to the rootfolder of the WDS"
)

set(BEME_WDS_BASE_ELEMENTS
        "${WDS_SRC_ROOT}/element.cpp"
        "${WDS_SRC_ROOT}/elements/network_element.cpp"
        "${WDS_SRC_ROOT}/elements/curve.cpp"
        "${WDS_SRC_ROOT}/elements/pattern.cpp"
)
set(BEME_WDS_NODES
        "${WDS_SRC_ROOT}/elements/network_elements/node.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/nodes/junction.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/nodes/tank.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/nodes/reservoir.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/nodes/source.cpp"
        
)
set(BEME_WDS_LINKS   
        "${WDS_SRC_ROOT}/elements/network_elements/link.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/links/dimensioned_link.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/links/pipe.cpp"
        "${WDS_SRC_ROOT}/elements/network_elements/links/pump.cpp"
)

set(BEME_WDS_UTIL
        "${WDS_SRC_ROOT}/utility/global_times.cpp"
        "${WDS_SRC_ROOT}/utility/time_series.cpp"
)

set(BEME_WDS
        "${WDS_SRC_ROOT}/water_distribution_system.cpp" 
        "${WDS_SRC_ROOT}/utility/epanet/water_distribution_system.cpp"
        ${BEME_WDS_BASE_ELEMENTS}
        ${BEME_WDS_NODES}
        ${BEME_WDS_LINKS}
        ${BEME_WDS_UTIL}
)

# ==============================
# SIMULATION
# ==============================
set(BEME_SIMULATION
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/simulation/hyd_sim_settings.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/simulation/solvers/epanet/hydraulic.cpp"
)

# ==============================
# EVALUATION
# ==============================
set(BEME_EVALUATION
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/evaluation/metrics/hydraulic_functions.cpp"
)


# ==============================
# PROBLEM
# ==============================
set(BEME_PROBLEM
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/problem/wds_problem.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/problem/wds_problem_detail.cpp"
)

add_library(bemelib 
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/hydraulic_functions.cpp"
        "${PROJECT_SOURCE_DIR}/bevarmejolib/src/factories.cpp"
        ${BEME_IO}
        ${BEME_PROBLEM}
        ${BEME_UTILS}
        ${BEME_WDS}
        ${BEME_SIMULATION}
        ${BEME_EVALUATION}
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
                        "include/bevarmejo/utility"
                        "include/bevarmejo/wds"
                        "include/bevarmejo/simulation"
                        "include/bevarmejo/evaluation"
                        "include/bevarmejo/problem"
)
