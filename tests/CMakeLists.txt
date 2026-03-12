enable_testing()

add_executable(tests
    sanity_test.cpp
    controller_test.cpp
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(tests
    PRIVATE
    GTest::gmock
    GTest::gtest
    service
    repo
    controller
)

add_test(NAME all_tests COMMAND tests)
