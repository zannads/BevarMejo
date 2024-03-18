# CMakeList.txt : CMake project for all the Anytown formulations
#

add_subdirectory(mixed_f1)
add_subdirectory(rehab_f1)

add_custom_target(all_anytown_formulations DEPENDS simAnytownMixed_f1 optAnytownMixed_f1 simAnytownRehab_f1 optAnytownRehab_f1)