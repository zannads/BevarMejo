include(GoogleTest)

add_executable( VarsTests
    vars_tests.cpp
)

target_link_libraries( VarsTests PUBLIC
    bemelib
    GTest::gtest_main
)

add_executable( ResTests
    results_tests.cpp
)

target_link_libraries( ResTests PUBLIC
    bemelib
    GTest::gtest_main
)

gtest_discover_tests( VarsTests )
gtest_discover_tests( ResTests )
