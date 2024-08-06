cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

project(voxel_engine.out)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/core.c
    ${CMAKE_SOURCE_DIR}/src/camera.c
    ${CMAKE_SOURCE_DIR}/src/input.c
    ${CMAKE_SOURCE_DIR}/src/render_batch.c
    ${CMAKE_SOURCE_DIR}/src/shader.c
    ${CMAKE_SOURCE_DIR}/src/voxel.c
)

set(GLAD_SOURCES
    ${CMAKE_SOURCE_DIR}/glad/src/glad.c
)

set(GLAD_INCLUDE
${CMAKE_SOURCE_DIR}/glad/include/
${CMAKE_SOURCE_DIR}/include/
)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.6
)

FetchContent_Declare(
    cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG v0.9.4 
)

FetchContent_MakeAvailable(SDL2 cglm)

add_executable(${PROJECT_NAME} ${SOURCES} ${GLAD_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 cglm -lm)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_INCLUDE})