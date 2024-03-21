# CMakeList.txt : CMake project for the Model of the Anytown problem
#

add_library(anytownMixedF1lib "model_anytown.hpp" "model_anytown.cpp" )

target_link_libraries(anytownMixedF1lib PUBLIC Pagmo::pagmo epanet bemelib)

target_include_directories(anytownMixedF1lib PUBLIC
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
							"${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
)

add_executable( simAnytownMixed_f1 "sim_anytown_mixed_f1.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simAnytownMixed_f1 PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(simAnytownMixed_f1 PRIVATE Pagmo::pagmo bemelib anytownMixedF1lib)

target_include_directories(simAnytownMixed_f1 PUBLIC
                          ${PROJECT_BINARY_DIR}
                          ${PROJECT_CURRENT_SOURCE_DIR}
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
                          )

add_executable (optAnytownMixed_f1 "opt_anytown_mixed_f1.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET optAnytownMixed_f1 PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(optAnytownMixed_f1 PRIVATE Pagmo::pagmo bemelib anytownMixedF1lib)

target_include_directories(optAnytownMixed_f1 PUBLIC
                          ${PROJECT_BINARY_DIR}
                          ${PROJECT_CURRENT_SOURCE_DIR}
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo"
                          "${PROJECT_SOURCE_DIR}/bevarmejolib/include/bevarmejo/wds"
                          )
