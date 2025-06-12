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

FetchContent_MakeAvailable(glfw glad glm)

target_link_libraries(spiro-core
    PRIVATE
        glfw
        glad
        glm
)
