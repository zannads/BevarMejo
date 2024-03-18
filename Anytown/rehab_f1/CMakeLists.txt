# CMakeList.txt : CMake project for the Model of the Anytown problem
#

add_library(anytownRehabF1lib "model_anytown.hpp" "model_anytown.cpp" )

target_link_libraries(anytownRehabF1lib PUBLIC Pagmo::pagmo epanet bemelib)

target_include_directories(anytownRehabF1lib PUBLIC
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
)

add_executable( simAnytownRehab_f1 "sim_anytown_rehab_f1.cpp" )

target_link_libraries(simAnytownRehab_f1 PRIVATE Pagmo::pagmo bemelib anytownRehabF1lib)

target_include_directories(simAnytownRehab_f1 PUBLIC
                          ${PROJECT_BINARY_DIR}
                          ${PROJECT_CURRENT_SOURCE_DIR}
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
                          )

add_executable (optAnytownRehab_f1 "opt_anytown_rehab_f1.cpp" )

target_link_libraries(optAnytownRehab_f1 PRIVATE Pagmo::pagmo bemelib anytownRehabF1lib)

target_include_directories(optAnytownRehab_f1 PUBLIC
                          ${PROJECT_BINARY_DIR}
                          ${PROJECT_CURRENT_SOURCE_DIR}
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
                          )
