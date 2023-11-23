include(GoogleTest)

set( LIBS bemelib GTest::gtest_main )

function( add_test_executable name )
    add_executable( ${name}Tests ${name}_tests.cpp )
    target_link_libraries( ${name}Tests PUBLIC ${LIBS} )
    target_include_directories( ${name}Tests PUBLIC "${EPANET_ROOT}/include")
    gtest_discover_tests( ${name}Tests )
endfunction()

add_test_executable( vars )

add_test_executable( junction )