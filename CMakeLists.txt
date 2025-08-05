# =============================================================================
# Project Definition & C++ Standard
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(HFT_System LANGUAGES CXX)

# --- Set Output Directories ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Dependency Management (Modern Approach)
# =============================================================================
include(FetchContent)

# --- spdlog (for logging) ---
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# --- GoogleTest (for testing) ---
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# --- simdjson (for high-performance JSON parsing) ---
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG        v3.9.5
)
FetchContent_MakeAvailable(simdjson)

# =============================================================================
# Project Structure
# =============================================================================
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)

# =============================================================================
# Main Executable Target
# =============================================================================
add_executable(hft_system main.cpp)
target_link_libraries(hft_system PRIVATE hft_system_core)