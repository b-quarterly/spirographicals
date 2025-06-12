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

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
endif()

target_include_directories(spiro-core
    PRIVATE 
        ${glm_SOURCE_DIR}
)

FetchContent_MakeAvailable(glfw glad)

target_link_libraries(spiro-core
    PRIVATE
        glfw
        glad
)
