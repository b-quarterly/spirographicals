# core/CMakeLists.txt
#
# This file is included by the root CMakeLists.txt.
# --------------------------------------------------------------------

add_library(spiro-core)
target_sources(spiro-core
    PRIVATE
        src/api.cpp
)

# Define the public include directory for this target.
target_include_directories(spiro-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Define a "FILE_SET" to group the public headers for installation
# and for other targets to consume.
target_sources(spiro-core
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/spirographicals/spirographicals.h
            include/spirographicals/Spirographicals.hpp
)

# Link our library against the dependencies fetched by the root CMakeLists.txt.
FetchContent_MakeAvailable(glfw glad glm)

target_link_libraries(spiro-core
    PUBLIC
        glfw
        glad
        glm
)
