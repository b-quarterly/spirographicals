include(GoogleTest)

add_executable(core-tests
    test_primitives.cpp
)

target_link_libraries(core-tests
    PRIVATE
        spiro-core
        gtest_main
)

gtest_add_tests(TARGET core-tests)
