include(GoogleTest)

add_executable( VarsTests
    vars_tests.cpp
)

target_link_libraries( VarsTests PUBLIC
    bemelib
    GTest::gtest_main
)

gtest_discover_tests( VarsTests )
