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

# Fetch glfw and glad as before
FetchContent_MakeAvailable(glfw glad)

# Fetch glm manually and define as header-only
FetchContent_Populate(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# Declare a header-only interface target for glm
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${glm_SOURCE_DIR})

# Link dependencies
target_link_libraries(spiro-core
    PRIVATE
        glfw
        glad
        glm
)
