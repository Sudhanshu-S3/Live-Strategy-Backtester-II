# tests/CMakeLists.txt

add_executable(run_tests
    main_test.cpp
    portfolio_manager_test.cpp
    strategy_manager_test.cpp
    execution_handler_test.cpp
    integration_test.cpp
)

target_include_directories(run_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(run_tests PRIVATE
    hft_system_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)