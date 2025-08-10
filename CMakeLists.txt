# =============================================================================
# Project Definition & C++ Standard
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(HFT_System LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Pass the project's root directory path to the C++ code
add_compile_definitions(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# --- Set Output Directories ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --- Find System Dependencies ---
find_package(Boost 1.74 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# =============================================================================
# Dependency Management (FetchContent)
# =============================================================================
include(FetchContent)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.14.1)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(simdjson GIT_REPOSITORY https://github.com/simdjson/simdjson.git GIT_TAG v3.9.5)
FetchContent_MakeAvailable(simdjson)

# =============================================================================
# Project Structure & Main Executable
# =============================================================================
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)

add_executable(hft_system main.cpp)

target_link_libraries(hft_system PRIVATE
    hft_system_core
    OpenSSL::SSL
    OpenSSL::Crypto
)