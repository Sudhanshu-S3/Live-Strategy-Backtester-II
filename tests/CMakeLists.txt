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

# **THE FIX IS HERE:** Link the test executable against OpenSSL.
target_link_libraries(run_tests PRIVATE
    hft_system_core
    GTest::gtest_main
    OpenSSL::SSL
    OpenSSL::Crypto
)

include(GoogleTest)
gtest_discover_tests(run_tests)