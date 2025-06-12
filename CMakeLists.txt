# =============================================================================
# CMakeLists.txt for Spirographicals
#
# This is the root configuration file for the C++ core engine. It handles
# dependency management, build options, and installation rules.
# =============================================================================

# --- 1. Project Preamble & Basic Configuration -------------------------------
# Set the minimum required version of CMake. Modern features are used.
cmake_minimum_required(VERSION 3.20)

# Define the project name, version, and supported languages.
project(spirographicals
    VERSION 0.1.0
    DESCRIPTION "Core rendering engine for Spirographicals"
    LANGUAGES C CXX
)

# Set the C++ standard for the entire project.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# --- 2. Build Options --------------------------------------------------------
# Add an option to allow developers to disable building tests.
option(SPIRO_BUILD_TESTS "Build the project tests" ON)
message(STATUS "Build tests: ${SPIRO_BUILD_TESTS}")


# --- 3. Dependency Management (FetchContent) ---------------------------------
# Include the modern CMake module for fetching dependencies at configure time.
include(FetchContent)

# Declare dependencies. They will be downloaded and configured if not found.
# GLFW for windowing and input
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

# GLAD2 for loading OpenGL functions
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/dav1d/glad.git
    GIT_TAG        v2.0.4
)

# GLM for mathematics (header-only)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)


# --- 4. Core Library Definition ----------------------------------------------
# Pass control to the CMakeLists.txt in the `core` directory.
# This is where the `spiro-core` library target will be defined.
add_subdirectory(core)


# --- 5. Testing Setup --------------------------------------------------------
if(SPIRO_BUILD_TESTS)
    message(STATUS "Configuring test suite...")
    # Enable the CTest framework.
    enable_testing()

    # Declare GoogleTest dependency.
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    # Make googletest available. This also creates the gtest and gmock targets.
    FetchContent_MakeAvailable(googletest)

    # Now that the main library and gtest exist, add the tests subdirectory.
    # The CMakeLists.txt inside `core/tests` will define the test executable
    # and link it against `spiro-core` and `gtest`.
    add_subdirectory(core/tests)
endif()


# --- 6. Installation Rules ---------------------------------------------------
# This section is CRITICAL. It defines how to place the final artifacts
# into the `/target/cpp` directory, which is essential for the Rust build.
# The `build_core.py` script will run `cmake --install` to execute these rules.
#
# The CMAKE_INSTALL_PREFIX will be set to `../../target/cpp` by the build script.
# The destinations below are relative to that prefix.
# -----------------------------------------------------------------------------
message(STATUS "Configuring installation rules...")

# Install the library targets (e.g., spiro-core.so, spiro-core.a)
install(TARGETS spiro-core
    EXPORT spirographicals-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the public headers associated with the library.
# This relies on the FILE_SET defined in `core/CMakeLists.txt`.
install(FILE_SET spiro-core_HEADERS
    DESTINATION include
)

# Generate and install a CMake configuration file. This allows other CMake
# projects to easily find and use this library with `find_package(spirographicals)`.
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/spirographicals-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/spirographicals-config.cmake"
    INSTALL_DESTINATION lib/cmake/spirographicals
)

# Install the generated config files.
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/spirographicals-config.cmake"
    DESTINATION lib/cmake/spirographicals
)

install(
    EXPORT spirographicals-targets
    FILE spirographicals-targets.cmake
    NAMESPACE spiro::
    DESTINATION lib/cmake/spirographicals
)

message(STATUS "CMake configuration finished.")
