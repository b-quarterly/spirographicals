cmake_minimum_required(VERSION 3.20)

project(spirographicals
    VERSION 0.1.0
    DESCRIPTION "Core rendering engine for Spirographicals"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(SET CMP0135 NEW)

option(SPIRO_BUILD_TESTS "Build the project tests" ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
)

FetchContent_Declare(
    glad
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.4.zip
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

add_subdirectory(core)

if(SPIRO_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(core/tests)
endif()

install(TARGETS spiro-core
    EXPORT spirographicals-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILE_SET spiro-core_HEADERS
    DESTINATION include
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/spirographicals-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/spirographicals-config.cmake"
    INSTALL_DESTINATION lib/cmake/spirographicals
)

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
