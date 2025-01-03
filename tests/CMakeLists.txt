include(GoogleTest)

set( LIBS bemelib GTest::gtest_main )

function( add_test_executable name position)
    add_executable( ${name}Tests ${position}${name}_tests.cpp )
    target_link_libraries( ${name}Tests PUBLIC ${LIBS} )
    target_include_directories( ${name}Tests PUBLIC "${EPANET_ROOT}/include")
    gtest_discover_tests( ${name}Tests )
endfunction()

# add_test_executable( vars )
# add_test_executable( demand )
# add_test_executable( junction )
# add_test_executable( tank )

# add_test_executable( pattern wds/auxiliary/ )

# add_test_executable( time_series wds/auxiliary/)
# add_test_executable( quantity_series )

add_test_executable( type_traits utility/ )

add_test_executable( metric evaluation/ )