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

FetchContent_MakeAvailable(glfw glad)

FetchContent_Populate(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${glm_SOURCE_DIR})

target_link_libraries(spiro-core
    PRIVATE
        glfw
        glad
        glm
)
