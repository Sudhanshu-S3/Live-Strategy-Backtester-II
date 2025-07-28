# =============================================================================
# Project Definition
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(System LANGUAGES CXX)

message(STATUS "Building System on ${CMAKE_HOST_SYSTEM_NAME}")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Use -std=c++20 instead of -std=gnu++20

# =============================================================================
# Dependency Management (Modern Approach)
# =============================================================================
# Use FetchContent to manage dependencies, ensuring reproducible builds.
# This is ideal for containerized environments like Codespaces.
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
# Make test libraries available only when building the 'tests' directory
FetchContent_MakeAvailable(googletest)

# --- simdjson (for high-performance JSON parsing) ---
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG        v3.9.5 # Using a specific tag for stability
)
FetchContent_MakeAvailable(simdjson)


# =============================================================================
# Project Structure & Binaries
# =============================================================================
# Add the source directory, which contains the main application logic.
# It will have its own CMakeLists.txt to define the executable.
add_subdirectory(src)

# Enable testing and add the tests directory.
# This code will only be processed if testing is enabled.
enable_testing()
add_subdirectory(tests)