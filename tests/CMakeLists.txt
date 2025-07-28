# tests/CMakeLists.txt

# Copy our data folder to the same directory where the executable will be created.
file(COPY data DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(run_tests
    main_test.cpp
)

target_include_directories(run_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(run_tests PRIVATE
    hft_system_core
    GTest::gtest_main
)

include(GoogleTest)

# Discover tests and explicitly set their working directory.
# This ensures that relative paths to test data are always correct.
gtest_discover_tests(run_tests
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)