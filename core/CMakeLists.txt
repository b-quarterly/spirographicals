add_library(spiro-core)

target_sources(spiro-core
    PRIVATE
        src/api.cpp
        ${CMAKE_SOURCE_DIR}/third_party/glad/glad.c
)

target_include_directories(spiro-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

target_include_directories(spiro-core
    PRIVATE
        ${glm_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/stb
        ${CMAKE_SOURCE_DIR}/third_party/glad/include
)

target_link_libraries(spiro-core
    PRIVATE
        glfw
)
