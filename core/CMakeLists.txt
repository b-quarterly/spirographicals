add_library(spiro-core)

target_sources(spiro-core
    PRIVATE
        src/api.cpp
)

target_include_directories(spiro-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Fetch glad and glfw
FetchContent_MakeAvailable(glfw glad)

# === Replace glm with a LOCAL fake target ===

include(FetchContent)
FetchContent_Declare(
    glm_src
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

FetchContent_Populate(glm_src)

# Create a header-only INTERFACE target
add_library(glm-header-only INTERFACE)
target_include_directories(glm-header-only INTERFACE ${glm_src_SOURCE_DIR})

# Use ONLY this glm target
target_link_libraries(spiro-core
    PRIVATE
        glfw
        glad
        glm-header-only
)
