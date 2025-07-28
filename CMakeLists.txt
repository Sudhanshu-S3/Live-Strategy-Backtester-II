# =============================================================================
# Project Definition & C++ Standard
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(HFT_System LANGUAGES CXX)

# --- Set Output Directories (Moved to the top) ---
# This ensures all targets are placed in a clean bin/ and lib/ structure.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Enforce strict standard compliance

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

# =============================================================================
# Project Structure
# =============================================================================
# Add the source directory, which contains its own CMakeLists.txt to build our library
add_subdirectory(src)

# Enable testing and add the tests directory
enable_testing()
add_subdirectory(tests)

# =============================================================================
# Main Executable Target
# =============================================================================
# Define the final executable from the main.cpp in the project root
add_executable(hft_system main.cpp)

# Link the executable against our core library to include all its components
target_link_libraries(hft_system PRIVATE hft_system_core)